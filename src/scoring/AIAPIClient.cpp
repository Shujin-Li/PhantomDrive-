#include "PhantomDrive/scoring/AIAPIClient.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace PhantomDrive {

namespace {

struct AIClientConfig {
    QString mode;
    int timeoutMs = 5000;

    QString deepseekApiKey;
    QString deepseekBaseUrl;
    QString deepseekModel;

    QString zhipuApiKey;
    QString zhipuBaseUrl;
    QString zhipuModel;
};

QString readEnv(const char* name, const QString& defaultValue = QString())
{
    const QByteArray value = qgetenv(name);
    if (value.isEmpty()) {
        return defaultValue;
    }
    return QString::fromUtf8(value).trimmed();
}

int readEnvInt(const char* name, int defaultValue)
{
    bool ok = false;
    const int value = readEnv(name).toInt(&ok);
    if (!ok || value <= 0) {
        return defaultValue;
    }
    return value;
}

QString normalizeMode(const QString& mode)
{
    const QString normalized = mode.trimmed().toLower();
    if (normalized == QStringLiteral("mock")
        || normalized == QStringLiteral("deepseek")
        || normalized == QStringLiteral("zhipu")
        || normalized == QStringLiteral("auto")) {
        return normalized;
    }
    return QStringLiteral("auto");
}

QString stripTrailingSlash(const QString& url)
{
    QString result = url.trimmed();
    while (result.endsWith('/')) {
        result.chop(1);
    }
    return result;
}

AIClientConfig loadConfig()
{
    AIClientConfig config;
    config.mode = normalizeMode(readEnv("PHANTOMDRIVE_AI_MODE", QStringLiteral("auto")));
    config.timeoutMs = readEnvInt("PHANTOMDRIVE_AI_TIMEOUT_MS", 5000);

    config.deepseekApiKey = readEnv("DEEPSEEK_API_KEY");
    config.deepseekBaseUrl = stripTrailingSlash(readEnv("DEEPSEEK_BASE_URL", QStringLiteral("https://api.deepseek.com")));
    config.deepseekModel = readEnv("DEEPSEEK_MODEL", QStringLiteral("deepseek-v4-flash"));

    config.zhipuApiKey = readEnv("ZHIPU_API_KEY");
    config.zhipuBaseUrl = stripTrailingSlash(readEnv("ZHIPU_BASE_URL", QStringLiteral("https://api.z.ai/api/paas/v4")));
    config.zhipuModel = readEnv("ZHIPU_MODEL", QStringLiteral("glm-4.7-flash"));

    return config;
}

QString buildReportPrompt(const ScoreReport& report)
{
    const QByteArray reportJson = QJsonDocument(report.toJson()).toJson(QJsonDocument::Compact);

    QString prompt;
    prompt += QStringLiteral("请用中文输出结构化驾驶教练报告，语气专业简洁。\\n");
    prompt += QStringLiteral("请包含：总评、3条优先改进建议、下一次训练策略、风险提示。\\n");
    prompt += QStringLiteral("以下是评分JSON：\\n");
    prompt += QString::fromUtf8(reportJson);
    return prompt;
}

QString extractContentFromReply(const QByteArray& body, QString& error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        error = QStringLiteral("invalid_json_reply");
        return QString();
    }

    const QJsonArray choices = document.object().value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        error = QStringLiteral("empty_choices");
        return QString();
    }

    const QJsonObject message = choices.first().toObject().value(QStringLiteral("message")).toObject();
    const QString content = message.value(QStringLiteral("content")).toString().trimmed();
    if (content.isEmpty()) {
        error = QStringLiteral("empty_content");
        return QString();
    }

    error.clear();
    return content;
}

QString callChatCompletions(const QString& providerName,
                            const QString& baseUrl,
                            const QString& model,
                            const QString& apiKey,
                            int timeoutMs,
                            const QString& prompt,
                            QString& error)
{
    if (apiKey.trimmed().isEmpty()) {
        error = providerName + QStringLiteral(": missing_api_key");
        return QString();
    }

    const QUrl endpoint(baseUrl + QStringLiteral("/chat/completions"));
    if (!endpoint.isValid()) {
        error = providerName + QStringLiteral(": invalid_endpoint");
        return QString();
    }

    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), model);
    payload.insert(QStringLiteral("temperature"), 0.2);

    QJsonArray messages;
    QJsonObject systemMessage;
    systemMessage.insert(QStringLiteral("role"), QStringLiteral("system"));
    systemMessage.insert(QStringLiteral("content"), QStringLiteral("你是驾驶教练助手，输出中文、条理清晰、可执行建议。"));
    messages.append(systemMessage);

    QJsonObject userMessage;
    userMessage.insert(QStringLiteral("role"), QStringLiteral("user"));
    userMessage.insert(QStringLiteral("content"), prompt);
    messages.append(userMessage);

    payload.insert(QStringLiteral("messages"), messages);

    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (reply != nullptr && reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start(timeoutMs);
    loop.exec();

    if (reply == nullptr) {
        error = providerName + QStringLiteral(": null_reply, url=%1").arg(endpoint.toString());
        return QString();
    }

    const bool timedOut = !timer.isActive();
    if (timer.isActive()) {
        timer.stop();
    }

    if (timedOut) {
        error = providerName + QStringLiteral(": network_error=timeout, url=%1").arg(endpoint.toString());
        reply->deleteLater();
        return QString();
    }

    const int statusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray rawBody = reply->readAll();
    const QString responseBody =
            QString::fromUtf8(rawBody).left(500).trimmed();

    if (reply->error() != QNetworkReply::NoError) {
        error = providerName
                + QStringLiteral(": url=%1, http_status=%2, network_error=%3, body=%4")
                        .arg(endpoint.toString())
                        .arg(statusCode)
                        .arg(reply->errorString())
                        .arg(responseBody.isEmpty() ? QStringLiteral("<empty>") : responseBody);
        reply->deleteLater();
        return QString();
    }
    reply->deleteLater();

    QString parseError;
    const QString content = extractContentFromReply(rawBody, parseError);
    if (content.isEmpty()) {
        error = providerName
                + QStringLiteral(": url=%1, http_status=%2, parse_error=%3, body=%4")
                        .arg(endpoint.toString())
                        .arg(statusCode)
                        .arg(parseError)
                        .arg(responseBody.isEmpty() ? QStringLiteral("<empty>") : responseBody);
        return QString();
    }

    error.clear();
    return content;
}

} // namespace

