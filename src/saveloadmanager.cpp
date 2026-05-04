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