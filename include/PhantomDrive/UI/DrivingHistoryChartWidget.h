#ifndef DRIVINGHISTORYCHARTWIDGET_H
#define DRIVINGHISTORYCHARTWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarCategoryAxis>

#include "datamodels.h"
#include "PhantomDrive_global.h"

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT DrivingHistoryChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrivingHistoryChartWidget(QWidget *parent = nullptr);

    void updateHistory(const QList<ScoreReport>& history);
    void addReport(const ScoreReport& report);
    void clear();
    QList<ScoreReport> getHistory() const { return m_history; }

signals:
    void reportSelected(int index, const ScoreReport& report);

private slots:
    void onReportSelected(int row, int column);

private:
    void setupUI();
    void setupHistoryChart();
    void setupBreakdownChart();
    void setupViolationChart();
    void updateChartData();
    void updateViolationPieChart();
    void updateReportTable();

    QTabWidget *m_tabWidget;

    // 历史趋势图
    QChart *m_historyChart;
    QChartView *m_historyChartView;
    QSplineSeries *m_totalScoreSeries;
    QSplineSeries *m_safetySeries;
    QSplineSeries *m_ruleSeries;
    QSplineSeries *m_smoothSeries;

    // 分项得分柱状图
    QChart *m_breakdownChart;
    QChartView *m_breakdownChartView;
    QBarSeries *m_barSeries;

    // 违规统计饼图
    QChart *m_violationChart;
    QChartView *m_violationChartView;

    // 报告列表
    QTableWidget *m_reportTable;

    QList<ScoreReport> m_history;
};

} // namespace PhantomDrive

#endif // DRIVINGHISTORYCHARTWIDGET_H
