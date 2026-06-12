#include "PhantomDrive/scoring/AIAPIClient.h"

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
    prompt += QStringLiteral("请用中文输出一份正式但适合课堂演示的驾驶教练报告，语气专业、具体、可执行。\n");
    prompt += QStringLiteral("必须使用 Markdown 分段，并严格包含这些二级标题：总览、驾驶亮点、主要问题、改进建议、规则遵守分析、速度控制分析、下一次训练目标。\n");
    prompt += QStringLiteral("每个分段至少 2 条要点；如果数据表现优秀，也要说明保持策略，不要只写一句表扬。\n");
    prompt += QStringLiteral("请结合分数、等级、违规事件、平均速度、最高速度、强化学习反馈中的 risk/rules/reward/actionHint 进行分析。\n");
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
    const int violationCount = report.violations.size();
    const qreal avgSpeed = report.metrics.averageSpeed;
    const qreal maxSpeed = report.metrics.maxSpeed;
    const qreal safety = report.breakdown.safetyScore;
    const qreal rules = report.breakdown.ruleComplianceScore;
    const qreal smoothness = report.breakdown.smoothnessScore;
    const qreal efficiency = report.breakdown.efficiencyScore;
    const qreal risk = report.qLearningFeedback.safetyRisk;
    const qreal compliance = report.qLearningFeedback.ruleCompliance;

    QStringList issueLines;
    if (violationCount > 0) {
        issueLines << QStringLiteral("- 本次记录到 %1 次违规或风险事件，需要优先复盘触发位置和发生前 3 秒的操作。").arg(violationCount);
    } else {
        issueLines << QStringLiteral("- 本次没有记录到明确违规，主要问题不在扣分项，而在继续保持注意力和稳定节奏。");
    }
    if (maxSpeed > 100.0) {
        issueLines << QStringLiteral("- 最高速度达到 %1 km/h，峰值速度较高，进入弯道、路口和交通目标区域前应更早收油。").arg(maxSpeed, 0, 'f', 1);
    } else if (avgSpeed > 0.0 && avgSpeed < 25.0) {
        issueLines << QStringLiteral("- 平均速度为 %1 km/h，整体偏谨慎；在安全路段可以更平顺地提速，避免效率分长期停滞。").arg(avgSpeed, 0, 'f', 1);
    } else {
        issueLines << QStringLiteral("- 速度峰值和平均速度处于可控区间，后续重点是减少急加速、急减速造成的节奏波动。");
    }

    QString fullText;
    fullText += QStringLiteral("# AI 教练报告（Mock）\n\n");
    fullText += fallbackReason.trimmed().isEmpty()
        ? QStringLiteral("> 当前使用本地 Mock 报告。\n\n")
        : QStringLiteral("> fallback: %1\n\n").arg(fallbackReason.trimmed());

    fullText += QStringLiteral("## 总览\n");
    fullText += QStringLiteral("- 本次总分为 %1，等级为 %2，整体表现处于“%3”区间。\n")
                    .arg(report.totalScore, 0, 'f', 1)
                    .arg(report.grade.isEmpty() ? QStringLiteral("--") : report.grade)
                    .arg(report.totalScore >= 85.0 ? QStringLiteral("优秀") : report.totalScore >= 70.0 ? QStringLiteral("良好") : QStringLiteral("需要强化"));
    fullText += QStringLiteral("- 平均速度 %1 km/h，最高速度 %2 km/h，安全风险 %3，规则遵守度 %4。\n\n")
                    .arg(avgSpeed, 0, 'f', 1)
                    .arg(maxSpeed, 0, 'f', 1)
                    .arg(risk, 0, 'f', 2)
                    .arg(compliance, 0, 'f', 2);

    fullText += QStringLiteral("## 驾驶亮点\n");
    fullText += QStringLiteral("- 安全分 %1、规则分 %2，说明多数关键操作没有触发严重风险。\n").arg(safety, 0, 'f', 0).arg(rules, 0, 'f', 0);
    fullText += QStringLiteral("- 平顺性 %1、效率 %2，当前节奏具备继续提升速度控制细节的基础。\n\n").arg(smoothness, 0, 'f', 0).arg(efficiency, 0, 'f', 0);

    fullText += QStringLiteral("## 主要问题\n");
    fullText += issueLines.join(QStringLiteral("\n"));
    fullText += QStringLiteral("\n\n");

    fullText += QStringLiteral("## 改进建议\n");
    if (report.coachAdvices.isEmpty()) {
        fullText += QStringLiteral("- 继续采用“提前观察、轻踩油门、分段制动”的驾驶节奏，避免临近目标点再突然修正。\n");
        fullText += QStringLiteral("- 每次进入路口、斑马线、检查点前，先确认限速和通行条件，再决定是否加速。\n");
    } else {
        for (const CoachAdvice& advice : report.coachAdvices) {
            fullText += QStringLiteral("- [%1 / %2] %3%4\n")
                            .arg(advice.category.isEmpty() ? QStringLiteral("Coach") : advice.category,
                                 advice.severity.isEmpty() ? QStringLiteral("info") : advice.severity,
                                 advice.message,
                                 advice.evidence.isEmpty() ? QString() : QStringLiteral("（证据：%1）").arg(advice.evidence));
        }
    }
    fullText += QStringLiteral("\n");

    fullText += QStringLiteral("## 规则遵守分析\n");
    fullText += QStringLiteral("- 规则遵守度为 %1；%2\n")
                    .arg(compliance, 0, 'f', 2)
                    .arg(violationCount == 0 ? QStringLiteral("本次没有明显违规，是报告中最稳定的部分。") : QStringLiteral("建议优先复盘违规类型，找出重复出现的触发条件。"));
    fullText += QStringLiteral("- 下次训练应把红灯、限速、逆行、行人避让作为驾驶前 5 秒预判项，而不是临场补救项。\n\n");

    fullText += QStringLiteral("## 速度控制分析\n");
    fullText += QStringLiteral("- 平均速度 %1 km/h 反映整体推进节奏；最高速度 %2 km/h 反映风险峰值。\n").arg(avgSpeed, 0, 'f', 1).arg(maxSpeed, 0, 'f', 1);
    fullText += QStringLiteral("- 建议用“直道稳定加速、弯前提前减速、出弯轻柔恢复”的节奏，把速度波动控制得更平滑。\n\n");

    fullText += QStringLiteral("## 下一次训练目标\n");
    fullText += QStringLiteral("- 目标一：保持总分不低于 %1，同时把安全风险控制在 %2 以下。\n")
                    .arg(qMax(80.0, report.totalScore - 2.0), 0, 'f', 0)
                    .arg(qMax(0.05, risk + 0.05), 0, 'f', 2);
    fullText += QStringLiteral("- 目标二：连续完成 2 圈或 2 次路线训练，保持零碰撞、零闯灯，并减少不必要的急加速。\n");
    fullText += QStringLiteral("- 强化学习反馈：reward %1，terminalPenalty %2，actionHint `%3`。\n")
                    .arg(report.qLearningFeedback.reward, 0, 'f', 4)
                    .arg(report.qLearningFeedback.terminalPenalty, 0, 'f', 4)
                    .arg(report.qLearningFeedback.recommendedActionHint.isEmpty() ? QStringLiteral("keep_smooth_and_stable") : report.qLearningFeedback.recommendedActionHint);

    return fullText;

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
