#include "UI/DrivingHistoryChartWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QLegend>
#include <QDateTime>
#include <QPainter>
#include <QDebug>
#include <QAbstractItemView>

namespace PhantomDrive {

DrivingHistoryChartWidget::DrivingHistoryChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_historyChart(nullptr)
    , m_totalScoreSeries(nullptr)
    , m_safetySeries(nullptr)
    , m_ruleSeries(nullptr)
    , m_smoothSeries(nullptr)
    , m_breakdownChart(nullptr)
    , m_barSeries(nullptr)
    , m_violationChart(nullptr)
    , m_violationChartView(nullptr)
{
    setupUI();
}

void DrivingHistoryChartWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    // 顶部标签
    auto *headerLabel = new QLabel("驾驶历史数据可视化", this);
    headerLabel->setStyleSheet(
        "QLabel { font-size: 18px; font-weight: bold; color: #2C3E50; padding: 10px; }"
    );
    mainLayout->addWidget(headerLabel);

    // Tab 页面
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    setupHistoryChart();
    setupBreakdownChart();
    setupViolationChart();

    // 报告列表
    m_reportTable = new QTableWidget(0, 5, this);
    QStringList headers;
    headers << "时间" << "总分" << "等级" << "安全分" << "规则分";
    m_reportTable->setHorizontalHeaderLabels(headers);
    m_reportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_reportTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_reportTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_reportTable->setStyleSheet(R"(
        QTableWidget {
            background-color: #ECF0F1;
            alternateBackgroundColor: #D5DBDB;
            color: #2C3E50;
            gridline-color: #BDC3C7;
            border: none;
            font-size: 12px;
        }
        QTableWidget::item:selected {
            background-color: #3498DB;
            color: white;
        }
        QHeaderView::section {
            background-color: #34495E;
            color: white;
            padding: 8px;
            font-weight: bold;
            border: 1px solid #2C3E50;
        }
    )");

    connect(m_reportTable, &QTableWidget::cellClicked,
            this, &DrivingHistoryChartWidget::onReportSelected);

    mainLayout->addWidget(m_reportTable);
}

void DrivingHistoryChartWidget::setupHistoryChart()
{
    m_historyChart = new QChart();
    m_historyChart->setTitle("总分及分项得分趋势");
    m_historyChart->setAnimationOptions(QChart::SeriesAnimations);
    m_historyChart->setBackgroundBrush(QBrush(QColor(236, 240, 241)));

    m_totalScoreSeries = new QSplineSeries();
    m_totalScoreSeries->setName("总分");
    m_totalScoreSeries->setColor(QColor(231, 76, 60));
    m_totalScoreSeries->setPen(QPen(QColor(231, 76, 60), 3));

    m_safetySeries = new QSplineSeries();
    m_safetySeries->setName("安全分");
    m_safetySeries->setColor(QColor(46, 204, 113));
    m_safetySeries->setPen(QPen(QColor(46, 204, 113), 2));

    m_ruleSeries = new QSplineSeries();
    m_ruleSeries->setName("规则分");
    m_ruleSeries->setColor(QColor(52, 152, 219));
    m_ruleSeries->setPen(QPen(QColor(52, 152, 219), 2));

    m_smoothSeries = new QSplineSeries();
    m_smoothSeries->setName("平顺分");
    m_smoothSeries->setColor(QColor(155, 89, 182));
    m_smoothSeries->setPen(QPen(QColor(155, 89, 182), 2));

    m_historyChart->addSeries(m_totalScoreSeries);
    m_historyChart->addSeries(m_safetySeries);
    m_historyChart->addSeries(m_ruleSeries);
    m_historyChart->addSeries(m_smoothSeries);

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("MM-dd hh:mm");
    axisX->setTitleText("时间");
    axisX->setLabelsColor(QColor(44, 62, 80));
    axisX->setLinePenColor(QColor(44, 62, 80));
    m_historyChart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0, 100);
    axisY->setTitleText("分数");
    axisY->setLabelsColor(QColor(44, 62, 80));
    axisY->setLinePenColor(QColor(44, 62, 80));
    axisY->setGridLineColor(QColor(189, 195, 199));
    m_historyChart->addAxis(axisY, Qt::AlignLeft);

    m_totalScoreSeries->attachAxis(axisX);
    m_totalScoreSeries->attachAxis(axisY);
    m_safetySeries->attachAxis(axisX);
    m_safetySeries->attachAxis(axisY);
    m_ruleSeries->attachAxis(axisX);
    m_ruleSeries->attachAxis(axisY);
    m_smoothSeries->attachAxis(axisX);
    m_smoothSeries->attachAxis(axisY);

    m_historyChart->legend()->setVisible(true);
    m_historyChart->legend()->setAlignment(Qt::AlignBottom);
    m_historyChart->legend()->setBackgroundVisible(true);
    m_historyChart->legend()->setBrush(QBrush(QColor(255, 255, 255, 200)));

    m_historyChartView = new QChartView(m_historyChart);
    m_historyChartView->setRenderHint(QPainter::Antialiasing);
    m_historyChartView->setMinimumSize(600, 300);

    m_tabWidget->addTab(m_historyChartView, "历史趋势");
}

