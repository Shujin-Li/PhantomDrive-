#ifndef SAVELOADMANAGER_H
#define SAVELOADMANAGER_H

#include <QObject>
#include <QList>
#include "datamodels.h"

class SaveLoadManager : public QObject
{
    Q_OBJECT
public:
    static SaveLoadManager& instance();   // 单例

    bool saveReport(const PracticeReport& report);
    QList<PracticeReport> loadHistory();

    bool deleteReport(int index);
    bool updateReport(int index, const PracticeReport& newReport);

signals:
    void historyChanged();   // 历史数据变化时发射

private:
    explicit SaveLoadManager(QObject *parent = nullptr);
    QString getFilePath() const;

    bool saveToFile(const QList<PracticeReport>& reports);
    QList<PracticeReport> loadFromFile();
};

#endif // SAVELOADMANAGER_H