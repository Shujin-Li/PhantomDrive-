#include "saveloadmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

SaveLoadManager::SaveLoadManager(QObject *parent) : QObject(parent) {}

SaveLoadManager& SaveLoadManager::instance()
{
    static SaveLoadManager instance;
    return instance;
}

QString SaveLoadManager::getFilePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/practice_history.json";
}

// ==================== JSON 序列化辅助函数 ====================

QJsonObject SaveLoadManager::metricsToJson(const PhantomDrive::ScoreMetrics& m) const
{
    QJsonObject obj;
    obj["dataPointCount"] = m.dataPointCount;
    obj["durationMs"] = static_cast<double>(m.durationMs);
    obj["averageSpeed"] = m.averageSpeed;
    obj["maxSpeed"] = m.maxSpeed;
    obj["collisionCount"] = m.collisionCount;
    obj["speedViolationCount"] = m.speedViolationCount;
    obj["redLightViolationCount"] = m.redLightViolationCount;
    obj["pedestrianViolationCount"] = m.pedestrianViolationCount;
    obj["wrongWayViolationCount"] = m.wrongWayViolationCount;
    obj["harshAccelerationCount"] = m.harshAccelerationCount;
    obj["harshBrakingCount"] = m.harshBrakingCount;
    obj["sharpSteeringCount"] = m.sharpSteeringCount;
    obj["overspeedFrameCount"] = m.overspeedFrameCount;
    obj["overspeedRatio"] = m.overspeedRatio;
    return obj;
}

QJsonObject SaveLoadManager::breakdownToJson(const PhantomDrive::ScoreBreakdown& b) const
{
    QJsonObject obj;
    obj["safetyScore"] = b.safetyScore;
    obj["ruleComplianceScore"] = b.ruleComplianceScore;
    obj["smoothnessScore"] = b.smoothnessScore;
    obj["efficiencyScore"] = b.efficiencyScore;
    obj["totalPenalty"] = b.totalPenalty;
    obj["collisionPenalty"] = b.collisionPenalty;
    obj["speedPenalty"] = b.speedPenalty;
    obj["redLightPenalty"] = b.redLightPenalty;
    obj["pedestrianPenalty"] = b.pedestrianPenalty;
    obj["smoothnessPenalty"] = b.smoothnessPenalty;
    return obj;
}

QJsonObject SaveLoadManager::qlearningToJson(const PhantomDrive::QLearningFeedback& q) const
{
    QJsonObject obj;
    obj["reward"] = q.reward;
    obj["normalizedScore"] = q.normalizedScore;
    obj["safetyRisk"] = q.safetyRisk;
    obj["ruleCompliance"] = q.ruleCompliance;
    obj["collisionPenalty"] = q.collisionPenalty;
    obj["speedPenalty"] = q.speedPenalty;
    obj["smoothnessPenalty"] = q.smoothnessPenalty;
    obj["terminalPenalty"] = q.terminalPenalty;
    obj["recommendedActionHint"] = q.recommendedActionHint;
    return obj;
}

QJsonArray SaveLoadManager::violationsToJson(const QList<PhantomDrive::ViolationEvent>& violations) const
{
    QJsonArray arr;
    for (const PhantomDrive::ViolationEvent& v : violations) {
        QJsonObject item;
        item["timestamp"] = QString::number(v.timestamp);
        item["type"] = PhantomDrive::ScoreReport::violationTypeToString(v.type);
        item["description"] = v.description;
        item["positionX"] = v.position.x();
        item["positionY"] = v.position.y();
        item["speedAtViolation"] = v.speedAtViolation;
        item["speedLimit"] = v.speedLimit;
        item["penaltyPoints"] = v.penaltyPoints;
        arr.append(item);
    }
    return arr;
}

QJsonArray SaveLoadManager::advicesToJson(const QList<PhantomDrive::CoachAdvice>& advices) const
{
    QJsonArray arr;
    for (const PhantomDrive::CoachAdvice& advice : advices) {
        QJsonObject item;
        item["category"] = advice.category;
        item["severity"] = advice.severity;
        item["message"] = advice.message;
        item["evidence"] = advice.evidence;
        arr.append(item);
    }
    return arr;
}

