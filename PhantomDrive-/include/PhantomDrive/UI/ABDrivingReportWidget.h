#ifndef ABDRIVINGREPORTWIDGET_H
#define ABDRIVINGREPORTWIDGET_H

#include <QList>
#include <QTableWidget>
#include <QTimer>
#include <QWidget>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>

class QLabel;

namespace PhantomDrive {

struct SpeedSample {
    qreal speed = 0.0;
    qint64 timestamp = 0;
};

struct ViolationItem {
    QString type;
    QString description;
    qreal speed = 0.0;
    qreal limit = 0.0;
    int penalty = 0;
    qint64 timestamp = 0;
};

struct ReportSummary {
    int totalScore = 0;
    QString grade;
    qreal averageSpeed = 0.0;
    qreal maxSpeed = 0.0;
    QString coachAdvice;
    QList<ViolationItem> violations;
};

class ABDrivingReportWidget : public QWidget
{
public:
    explicit ABDrivingReportWidget(QWidget* parent = nullptr);
    ~ABDrivingReportWidget() override;

    void addSpeedData(qreal speed, qint64 timestamp = -1);
    void setReportSummary(const ReportSummary& summary);
    void setHistoryReports(const QList<ReportSummary>& reports);
    void clearData();
    void setMockDataEnabled(bool enabled);

private:
    void setupUI();
    void applyDarkTheme();
    void updateMockData();
    void updateStatsFromSamples();
    void updateBreakdownChart(const ReportSummary& summary);
    void updateViolationTable(const QList<ViolationItem>& violations);
    void updateCoachAdvice(const QString& advice);
    void resetBreakdownChart();

    static qreal boundedScore(qreal value);
    static QString formattedTime(qint64 timestamp);

    QSplineSeries* m_speedSeries;
    QChart* m_speedChart;
    QChartView* m_speedChartView;
    QValueAxis* m_speedAxisX;
    QValueAxis* m_speedAxisY;

    QBarSeries* m_breakdownBarSeries;
    QChart* m_breakdownChart;
    QChartView* m_breakdownChartView;

    QLineSeries* m_historyScoreSeries;
    QChart* m_historyChart;
    QChartView* m_historyChartView;
    QValueAxis* m_historyAxisX;
    QValueAxis* m_historyAxisY;

    QLabel* m_currentSpeedLabel;
    QLabel* m_avgSpeedLabel;
    QLabel* m_maxSpeedLabel;
    QLabel* m_totalScoreLabel;
    QLabel* m_gradeLabel;
    QLabel* m_violationCountLabel;
    QLabel* m_aiReportLabel;

    QTableWidget* m_violationTable;
    QTimer* m_timer;

    QList<SpeedSample> m_speedSamples;
    QList<ViolationItem> m_violations;
    QList<ReportSummary> m_historyReports;
    ReportSummary m_currentSummary;

    qint64 m_nextMockTimestamp;
    bool m_useMockData;
};

} // namespace PhantomDrive

#endif // ABDRIVINGREPORTWIDGET_H