void DrivingHistoryChartWidget::setupBreakdownChart()
{
    m_breakdownChart = new QChart();
    m_breakdownChart->setTitle("最新报告分项得分详情");
    m_breakdownChart->setAnimationOptions(QChart::SeriesAnimations);
    m_breakdownChart->setBackgroundBrush(QBrush(QColor(236, 240, 241)));

    m_barSeries = new QBarSeries();

    QBarSet *safetySet = new QBarSet("安全分");
    QBarSet *ruleSet = new QBarSet("规则分");
    QBarSet *smoothSet = new QBarSet("平顺分");
    QBarSet *efficiencySet = new QBarSet("效率分");

    *safetySet << 0;
    *ruleSet << 0;
    *smoothSet << 0;
    *efficiencySet << 0;

    m_barSeries->append(safetySet);
    m_barSeries->append(ruleSet);
    m_barSeries->append(smoothSet);
    m_barSeries->append(efficiencySet);

    m_breakdownChart->addSeries(m_barSeries);

    QBarCategoryAxis *axisX = new QBarCategoryAxis;
    QStringList categories;
    categories << QStringLiteral("最新报告");
    axisX->append(categories);
    axisX->setLabelsColor(QColor(44, 62, 80));
    axisX->setLinePenColor(QColor(44, 62, 80));
    m_breakdownChart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0, 100);
    axisY->setTitleText("分数");
    axisY->setLabelsColor(QColor(44, 62, 80));
    axisY->setLinePenColor(QColor(44, 62, 80));
    axisY->setGridLineColor(QColor(189, 195, 199));
    m_breakdownChart->addAxis(axisY, Qt::AlignLeft);

    m_barSeries->attachAxis(axisX);
    m_barSeries->attachAxis(axisY);

    m_breakdownChart->legend()->setVisible(true);
    m_breakdownChart->legend()->setAlignment(Qt::AlignBottom);

    m_breakdownChartView = new QChartView(m_breakdownChart);
    m_breakdownChartView->setRenderHint(QPainter::Antialiasing);
    m_breakdownChartView->setMinimumSize(600, 300);

    m_tabWidget->addTab(m_breakdownChartView, "分项得分");
}

void DrivingHistoryChartWidget::setupViolationChart()
{
    m_violationChart = new QChart();
    m_violationChart->setTitle("违规类型统计");
    m_violationChart->setAnimationOptions(QChart::AllAnimations);
    m_violationChart->setBackgroundBrush(QBrush(QColor(236, 240, 241)));

    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.35);
    series->append("无违规", 1);

    m_violationChart->addSeries(series);

    m_violationChart->legend()->setVisible(true);
    m_violationChart->legend()->setAlignment(Qt::AlignRight);

    m_violationChartView = new QChartView(m_violationChart);
    m_violationChartView->setRenderHint(QPainter::Antialiasing);
    m_violationChartView->setMinimumSize(600, 300);

    m_tabWidget->addTab(m_violationChartView, "违规统计");
}

void DrivingHistoryChartWidget::updateHistory(const QList<ScoreReport>& history)
{
    m_history = history;
    updateChartData();
    updateReportTable();
}

void DrivingHistoryChartWidget::addReport(const ScoreReport& report)
{
    m_history.append(report);
    updateChartData();
    updateReportTable();
}

void DrivingHistoryChartWidget::clear()
{
    m_history.clear();
    m_totalScoreSeries->clear();
    m_safetySeries->clear();
    m_ruleSeries->clear();
    m_smoothSeries->clear();
    m_reportTable->setRowCount(0);

    QList<QBarSet*> barSets = m_barSeries->barSets();
    if (barSets.size() >= 4) {
        barSets[0]->remove(0);
        barSets[1]->remove(0);
        barSets[2]->remove(0);
        barSets[3]->remove(0);
        *barSets[0] << 0;
        *barSets[1] << 0;
        *barSets[2] << 0;
        *barSets[3] << 0;
    }

    QPieSeries *pieSeries = qobject_cast<QPieSeries*>(m_violationChart->series().first());
    if (pieSeries) {
        pieSeries->clear();
        pieSeries->append("无违规", 1);
    }
}

