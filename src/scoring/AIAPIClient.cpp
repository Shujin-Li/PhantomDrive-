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
    prompt += QStringLiteral("你是驾驶教练。请仅输出中文 Markdown，并严格使用以下标题与顺序：\n");
    prompt += QStringLiteral("# 驾驶教练报告\n");
    prompt += QStringLiteral("## 总评\n");
    prompt += QStringLiteral("## 主要问题\n");
    prompt += QStringLiteral("## 优先改进建议\n");
    prompt += QStringLiteral("## 下一次训练计划\n");
    prompt += QStringLiteral("## 风险提醒\n\n");
    prompt += QStringLiteral("写作约束：\n");
    prompt += QStringLiteral("1) 禁止出现这些表述：根据、结合、你提供的输入、评分JSON、数据来源。\n");
    prompt += QStringLiteral("2) 禁止提及或输出内部字段名，尤其是 QLearningFeedback、reward、terminalPenalty、recommendedActionHint。\n");
    prompt += QStringLiteral("3) 在“主要问题”和“优先改进建议”中，每条都必须包含三部分：问题是什么；为什么危险或影响分数；下一次具体怎么做。\n");
    prompt += QStringLiteral("4) 输出中不要解释你的信息来源。\n\n");
    prompt += QStringLiteral("结构化报告数据（仅用于分析，不要在正文提及来源）：\n<report_json>");
    prompt += QString::fromUtf8(reportJson);
    prompt += QStringLiteral("</report_json>");
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

bool containsAnyBannedToken(const QString& text)
{
    static const QStringList kBanned = {
        QStringLiteral("根据"),
        QStringLiteral("结合"),
        QStringLiteral("你提供的输入"),
        QStringLiteral("评分JSON"),
        QStringLiteral("数据来源"),
        QStringLiteral("QLearningFeedback"),
        QStringLiteral("reward"),
        QStringLiteral("terminalPenalty"),
        QStringLiteral("recommendedActionHint")
    };
    for (const QString& token : kBanned) {
        if (text.contains(token, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
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
    const bool hasSpeedRisk = report.metrics.speedViolationCount > 0 || report.metrics.overspeedRatio > 0.12;
    const bool hasSafetyRisk = report.metrics.collisionCount > 0 || report.metrics.pedestrianViolationCount > 0;

    text += QStringLiteral("# 驾驶教练报告\n\n");
    if (!fallbackReason.trimmed().isEmpty()) {
        text += QStringLiteral("> 本地报告模式：%1\n\n").arg(fallbackReason.trimmed());
    }

    text += QStringLiteral("## 总评\n");
    text += QStringLiteral("- 总分：%1\n").arg(report.totalScore, 0, 'f', 1);
    text += QStringLiteral("- 等级：%1\n").arg(report.grade);
    text += QStringLiteral("- 当前重点：%1\n\n")
            .arg(hasSafetyRisk ? QStringLiteral("先控制安全风险，再提升规则稳定性")
                               : QStringLiteral("保持稳定基础，重点优化规则执行细节"));

    text += QStringLiteral("## 主要问题\n");
    if (hasSafetyRisk) {
        text += QStringLiteral("- 问题是什么：出现碰撞或行人相关违规。\n");
        text += QStringLiteral("  为什么危险或影响分数：会直接拉低安全得分，且真实道路风险最高。\n");
        text += QStringLiteral("  下一次具体怎么做：进入路口前提早减速，保持可刹停速度，视线提前覆盖行人过街区域。\n");
    }
    if (hasSpeedRisk) {
        text += QStringLiteral("- 问题是什么：限速区内存在超速行为或连续超速帧。\n");
        text += QStringLiteral("  为什么危险或影响分数：会降低规则遵守得分，并增加反应与制动压力。\n");
        text += QStringLiteral("  下一次具体怎么做：看见限速标志后 2 秒内把车速收至限速以下，优先轻收油再轻刹。\n");
    }
    if (!hasSafetyRisk && !hasSpeedRisk) {
        text += QStringLiteral("- 问题是什么：高风险违规较少，但稳定性细节有波动。\n");
        text += QStringLiteral("  为什么危险或影响分数：细小波动会持续损耗平顺性和效率分。\n");
        text += QStringLiteral("  下一次具体怎么做：保持固定跟车节奏，弯前一次性完成减速，减少无效加减速。\n");
    }
    text += QStringLiteral("\n");

    text += QStringLiteral("## 优先改进建议\n");
    text += QStringLiteral("- 问题是什么：进路口前速度控制不稳定。\n");
    text += QStringLiteral("  为什么危险或影响分数：容易叠加红灯/行人风险，并拉低安全与规则分。\n");
    text += QStringLiteral("  下一次具体怎么做：建立“看灯-松油-覆盖刹车”三步动作，路口前至少提前 30 米执行。\n");
    text += QStringLiteral("- 问题是什么：限速区速度回落慢。\n");
    text += QStringLiteral("  为什么危险或影响分数：超速事件和超速帧会持续影响规则维度。\n");
    text += QStringLiteral("  下一次具体怎么做：入限速区第一秒检查速度表，超限时先收油 1 秒再轻刹到目标速度。\n\n");

    text += QStringLiteral("## 下一次训练计划\n");
    text += QStringLiteral("1. 10 分钟限速跟随训练：每次进入限速区都在 2 秒内稳定到限速以下。\n");
    text += QStringLiteral("2. 10 分钟路口预判训练：黄灯/行人区域提前减速，不做最后一刻制动。\n");
    text += QStringLiteral("3. 10 分钟平顺控制训练：把急加速和急刹车次数控制在可复盘范围。\n\n");

    text += QStringLiteral("## 风险提醒\n");
    text += QStringLiteral("- 连续超速并伴随急刹时，碰撞风险会明显上升。\n");
    text += QStringLiteral("- 路口观察滞后时，红灯和行人相关违规会在短时间内集中出现。\n");

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

    auto sanitizeOrFallback = [&](const QString& content, const QString& reason) {
        if (content.isEmpty() || containsAnyBannedToken(content)) {
            return generateMockCoachReport(report, reason);
        }
        return content;
    };

    if (config.mode == QStringLiteral("deepseek")) {
        const auto result = tryDeepSeek();
        return sanitizeOrFallback(result.first,
                                  QStringLiteral("mode=deepseek failed: %1").arg(result.second));
    }

    if (config.mode == QStringLiteral("zhipu")) {
        const auto result = tryZhipu();
        return sanitizeOrFallback(result.first,
                                  QStringLiteral("mode=zhipu failed: %1").arg(result.second));
    }

    QStringList failures;

    if (!config.deepseekApiKey.trimmed().isEmpty()) {
        const auto result = tryDeepSeek();
        if (!result.first.isEmpty() && !containsAnyBannedToken(result.first)) {
            return result.first;
        }
        failures.append(result.second.isEmpty() ? QStringLiteral("DeepSeek: banned_content_or_empty") : result.second);
    } else {
        failures.append(QStringLiteral("DeepSeek: missing_api_key"));
    }

    if (!config.zhipuApiKey.trimmed().isEmpty()) {
        const auto result = tryZhipu();
        if (!result.first.isEmpty() && !containsAnyBannedToken(result.first)) {
            return result.first;
        }
        failures.append(result.second.isEmpty() ? QStringLiteral("Zhipu: banned_content_or_empty") : result.second);
    } else {
        failures.append(QStringLiteral("Zhipu: missing_api_key"));
    }

    return generateMockCoachReport(report, failures.join(QStringLiteral("; ")));
}

} // namespace PhantomDrive