AIAPIClient::AIAPIClient(QObject* parent)
    : QObject(parent)
{
}

QString AIAPIClient::generateShortSummary(const ScoreReport& report) const
{
    return QStringLiteral("评分 %1（%2），违规 %3 次。")
            .arg(report.totalScore, 0, 'f', 1)
            .arg(report.grade)
            .arg(report.violations.size());
}

QString AIAPIClient::generateMockCoachReport(const ScoreReport& report, const QString& fallbackReason) const
{
    QString text;
    text += QStringLiteral("# AI 教练报告（Mock）\n\n");
    if (!fallbackReason.trimmed().isEmpty()) {
        text += QStringLiteral("> fallback: %1\n\n").arg(fallbackReason.trimmed());
    } else {
        text += QStringLiteral("> 当前使用本地 Mock 报告。\n\n");
    }

    text += QStringLiteral("## 总览\n");
    text += QStringLiteral("- 总分：%1\n").arg(report.totalScore, 0, 'f', 1);
    text += QStringLiteral("- 等级：%1\n").arg(report.grade);
    text += QStringLiteral("- 安全风险：%1\n").arg(report.qLearningFeedback.safetyRisk, 0, 'f', 2);
    text += QStringLiteral("- 规则遵守度：%1\n\n").arg(report.qLearningFeedback.ruleCompliance, 0, 'f', 2);

    text += QStringLiteral("## 重点建议\n");
    if (report.coachAdvices.isEmpty()) {
        text += QStringLiteral("- 暂无\n");
    } else {
        for (const CoachAdvice& advice : report.coachAdvices) {
            text += QStringLiteral("- [%1/%2] %3（%4）\n")
                    .arg(advice.category, advice.severity, advice.message, advice.evidence);
        }
    }
    text += QStringLiteral("\n");

    text += QStringLiteral("## 强化学习反馈\n");
    text += QStringLiteral("- reward: %1\n").arg(report.qLearningFeedback.reward, 0, 'f', 4);
    text += QStringLiteral("- terminalPenalty: %1\n").arg(report.qLearningFeedback.terminalPenalty, 0, 'f', 4);
    text += QStringLiteral("- actionHint: %1\n").arg(report.qLearningFeedback.recommendedActionHint);

    return text;
}

QString AIAPIClient::generateCoachReport(const ScoreReport& report) const
{
    const AIClientConfig config = loadConfig();

    if (config.mode == QStringLiteral("mock")) {
        return generateMockCoachReport(report, QStringLiteral("mode=mock"));
    }

    const QString prompt = buildReportPrompt(report);

    auto tryDeepSeek = [&]() -> QPair<QString, QString> {
        QString error;
        const QString content = callChatCompletions(
                QStringLiteral("DeepSeek"),
                config.deepseekBaseUrl,
                config.deepseekModel,
                config.deepseekApiKey,
                config.timeoutMs,
                prompt,
                error);
        return {content, error};
    };

    auto tryZhipu = [&]() -> QPair<QString, QString> {
        QString error;
        const QString content = callChatCompletions(
                QStringLiteral("Zhipu"),
                config.zhipuBaseUrl,
                config.zhipuModel,
                config.zhipuApiKey,
                config.timeoutMs,
                prompt,
                error);
        return {content, error};
    };

    if (config.mode == QStringLiteral("deepseek")) {
        const auto result = tryDeepSeek();
        if (!result.first.isEmpty()) {
            return result.first;
        }
        return generateMockCoachReport(report, QStringLiteral("mode=deepseek failed: %1").arg(result.second));
    }

    if (config.mode == QStringLiteral("zhipu")) {
        const auto result = tryZhipu();
        if (!result.first.isEmpty()) {
            return result.first;
        }
        return generateMockCoachReport(report, QStringLiteral("mode=zhipu failed: %1").arg(result.second));
    }

    QStringList failures;

    if (!config.deepseekApiKey.trimmed().isEmpty()) {
        const auto result = tryDeepSeek();
        if (!result.first.isEmpty()) {
            return result.first;
        }
        failures.append(result.second);
    } else {
        failures.append(QStringLiteral("DeepSeek: missing_api_key"));
    }

    if (!config.zhipuApiKey.trimmed().isEmpty()) {
        const auto result = tryZhipu();
        if (!result.first.isEmpty()) {
            return result.first;
        }
        failures.append(result.second);
    } else {
        failures.append(QStringLiteral("Zhipu: missing_api_key"));
    }

    return generateMockCoachReport(report, failures.join(QStringLiteral("; ")));
}

} // namespace PhantomDrive