QJsonObject SaveLoadManager::reportToJson(const PhantomDrive::ScoreReport& r) const
{
    QJsonObject obj;
    obj["sessionId"] = r.sessionId;
    obj["vehicleId"] = r.vehicleId;
    obj["drivingMode"] = r.drivingMode;
    obj["reportContext"] = r.reportContext;
    obj["generatedAt"] = r.generatedAt.toString(Qt::ISODate);
    obj["totalScore"] = r.totalScore;
    obj["grade"] = r.grade;
    obj["summary"] = r.summary;
    obj["aiCoachReportMarkdown"] = r.aiCoachReportMarkdown;

    obj["metrics"] = metricsToJson(r.metrics);
    obj["breakdown"] = breakdownToJson(r.breakdown);
    obj["violations"] = violationsToJson(r.violations);
    obj["coachAdvices"] = advicesToJson(r.coachAdvices);
    obj["qLearningFeedback"] = qlearningToJson(r.qLearningFeedback);

    return obj;
}

// ==================== JSON 反序列化辅助函数 ====================

PhantomDrive::ScoreMetrics SaveLoadManager::jsonToMetrics(const QJsonObject& obj) const
{
    PhantomDrive::ScoreMetrics m;
    m.dataPointCount = obj["dataPointCount"].toInt();
    m.durationMs = obj["durationMs"].toVariant().toLongLong();
    m.averageSpeed = obj["averageSpeed"].toDouble();
    m.maxSpeed = obj["maxSpeed"].toDouble();
    m.collisionCount = obj["collisionCount"].toInt();
    m.speedViolationCount = obj["speedViolationCount"].toInt();
    m.redLightViolationCount = obj["redLightViolationCount"].toInt();
    m.pedestrianViolationCount = obj["pedestrianViolationCount"].toInt();
    m.wrongWayViolationCount = obj["wrongWayViolationCount"].toInt();
    m.harshAccelerationCount = obj["harshAccelerationCount"].toInt();
    m.harshBrakingCount = obj["harshBrakingCount"].toInt();
    m.sharpSteeringCount = obj["sharpSteeringCount"].toInt();
    m.overspeedFrameCount = obj["overspeedFrameCount"].toInt();
    m.overspeedRatio = obj["overspeedRatio"].toDouble();
    return m;
}

PhantomDrive::ScoreBreakdown SaveLoadManager::jsonToBreakdown(const QJsonObject& obj) const
{
    PhantomDrive::ScoreBreakdown b;
    b.safetyScore = obj["safetyScore"].toDouble();
    b.ruleComplianceScore = obj["ruleComplianceScore"].toDouble();
    b.smoothnessScore = obj["smoothnessScore"].toDouble();
    b.efficiencyScore = obj["efficiencyScore"].toDouble();
    b.totalPenalty = obj["totalPenalty"].toDouble();
    b.collisionPenalty = obj["collisionPenalty"].toDouble();
    b.speedPenalty = obj["speedPenalty"].toDouble();
    b.redLightPenalty = obj["redLightPenalty"].toDouble();
    b.pedestrianPenalty = obj["pedestrianPenalty"].toDouble();
    b.smoothnessPenalty = obj["smoothnessPenalty"].toDouble();
    return b;
}

PhantomDrive::QLearningFeedback SaveLoadManager::jsonToQLearning(const QJsonObject& obj) const
{
    PhantomDrive::QLearningFeedback q;
    q.reward = obj["reward"].toDouble();
    q.normalizedScore = obj["normalizedScore"].toDouble();
    q.safetyRisk = obj["safetyRisk"].toDouble();
    q.ruleCompliance = obj["ruleCompliance"].toDouble();
    q.collisionPenalty = obj["collisionPenalty"].toDouble();
    q.speedPenalty = obj["speedPenalty"].toDouble();
    q.smoothnessPenalty = obj["smoothnessPenalty"].toDouble();
    q.terminalPenalty = obj["terminalPenalty"].toDouble();
    q.recommendedActionHint = obj["recommendedActionHint"].toString();
    return q;
}

QList<PhantomDrive::ViolationEvent> SaveLoadManager::jsonToViolations(const QJsonArray& arr) const
{
    QList<PhantomDrive::ViolationEvent> violations;
    for (const QJsonValue& val : arr) {
        QJsonObject item = val.toObject();
        PhantomDrive::ViolationEvent v;
        v.timestamp = item["timestamp"].toString().toLongLong();
        QString typeStr = item["type"].toString();
        if (typeStr == "RedLight") v.type = PhantomDrive::ViolationType::RedLight;
        else if (typeStr == "SpeedOverLimit") v.type = PhantomDrive::ViolationType::SpeedOverLimit;
        else if (typeStr == "PedestrianCollision") v.type = PhantomDrive::ViolationType::PedestrianCollision;
        else if (typeStr == "WrongWay") v.type = PhantomDrive::ViolationType::WrongWay;
        else if (typeStr == "Collision") v.type = PhantomDrive::ViolationType::Collision;
        else v.type = PhantomDrive::ViolationType::None;
        v.description = item["description"].toString();
        v.position.setX(item["positionX"].toDouble());
        v.position.setY(item["positionY"].toDouble());
        v.speedAtViolation = item["speedAtViolation"].toDouble();
        v.speedLimit = item["speedLimit"].toDouble();
        v.penaltyPoints = item["penaltyPoints"].toInt();
        violations.append(v);
    }
    return violations;
}

