#ifndef DRIVINGREPORTWIDGET_H
#define DRIVINGREPORTWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAbstractSeries>

#include "PhantomDrive_global.h"
#include "datamodels.h"

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT DrivingReportWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrivingReportWidget(QWidget *parent = nullptr);
    ~DrivingReportWidget();

    void addSpeedData(qreal speed, qint64 timestamp = -1);
    void setCurrentReport(const ScoreReport& report);
    void setCoachReportMarkdown(const QString& markdown);
    void addViolationEvent(const ViolationEvent& violation);
    void clearData();
    void setMockDataEnabled(bool enabled);
    
    // 历史数据加载
    void loadHistoryFromSaveLoadManager();
    void loadHistoryReports(const QList<ScoreReport>& reports);

signals:
    void reportUpdated(const ScoreReport& report);

private slots:
    void updateMockData();

private:
    void setupUI();
    void updateBreakdownChart();
    void updateViolationTable();
    void updateCoachAdvice();

    // 速度图表
    QSplineSeries *m_speedSeries;
    QChart *m_speedChart;
    QChartView *m_speedChartView;
    QValueAxis *m_speedAxisX;
    QValueAxis *m_speedAxisY;

    // 分项得分
    QBarSeries *m_breakdownBarSeries;
    QChart *m_breakdownChart;
    QChartView *m_breakdownChartView;

    // 统计数据
    QLabel *m_avgSpeedLabel;
    QLabel *m_maxSpeedLabel;
    QLabel *m_currentSpeedLabel;
    QLabel *m_totalScoreLabel;
    QLabel *m_gradeLabel;
    QLabel *m_violationCountLabel;

    // 违规事件表
    QTableWidget *m_violationTable;

    // 教练建议
    QWidget *m_coachAdviceWidget;
    QLabel *m_aiReportLabel;
    
    // 历史数据图表
    QLineSeries *m_historyScoreSeries;
    QLineSeries *m_historySafetySeries;
    QLineSeries *m_historyRuleSeries;
    QLineSeries *m_historySmoothSeries;
    QLineSeries *m_historyEfficiencySeries;
    QChart *m_historyChart;
    QChartView *m_historyChartView;
    QValueAxis *m_historyAxisX;
    QValueAxis *m_historyAxisY;
    QList<ScoreReport> m_historyReports;

    // 定时器
    QTimer *m_timer;
    int m_tick;

    // 数据缓存
    qreal m_totalSpeed;
    int m_speedDataCount;
    qreal m_maxRecordedSpeed;
    QList<ViolationEvent> m_violations;
    ScoreReport m_currentReport;
    bool m_useMockData;
};

} // namespace PhantomDrive

#endif // DRIVINGREPORTWIDGET_H
