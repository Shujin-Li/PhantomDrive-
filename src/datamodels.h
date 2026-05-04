#ifndef DATAMODELS_H
#define DATAMODELS_H

#include <QString>
#include <QDateTime>
#include <QList>

// 临时数据结构，模拟未来的 ScoreReport
// 用于练习 JSON 存档
struct PracticeReport
{
    QDateTime timestamp;      // 比赛时间
    QString trackName;        // 赛道名称
    double totalScore;        // 总分
    QList<int> scores;        // 各分项得分（如过弯、速度控制等）
    QString aiSuggestion;     // AI 建议
};

#endif // DATAMODELS_H