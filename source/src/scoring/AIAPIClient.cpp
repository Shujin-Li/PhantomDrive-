#include "PhantomDrive/scoring/AIAPIClient.h"

#include <QDebug>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QTimer>
#include <QUrl>

namespace PhantomDrive {

namespace {

struct AIClientConfig {
    QString mode;
    int timeoutMs = 30000;

    QString deepseekApiKey;
    QString deepseekBaseUrl;
    QString deepseekModel;

    QString zhipuApiKey;
    QString zhipuBaseUrl;
    QString zhipuModel;
};

constexpr int DefaultTimeoutMs = 30000;

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

bool isUsableApiKey(const QString& value)
{
    const QString trimmed = value.trimmed();
    return !trimmed.isEmpty()
           && !trimmed.contains(QStringLiteral("PUT_YOUR"), Qt::CaseInsensitive)
           && !trimmed.contains(QStringLiteral("PASTE_YOUR"), Qt::CaseInsensitive)
           && !trimmed.contains(QStringLiteral("YOUR_"), Qt::CaseInsensitive);
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

QString stripTrailingSlash(QString url)
{
    url = url.trimmed();
    while (url.endsWith('/')) {
        url.chop(1);
    }
    return url;
}

QString buildChatCompletionsEndpoint(const QString& baseUrl, QString& error)
{
    QString normalized = stripTrailingSlash(baseUrl);
    if (normalized.isEmpty()) {
        error = QStringLiteral("empty_base_url");
        return QString();
    }

    if (!normalized.endsWith(QStringLiteral("/chat/completions"), Qt::CaseInsensitive)) {
        normalized += QStringLiteral("/chat/completions");
    }

    const QUrl endpoint(normalized);
    if (!endpoint.isValid() || endpoint.scheme().isEmpty() || endpoint.host().isEmpty()) {
        error = QStringLiteral("invalid_endpoint=%1").arg(normalized);
        return QString();
    }

    error.clear();
    return endpoint.toString(QUrl::RemoveQuery | QUrl::RemoveFragment);
}

AIClientConfig loadConfig()
{
    AIClientConfig config;
    config.mode = normalizeMode(readEnv("PHANTOMDRIVE_AI_MODE", QStringLiteral("auto")));
    config.timeoutMs = readEnvInt("PHANTOMDRIVE_AI_TIMEOUT_MS", DefaultTimeoutMs);

    config.deepseekApiKey = readEnv("DEEPSEEK_API_KEY");
    config.deepseekBaseUrl = stripTrailingSlash(
        readEnv("DEEPSEEK_BASE_URL", QStringLiteral("https://api.deepseek.com")));
    config.deepseekModel = readEnv("DEEPSEEK_MODEL", QStringLiteral("deepseek-v4-flash"));

    config.zhipuApiKey = readEnv("ZHIPU_API_KEY");
    config.zhipuBaseUrl = stripTrailingSlash(
        readEnv("ZHIPU_BASE_URL", QStringLiteral("https://api.z.ai/api/paas/v4")));
    config.zhipuModel = readEnv("ZHIPU_MODEL", QStringLiteral("glm-4.7-flash"));
    return config;
}

void logEffectiveConfig(const AIClientConfig& config)
{
    qInfo().noquote()
        << QStringLiteral("[AIAPIClient] effective config: mode=%1 deepseekKey=%2 deepseekKeyLen=%3 deepseekBaseUrl=%4 deepseekModel=%5 zhipuKey=%6 zhipuKeyLen=%7 zhipuBaseUrl=%8 zhipuModel=%9 timeoutMs=%10")
               .arg(config.mode,
                    isUsableApiKey(config.deepseekApiKey) ? QStringLiteral("true") : QStringLiteral("false"))
               .arg(config.deepseekApiKey.trimmed().size())
               .arg(config.deepseekBaseUrl,
                    config.deepseekModel,
                    isUsableApiKey(config.zhipuApiKey) ? QStringLiteral("true") : QStringLiteral("false"))
               .arg(config.zhipuApiKey.trimmed().size())
               .arg(config.zhipuBaseUrl,
                    config.zhipuModel)
               .arg(config.timeoutMs);
}

QString buildReportPrompt(const ScoreReport& report)
{
    const QByteArray reportJson = QJsonDocument(report.toJson()).toJson(QJsonDocument::Compact);

    QString prompt;
    prompt += QStringLiteral("Write a professional driving coach report in Simplified Chinese.\n");
    prompt += QStringLiteral("Use Markdown. Include these level-2 sections exactly: 总览, 驾驶亮点, 主要问题, 改进建议, 规则遵守分析, 速度控制分析, 下一次训练目标.\n");
    prompt += QStringLiteral("Each section must contain at least two concrete bullet points. If the score is high, explain how to keep the good habits instead of only praising.\n");
    prompt += QStringLiteral("Analyze score, grade, violations, average speed, max speed, and qLearningFeedback fields such as safetyRisk, ruleCompliance, reward, terminalPenalty, and recommendedActionHint.\n");
    if (!report.drivingMode.trimmed().isEmpty() || !report.reportContext.trimmed().isEmpty()) {
        prompt += QStringLiteral("Mode context:\n");
        prompt += QStringLiteral("- drivingMode: %1\n").arg(report.drivingMode.trimmed().isEmpty() ? QStringLiteral("Unknown") : report.drivingMode.trimmed());
        prompt += QStringLiteral("- reportContext: %1\n").arg(report.reportContext.trimmed().isEmpty() ? QStringLiteral("None") : report.reportContext.trimmed());
        prompt += QStringLiteral("Adjust the advice to this mode: Learning should emphasize traffic-rule practice; Arcade should emphasize race pace, powerups, and lap consistency; Custom Track should emphasize checkpoints, finish route discipline, and track layout; Two-Player should include player-specific comparison/fair racing advice; Adaptive AI should mention AI difficulty adaptation and how the driver can respond.\n");
    }
    prompt += QStringLiteral("Here is the ScoreReport JSON:\n");
    prompt += QString::fromUtf8(reportJson);
    return prompt;
}

QString extractContentFromReply(const QByteArray& body, QString& error, QString& finishReason)
{
    finishReason.clear();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        error = QStringLiteral("invalid_json_reply");
        return QString();
    }

    const QJsonObject root = document.object();
    if (root.contains(QStringLiteral("error"))) {
        const QJsonObject errorObject = root.value(QStringLiteral("error")).toObject();
        error = QStringLiteral("api_error=%1")
                    .arg(errorObject.value(QStringLiteral("message")).toString(QStringLiteral("unknown_error")));
        return QString();
    }

    const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        error = QStringLiteral("empty_choices");
        return QString();
    }