QList<PhantomDrive::CoachAdvice> SaveLoadManager::jsonToAdvices(const QJsonArray& arr) const
{
    QList<PhantomDrive::CoachAdvice> advices;
    for (const QJsonValue& val : arr) {
        QJsonObject item = val.toObject();
        PhantomDrive::CoachAdvice advice;
        advice.category = item["category"].toString();
        advice.severity = item["severity"].toString();
        advice.message = item["message"].toString();
        advice.evidence = item["evidence"].toString();
        advices.append(advice);
    }
    return advices;
}

PhantomDrive::ScoreReport SaveLoadManager::jsonToReport(const QJsonObject& obj) const
{
    PhantomDrive::ScoreReport r;
    r.sessionId = obj["sessionId"].toString();
    r.vehicleId = obj["vehicleId"].toString();
    r.drivingMode = obj["drivingMode"].toString();
    r.reportContext = obj["reportContext"].toString();
    r.generatedAt = QDateTime::fromString(obj["generatedAt"].toString(), Qt::ISODate);
    r.totalScore = obj["totalScore"].toDouble();
    r.grade = obj["grade"].toString();
    r.summary = obj["summary"].toString();
    r.aiCoachReportMarkdown = obj["aiCoachReportMarkdown"].toString();

    if (obj.contains("metrics"))
        r.metrics = jsonToMetrics(obj["metrics"].toObject());
    if (obj.contains("breakdown"))
        r.breakdown = jsonToBreakdown(obj["breakdown"].toObject());
    if (obj.contains("violations"))
        r.violations = jsonToViolations(obj["violations"].toArray());
    if (obj.contains("coachAdvices"))
        r.coachAdvices = jsonToAdvices(obj["coachAdvices"].toArray());
    if (obj.contains("qLearningFeedback"))
        r.qLearningFeedback = jsonToQLearning(obj["qLearningFeedback"].toObject());

    return r;
}

// ==================== 文件 I/O ====================