void DrivingHistoryChartWidget::updateChartData()
{
    m_totalScoreSeries->clear();
    m_safetySeries->clear();
    m_ruleSeries->clear();
    m_smoothSeries->clear();

    for (int i = 0; i < m_history.size(); ++i) {
        const ScoreReport &report = m_history[i];
        qint64 timestamp = report.generatedAt.toMSecsSinceEpoch();

        m_totalScoreSeries->append(timestamp, report.totalScore);
        m_safetySeries->append(timestamp, report.breakdown.safetyScore);
        m_ruleSeries->append(timestamp, report.breakdown.ruleComplianceScore);
        m_smoothSeries->append(timestamp, report.breakdown.smoothnessScore);
    }

    if (!m_history.isEmpty()) {
        const ScoreReport &latest = m_history.last();
        QList<QBarSet*> barSets = m_barSeries->barSets();
        if (barSets.size() >= 4) {
            barSets[0]->remove(0);
            barSets[1]->remove(0);
            barSets[2]->remove(0);
            barSets[3]->remove(0);
            *barSets[0] << latest.breakdown.safetyScore;
            *barSets[1] << latest.breakdown.ruleComplianceScore;
            *barSets[2] << latest.breakdown.smoothnessScore;
            *barSets[3] << latest.breakdown.efficiencyScore;
        }
    }

    updateViolationPieChart();
    updateReportTable();
}

void DrivingHistoryChartWidget::updateViolationPieChart()
{
    QPieSeries *pieSeries = qobject_cast<QPieSeries*>(m_violationChart->series().first());
    if (!pieSeries) return;

    pieSeries->clear();

    int collisionCount = 0;
    int speedCount = 0;
    int redLightCount = 0;
    int pedestrianCount = 0;
    int wrongWayCount = 0;

    for (const ScoreReport &report : m_history) {
        for (const ViolationEvent &v : report.violations) {
            switch (v.type) {
            case ViolationType::Collision:
                collisionCount++;
                break;
            case ViolationType::SpeedOverLimit:
                speedCount++;
                break;
            case ViolationType::RedLight:
                redLightCount++;
                break;
            case ViolationType::PedestrianCollision:
                pedestrianCount++;
                break;
            case ViolationType::WrongWay:
                wrongWayCount++;
                break;
            default:
                break;
            }
        }
    }

    int total = collisionCount + speedCount + redLightCount + pedestrianCount + wrongWayCount;

    if (total == 0) {
        pieSeries->append("无违规", 1);
    } else {
        if (collisionCount > 0)
            pieSeries->append(QString("碰撞 %1").arg(collisionCount), collisionCount);
        if (speedCount > 0)
            pieSeries->append(QString("超速 %1").arg(speedCount), speedCount);
        if (redLightCount > 0)
            pieSeries->append(QString("红灯 %1").arg(redLightCount), redLightCount);
        if (pedestrianCount > 0)
            pieSeries->append(QString("行人 %1").arg(pedestrianCount), pedestrianCount);
        if (wrongWayCount > 0)
            pieSeries->append(QString("逆行 %1").arg(wrongWayCount), wrongWayCount);
    }

    int sliceIndex = 0;
    QList<QColor> colors = {
        QColor(231, 76, 60),
        QColor(230, 126, 34),
        QColor(241, 196, 15),
        QColor(142, 68, 173),
        QColor(52, 73, 94)
    };

    for (QPieSlice *slice : pieSeries->slices()) {
        if (sliceIndex < colors.size()) {
            slice->setColor(colors[sliceIndex]);
        }
        slice->setLabelVisible(true);
        sliceIndex++;
    }
}

void DrivingHistoryChartWidget::updateReportTable()
{
    m_reportTable->setRowCount(0);

    for (int i = 0; i < m_history.size(); ++i) {
        const ScoreReport &report = m_history[i];
        int row = m_reportTable->rowCount();
        m_reportTable->insertRow(row);

        m_reportTable->setItem(row, 0, new QTableWidgetItem(
            report.generatedAt.toString("yyyy-MM-dd hh:mm:ss")));
        m_reportTable->setItem(row, 1, new QTableWidgetItem(
            QString::number(report.totalScore, 'f', 1)));
        m_reportTable->setItem(row, 2, new QTableWidgetItem(report.grade));
        m_reportTable->setItem(row, 3, new QTableWidgetItem(
            QString::number(report.breakdown.safetyScore, 'f', 1)));
        m_reportTable->setItem(row, 4, new QTableWidgetItem(
            QString::number(report.breakdown.ruleComplianceScore, 'f', 1)));
    }
}

void DrivingHistoryChartWidget::onReportSelected(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0 && row < m_history.size()) {
        emit reportSelected(row, m_history[row]);
    }
}

} // namespace PhantomDrive