    const QJsonObject choice = choices.first().toObject();
    finishReason = choice.value(QStringLiteral("finish_reason")).toString();
    const QJsonObject message = choice.value(QStringLiteral("message")).toObject();
    QString content = message.value(QStringLiteral("content")).toString().trimmed();
    if (content.isEmpty()) {
        content = choice.value(QStringLiteral("text")).toString().trimmed();
    }
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
    if (!isUsableApiKey(apiKey)) {
        error = providerName + QStringLiteral(": missing_api_key");
        return QString();
    }

    QString endpointError;
    const QString endpointUrl = buildChatCompletionsEndpoint(baseUrl, endpointError);
    if (!endpointError.isEmpty()) {
        error = providerName + QStringLiteral(": %1").arg(endpointError);
        return QString();
    }
    const QUrl endpoint(endpointUrl);

    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());
    request.setTransferTimeout(timeoutMs);

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), model);
    payload.insert(QStringLiteral("temperature"), 0.2);
    payload.insert(QStringLiteral("max_tokens"), 3000);
    payload.insert(QStringLiteral("stream"), false);

    QJsonArray messages;
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("system")},
        {QStringLiteral("content"),
         QStringLiteral("You are a driving coach assistant. Output clear, structured, actionable Simplified Chinese.")}
    });
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("user")},
        {QStringLiteral("content"), prompt}
    });
    payload.insert(QStringLiteral("messages"), messages);

    qInfo().noquote()
        << QStringLiteral("[AIAPIClient] Requesting coach report %1 url=%2 model=%3 timeoutMs=%4")
               .arg(providerName, endpoint.toString(), model)
               .arg(timeoutMs);

    QNetworkAccessManager manager;
    QNetworkReply* reply = manager.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start(timeoutMs);
    loop.exec();

    if (!reply) {
        error = providerName + QStringLiteral(": null_reply, url=%1").arg(endpoint.toString());
        qWarning() << "[AIAPIClient]" << error;
        return QString();
    }

    const bool timedOut = !timer.isActive();
    if (timer.isActive()) {
        timer.stop();
    }

    if (timedOut) {
        error = providerName + QStringLiteral(": network_error=timeout, url=%1").arg(endpoint.toString());
        reply->deleteLater();
        qWarning() << "[AIAPIClient]" << error;
        return QString();
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray rawBody = reply->readAll();
    const QString responseBody = QString::fromUtf8(rawBody).left(800).trimmed();

    if (reply->error() != QNetworkReply::NoError) {
        error = providerName
                + QStringLiteral(": url=%1, http_status=%2, network_error=%3, body=%4")
                      .arg(endpoint.toString())
                      .arg(statusCode)
                      .arg(reply->errorString())
                      .arg(responseBody.isEmpty() ? QStringLiteral("<empty>") : responseBody);
        reply->deleteLater();
        qWarning() << "[AIAPIClient]" << error;
        return QString();
    }
    reply->deleteLater();

    QString parseError;
    QString finishReason;
    const QString content = extractContentFromReply(rawBody, parseError, finishReason);
    if (content.isEmpty()) {
        error = providerName
                + QStringLiteral(": url=%1, http_status=%2, parse_error=%3, body=%4")
                      .arg(endpoint.toString())
                      .arg(statusCode)
                      .arg(parseError)
                      .arg(responseBody.isEmpty() ? QStringLiteral("<empty>") : responseBody);
        qWarning() << "[AIAPIClient]" << error;
        return QString();
    }

    qInfo() << "[AIAPIClient] Coach report received from" << providerName
            << "chars=" << content.size()
            << "finishReason=" << (finishReason.isEmpty() ? QStringLiteral("<missing>") : finishReason);
    if (finishReason == QStringLiteral("length")) {
        qWarning() << "[AIAPIClient] Coach report may be truncated by provider token limit"
                   << "provider=" << providerName
                   << "chars=" << content.size();
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
    return QStringLiteral("Score %1 (%2), violations %3")
        .arg(report.totalScore, 0, 'f', 1)
        .arg(report.grade)
        .arg(report.violations.size());
}

QString AIAPIClient::generateMockCoachReport(const ScoreReport& report, const QString& fallbackReason) const
{
    QString text;
    text += QStringLiteral("# AI Coach Report (Local Fallback)\n\n");
    if (!fallbackReason.trimmed().isEmpty()) {
        text += QStringLiteral("> AI API fallback reason: %1\n\n").arg(fallbackReason.trimmed());
    } else {
        text += QStringLiteral("> AI API was not used; showing local fallback advice.\n\n");
    }

    text += QStringLiteral("## 总览\n");
    text += QStringLiteral("- 本次总分 %1，等级 %2。\n")
                .arg(report.totalScore, 0, 'f', 1)
                .arg(report.grade.isEmpty() ? QStringLiteral("--") : report.grade);
    if (!report.drivingMode.trimmed().isEmpty() || !report.reportContext.trimmed().isEmpty()) {
        text += QStringLiteral("- 模式：%1；上下文：%2。\n")
                    .arg(report.drivingMode.trimmed().isEmpty() ? QStringLiteral("Unknown") : report.drivingMode.trimmed(),
                         report.reportContext.trimmed().isEmpty() ? QStringLiteral("None") : report.reportContext.trimmed());
    }
    text += QStringLiteral("- 平均速度 %1 km/h，最高速度 %2 km/h，违规 %3 次。\n\n")
                .arg(report.metrics.averageSpeed, 0, 'f', 1)
                .arg(report.metrics.maxSpeed, 0, 'f', 1)
                .arg(report.violations.size());

    text += QStringLiteral("## 驾驶亮点\n");
    text += QStringLiteral("- 安全分 %1，规则分 %2。\n")
                .arg(report.breakdown.safetyScore, 0, 'f', 0)
                .arg(report.breakdown.ruleComplianceScore, 0, 'f', 0);
    text += QStringLiteral("- 平顺分 %1，效率分 %2，可以继续保持稳定节奏。\n\n")
                .arg(report.breakdown.smoothnessScore, 0, 'f', 0)
                .arg(report.breakdown.efficiencyScore, 0, 'f', 0);

    text += QStringLiteral("## 主要问题\n");
    if (report.violations.isEmpty()) {
        text += QStringLiteral("- 本次没有明显违规，重点是保持观察和控速习惯。\n");
        text += QStringLiteral("- 若最高速度偏高，进入弯道或检查点前仍要提前收油。\n\n");
    } else {
        text += QStringLiteral("- 本次记录到 %1 次违规或风险事件。\n").arg(report.violations.size());
        text += QStringLiteral("- 建议复盘违规前 3 秒的速度、转向和制动输入。\n\n");
    }

    text += QStringLiteral("## 改进建议\n");
    if (report.coachAdvices.isEmpty()) {
        text += QStringLiteral("- 继续使用提前观察、轻踩油门、分段制动的驾驶节奏。\n");
        text += QStringLiteral("- 每次进入路口、限速区或检查点前，先确认规则再加速。\n\n");
    } else {
        for (const CoachAdvice& advice : report.coachAdvices) {
            text += QStringLiteral("- [%1 / %2] %3%4\n")
                        .arg(advice.category.isEmpty() ? QStringLiteral("Coach") : advice.category,
                             advice.severity.isEmpty() ? QStringLiteral("info") : advice.severity,
                             advice.message,
                             advice.evidence.isEmpty()
                                 ? QString()
                                 : QStringLiteral("（证据：%1）").arg(advice.evidence));
        }
        text += QStringLiteral("\n");
    }

    text += QStringLiteral("## 规则遵守分析\n");
    text += QStringLiteral("- 规则遵守反馈值为 %1。\n")
                .arg(report.qLearningFeedback.ruleCompliance, 0, 'f', 2);
    text += QStringLiteral("- 下一次训练请把红灯、限速、逆行和行人避让作为驾驶前 5 秒预判项。\n\n");

    text += QStringLiteral("## 速度控制分析\n");
    text += QStringLiteral("- 平均速度反映整体推进节奏，最高速度反映风险峰值。\n");
    text += QStringLiteral("- 建议直道稳定加速、弯前提前减速、出弯轻柔恢复速度。\n\n");

    text += QStringLiteral("## 下一次训练目标\n");
    text += QStringLiteral("- 保持总分不低于 %1，同时减少急加速和急刹。\n")
                .arg(qMax(80.0, report.totalScore - 2.0), 0, 'f', 0);
    text += QStringLiteral("- 强化学习反馈：reward %1, terminalPenalty %2, actionHint `%3`。\n")
                .arg(report.qLearningFeedback.reward, 0, 'f', 4)
                .arg(report.qLearningFeedback.terminalPenalty, 0, 'f', 4)
                .arg(report.qLearningFeedback.recommendedActionHint.isEmpty()
                         ? QStringLiteral("keep_smooth_and_stable")
                         : report.qLearningFeedback.recommendedActionHint);

    return text;
}

QString AIAPIClient::generateCoachReport(const ScoreReport& report) const
{
    const AIClientConfig config = loadConfig();

    logEffectiveConfig(config);

    if (config.mode == QStringLiteral("mock")) {
        return generateMockCoachReport(report, QStringLiteral("mode=mock"));
    }

    const QString prompt = buildReportPrompt(report);

    auto tryDeepSeek = [&]() -> QPair<QString, QString> {
        QString error;
        const QString content = callChatCompletions(QStringLiteral("DeepSeek"),
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
        const QString content = callChatCompletions(QStringLiteral("Zhipu"),
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
    if (isUsableApiKey(config.deepseekApiKey)) {
        const auto result = tryDeepSeek();
        if (!result.first.isEmpty()) {
            return result.first;
        }
        failures.append(result.second);
    } else {
        failures.append(QStringLiteral("DeepSeek: missing_api_key"));
    }

    if (isUsableApiKey(config.zhipuApiKey)) {
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