bool SaveLoadManager::saveToFile(const QList<PhantomDrive::ScoreReport>& reports)
{
    QJsonArray historyArr;
    for (const auto& r : reports) {
        historyArr.append(reportToJson(r));
    }

    QJsonObject root;
    root["version"] = 2;  // 升级版本号以区分新旧格式
    root["history"] = historyArr;

    QFile file(getFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for writing:" << file.errorString();
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QList<PhantomDrive::ScoreReport> SaveLoadManager::loadFromFile()
{
    QList<PhantomDrive::ScoreReport> reports;
    QFile file(getFilePath());
    if (!file.exists()) return reports;
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for reading:" << file.errorString();
        return reports;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "SaveLoadManager: JSON parse error:" << parseError.errorString();
        return reports;
    }

    QJsonObject root = doc.object();
    int version = root["version"].toInt(1);

    if (!root.contains("history") || !root["history"].isArray()) {
        qWarning() << "SaveLoadManager: missing or invalid 'history' array in save file";
        return reports;
    }

    QJsonArray historyArr = root["history"].toArray();
    for (auto val : historyArr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        // 兼容旧版 PracticeReport 格式 (version 1)
        if (version == 1) {
            PhantomDrive::ScoreReport r;
            r.sessionId = obj["sessionId"].toString();
            r.vehicleId = "legacy_vehicle";
            r.generatedAt = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
            r.totalScore = obj["totalScore"].toDouble();
            r.grade = PhantomDrive::ScoreReport::gradeFromScore(r.totalScore);
            r.summary = QString("Legacy record: %1").arg(obj["trackName"].toString());

            QJsonArray scoresArr = obj["scores"].toArray();
            if (scoresArr.size() > 0) {
                r.breakdown.safetyScore = scoresArr.at(0).toInt(100);
            }
            if (scoresArr.size() > 1) {
                r.breakdown.ruleComplianceScore = scoresArr.at(1).toInt(100);
            }
            if (scoresArr.size() > 2) {
                r.breakdown.smoothnessScore = scoresArr.at(2).toInt(100);
            }

            // AI 建议转为 CoachAdvice
            QString aiSuggestion = obj["aiSuggestion"].toString();
            if (!aiSuggestion.isEmpty()) {
                PhantomDrive::CoachAdvice advice;
                advice.category = "AI";
                advice.severity = "info";
                advice.message = aiSuggestion;
                advice.evidence = "legacy_import";
                r.coachAdvices.append(advice);
            }

            reports.append(r);
        } else {
            // 新版完整格式
            reports.append(jsonToReport(obj));
        }
    }
    return reports;
}

// ==================== 公共 API ====================

bool SaveLoadManager::saveReport(const PhantomDrive::ScoreReport& report)
{
    if (report.sessionId.isEmpty()) {
        qWarning() << "SaveLoadManager::saveReport: skipping report with empty sessionId";
        return false;
    }
    QList<PhantomDrive::ScoreReport> reports = loadFromFile();
    reports.append(report);
    bool ok = saveToFile(reports);
    if (ok) {
        emit reportSaved(report);
        emit historyChanged();
    }
    return ok;
}

bool SaveLoadManager::updateReportAiCoachReport(const QString& sessionId,
                                                const QString& vehicleId,
                                                const QString& markdown)
{
    const QString trimmedSessionId = sessionId.trimmed();
    const QString trimmedMarkdown = markdown.trimmed();
    if (trimmedSessionId.isEmpty() || trimmedMarkdown.isEmpty()) {
        qWarning() << "SaveLoadManager::updateReportAiCoachReport: empty sessionId or markdown";
        return false;
    }

    QList<PhantomDrive::ScoreReport> reports = loadFromFile();
    for (int i = reports.size() - 1; i >= 0; --i) {
        const bool sessionMatches = reports.at(i).sessionId == trimmedSessionId;
        const bool vehicleMatches = vehicleId.trimmed().isEmpty()
            || reports.at(i).vehicleId == vehicleId.trimmed();
        if (sessionMatches && vehicleMatches) {
            reports[i].aiCoachReportMarkdown = trimmedMarkdown;
            const bool ok = saveToFile(reports);
            if (ok) {
                emit reportUpdated(i, reports.at(i));
                emit historyChanged();
            }
            return ok;
        }
    }

    qWarning() << "SaveLoadManager::updateReportAiCoachReport: report not found"
               << "sessionId=" << trimmedSessionId
               << "vehicleId=" << vehicleId;
    return false;
}

bool SaveLoadManager::saveReportJson(const QJsonObject& reportJson)
{
    const QJsonObject payload = reportJson.contains("report") && reportJson.value("report").isObject()
        ? reportJson.value("report").toObject()
        : reportJson;
    return saveReport(jsonToReport(payload));
}

bool SaveLoadManager::append(const QJsonObject& reportJson)
{
    return saveReportJson(reportJson);
}

QList<PhantomDrive::ScoreReport> SaveLoadManager::loadHistory()
{
    return loadFromFile();
}

bool SaveLoadManager::deleteReport(int index)
{
    QList<PhantomDrive::ScoreReport> reports = loadFromFile();

    if (index < 0 || index >= reports.size()) {
        qWarning() << "Delete failed: invalid index" << index;
        return false;
    }

    reports.removeAt(index);
    bool ok = saveToFile(reports);

    if (ok) {
        emit reportDeleted(index);
        emit historyChanged();
    }

    return ok;
}

bool SaveLoadManager::updateReport(int index, const PhantomDrive::ScoreReport& newReport)
{
    QList<PhantomDrive::ScoreReport> reports = loadFromFile();

    if (index < 0 || index >= reports.size()) {
        qWarning() << "Update failed: invalid index" << index;
        return false;
    }

    reports[index] = newReport;
    bool ok = saveToFile(reports);

    if (ok) {
        emit reportUpdated(index, newReport);
        emit historyChanged();
    }

    return ok;
}

bool SaveLoadManager::exportReport(const PhantomDrive::ScoreReport& report, const QString& filePath)
{
    QJsonObject root;
    root["version"] = 2;
    root["report"] = reportToJson(report);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for export:" << file.errorString();
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool SaveLoadManager::importReport(const QString& filePath, PhantomDrive::ScoreReport& outReport)
{
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "Import file does not exist:" << filePath;
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for import:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in import file:" << filePath;
        return false;
    }

    QJsonObject root = doc.object();
    if (root.contains("report")) {
        outReport = jsonToReport(root["report"].toObject());
    } else {
        // 兼容：直接是报告对象
        outReport = jsonToReport(root);
    }

    return true;
}
