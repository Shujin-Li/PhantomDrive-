#ifndef SAVELOADMANAGER_H
#define SAVELOADMANAGER_H

#include <QObject>
#include <QList>
#include <QJsonArray>
#include <QJsonObject>
#include "datamodels.h"
#include "PhantomDrive_global.h"

class PHANTOMDRIVE_EXPORT SaveLoadManager : public QObject
{
    Q_OBJECT
public:
    static SaveLoadManager& instance();   // 单例

    // 使用 PhantomDrive::ScoreReport (即 PracticeReport 的别名)
    bool saveReport(const PhantomDrive::ScoreReport& report);
    bool updateReportAiCoachReport(const QString& sessionId,
                                   const QString& vehicleId,
                                   const QString& markdown);
    QList<PhantomDrive::ScoreReport> loadHistory();

    bool deleteReport(int index);
    bool updateReport(int index, const PhantomDrive::ScoreReport& newReport);

    // 新增：导出到指定文件
    bool exportReport(const PhantomDrive::ScoreReport& report, const QString& filePath);
    bool importReport(const QString& filePath, PhantomDrive::ScoreReport& outReport);

public slots:
    bool saveReportJson(const QJsonObject& reportJson);
    bool append(const QJsonObject& reportJson);

signals:
    void historyChanged();   // 历史数据变化时发射
    void reportSaved(const PhantomDrive::ScoreReport& report);
    void reportDeleted(int index);
    void reportUpdated(int index, const PhantomDrive::ScoreReport& newReport);

public:
    // 获取完整报告的 JSON
    QJsonObject reportToJson(const PhantomDrive::ScoreReport& r) const;
    // 从 JSON 解析报告
    PhantomDrive::ScoreReport jsonToReport(const QJsonObject& obj) const;

private:
    explicit SaveLoadManager(QObject *parent = nullptr);
    QString getFilePath() const;

    bool saveToFile(const QList<PhantomDrive::ScoreReport>& reports);
    QList<PhantomDrive::ScoreReport> loadFromFile();

    // JSON 序列化辅助
    QJsonObject metricsToJson(const PhantomDrive::ScoreMetrics& m) const;
    QJsonObject breakdownToJson(const PhantomDrive::ScoreBreakdown& b) const;
    QJsonObject qlearningToJson(const PhantomDrive::QLearningFeedback& q) const;
    QJsonArray violationsToJson(const QList<PhantomDrive::ViolationEvent>& violations) const;
    QJsonArray advicesToJson(const QList<PhantomDrive::CoachAdvice>& advices) const;

    PhantomDrive::ScoreMetrics jsonToMetrics(const QJsonObject& obj) const;
    PhantomDrive::ScoreBreakdown jsonToBreakdown(const QJsonObject& obj) const;
    PhantomDrive::QLearningFeedback jsonToQLearning(const QJsonObject& obj) const;
    QList<PhantomDrive::ViolationEvent> jsonToViolations(const QJsonArray& arr) const;
    QList<PhantomDrive::CoachAdvice> jsonToAdvices(const QJsonArray& arr) const;
};

#endif // SAVELOADMANAGER_H
