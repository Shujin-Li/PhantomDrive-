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

QJsonObject reportToJson(const PracticeReport& r)
{
    QJsonObject obj;
    obj["timestamp"] = r.timestamp.toString(Qt::ISODate);
    obj["trackName"] = r.trackName;
    obj["totalScore"] = r.totalScore;

    QJsonArray scoresArr;
    for (int s : r.scores) {
        scoresArr.append(s);
    }
    obj["scores"] = scoresArr;
    obj["aiSuggestion"] = r.aiSuggestion;
    return obj;
}

PracticeReport jsonToReport(const QJsonObject& obj)
{
    PracticeReport r;
    r.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
    r.trackName = obj["trackName"].toString();
    r.totalScore = obj["totalScore"].toDouble();

    QJsonArray scoresArr = obj["scores"].toArray();
    for (auto v : scoresArr) {
        r.scores.append(v.toInt());
    }

    r.aiSuggestion = obj["aiSuggestion"].toString();
    return r;
}

bool SaveLoadManager::saveToFile(const QList<PracticeReport>& reports)
{
    QJsonArray historyArr;
    for (const auto& r : reports) {
        historyArr.append(reportToJson(r));
    }

    QJsonObject root;
    root["version"] = 1;
    root["history"] = historyArr;

    QFile file(getFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for writing:" << file.errorString();
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();
    return true;
}

QList<PracticeReport> SaveLoadManager::loadFromFile()
{
    QList<PracticeReport> reports;
    QFile file(getFilePath());
    if (!file.exists()) return reports;
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for reading:" << file.errorString();
        return reports;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) return reports;

    QJsonObject root = doc.object();
    QJsonArray historyArr = root["history"].toArray();
    for (auto val : historyArr) {
        reports.append(jsonToReport(val.toObject()));
    }
    return reports;
}

bool SaveLoadManager::saveReport(const PracticeReport& report)
{
    QList<PracticeReport> reports = loadFromFile();
    reports.append(report);
    bool ok = saveToFile(reports);
    if (ok) {
        emit historyChanged();
    }
    return ok;
}

QList<PracticeReport> SaveLoadManager::loadHistory()
{
    return loadFromFile();
}

bool SaveLoadManager::deleteReport(int index)
{
    // 1. 加载所有现有的历史记录
    QList<PracticeReport> reports = loadFromFile();

    // 2. 检查索引是否有效
    if (index < 0 || index >= reports.size()) {
        qWarning() << "删除失败：索引无效" << index;
        return false;
    }

    // 3. 删除指定索引的记录
    reports.removeAt(index);

    // 4. 保存回文件
    bool ok = saveToFile(reports);

    // 5. 如果保存成功，通知 UI 刷新
    if (ok) {
        emit historyChanged();
        qDebug() << "删除成功，还剩" << reports.size() << "条记录";
    }

    return ok;
}

bool SaveLoadManager::updateReport(int index, const PracticeReport& newReport)
{
    // 1. 加载所有现有的历史记录
    QList<PracticeReport> reports = loadFromFile();

    // 2. 检查索引是否有效
    if (index < 0 || index >= reports.size()) {
        qWarning() << "更新失败：索引无效" << index;
        return false;
    }

    // 3. 替换指定索引的记录
    reports[index] = newReport;

    // 4. 保存回文件
    bool ok = saveToFile(reports);

    // 5. 如果保存成功，通知 UI 刷新
    if (ok) {
        emit historyChanged();
        qDebug() << "更新成功，索引" << index;
    }

    return ok;
}