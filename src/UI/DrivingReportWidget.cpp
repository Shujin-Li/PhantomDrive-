#include "UI/DrivingReportWidget.h"
#include "UI/ThemeManager.h"
#include "scoring/AIAPIClient.h"
#include "core/saveloadmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QDateTime>
#include <QRandomGenerator>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QAbstractItemView>

namespace PhantomDrive {

DrivingReportWidget::DrivingReportWidget(QWidget *parent)
    : QWidget(parent)
    , m_speedSeries(nullptr)
    , m_speedChart(nullptr)
    , m_speedChartView(nullptr)
    , m_breakdownBarSeries(nullptr)
    , m_breakdownChart(nullptr)
    , m_breakdownChartView(nullptr)
    , m_timer(nullptr)
    , m_tick(0)
    , m_totalSpeed(0.0)
    , m_speedDataCount(0)
    , m_maxRecordedSpeed(0.0)
    , m_useMockData(true)
    , m_aiReportLabel(nullptr)
    , m_coachAdviceWidget(nullptr)
    , m_historyScoreSeries(nullptr)
    , m_historyChart(nullptr)
    , m_historyChartView(nullptr)
{
    setupUI();
    setStyleSheet(ThemeManager::darkTheme());

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DrivingReportWidget::updateMockData);
    m_timer->start(1000);
}

DrivingReportWidget::~DrivingReportWidget()
{
    if (m_timer) {
        m_timer->stop();
    }
}

void DrivingReportWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // ========== 顶部统计面板 ==========
    QFrame *statsPanel = new QFrame(this);
    statsPanel->setObjectName("cardPanel");
    statsPanel->setStyleSheet(R"(
        #cardPanel {
            background-color: rgba(22, 33, 62, 230);
            border-radius: 12px;
            border: 1px solid rgba(255, 255, 255, 30);
        }
    )");

    auto *statsLayout = new QHBoxLayout(statsPanel);
    statsLayout->setSpacing(20);

    // 当前速度
    QVBoxLayout *currentSpeedLayout = new QVBoxLayout();
    m_currentSpeedLabel = new QLabel("0", statsPanel);
    m_currentSpeedLabel->setStyleSheet(R"(
        QLabel { color: #2ECC71; font-size: 36px; font-weight: bold; font-family: 'Segoe UI', monospace; }
    )");
    QLabel *currentSpeedUnit = new QLabel("km/h 当前速度", statsPanel);
    currentSpeedUnit->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    currentSpeedLayout->addWidget(m_currentSpeedLabel);
    currentSpeedLayout->addWidget(currentSpeedUnit);
    statsLayout->addLayout(currentSpeedLayout);

    // 平均速度
    QVBoxLayout *avgSpeedLayout = new QVBoxLayout();
    m_avgSpeedLabel = new QLabel("0.0", statsPanel);
    m_avgSpeedLabel->setStyleSheet(R"(
        QLabel { color: #3498DB; font-size: 24px; font-weight: bold; }
    )");
    QLabel *avgSpeedUnit = new QLabel("km/h 平均速度", statsPanel);
    avgSpeedUnit->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    avgSpeedLayout->addWidget(m_avgSpeedLabel);
    avgSpeedLayout->addWidget(avgSpeedUnit);
    statsLayout->addLayout(avgSpeedLayout);

    // 最高速度
    QVBoxLayout *maxSpeedLayout = new QVBoxLayout();
    m_maxSpeedLabel = new QLabel("0.0", statsPanel);
    m_maxSpeedLabel->setStyleSheet(R"(
        QLabel { color: #E74C3C; font-size: 24px; font-weight: bold; }
    )");
    QLabel *maxSpeedUnit = new QLabel("km/h 最高速度", statsPanel);
    maxSpeedUnit->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    maxSpeedLayout->addWidget(m_maxSpeedLabel);
    maxSpeedLayout->addWidget(maxSpeedUnit);
    statsLayout->addLayout(maxSpeedLayout);

    // 总分
    QVBoxLayout *scoreLayout = new QVBoxLayout();
    m_totalScoreLabel = new QLabel("--", statsPanel);
    m_totalScoreLabel->setStyleSheet(R"(
        QLabel { color: #F1C40F; font-size: 24px; font-weight: bold; }
    )");
    QLabel *scoreTitle = new QLabel("总分", statsPanel);
    scoreTitle->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    scoreLayout->addWidget(m_totalScoreLabel);
    scoreLayout->addWidget(scoreTitle);
    statsLayout->addLayout(scoreLayout);

    // 等级
    QVBoxLayout *gradeLayout = new QVBoxLayout();
    m_gradeLabel = new QLabel("--", statsPanel);
    m_gradeLabel->setStyleSheet(R"(
        QLabel { color: #9B59B6; font-size: 32px; font-weight: bold; }
    )");
    QLabel *gradeTitle = new QLabel("等级", statsPanel);
    gradeTitle->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    gradeLayout->addWidget(m_gradeLabel);
    gradeLayout->addWidget(gradeTitle);
    statsLayout->addLayout(gradeLayout);

    // 违规数
    QVBoxLayout *violationLayout = new QVBoxLayout();
    m_violationCountLabel = new QLabel("0", statsPanel);
    m_violationCountLabel->setStyleSheet(R"(
        QLabel { color: #E74C3C; font-size: 24px; font-weight: bold; }
    )");
    QLabel *violationTitle = new QLabel("违规次数", statsPanel);
    violationTitle->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    violationLayout->addWidget(m_violationCountLabel);
    violationLayout->addWidget(violationTitle);
    statsLayout->addLayout(violationLayout);

    mainLayout->addWidget(statsPanel);

    // ========== 实时速度图表 ==========
    QFrame *chartPanel = new QFrame(this);
    chartPanel->setObjectName("cardPanel");

    auto *chartLayout = new QVBoxLayout(chartPanel);

    QLabel *chartTitle = new QLabel("实时车速监控", chartPanel);
    chartTitle->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; padding: 5px; }");
    chartLayout->addWidget(chartTitle);

    m_speedSeries = new QSplineSeries();
    m_speedSeries->setName("实时车速");
    m_speedSeries->setColor(QColor(46, 204, 113));
    m_speedSeries->setPen(QPen(QColor(46, 204, 113), 3));

    m_speedChart = new QChart();
    m_speedChart->addSeries(m_speedSeries);
    m_speedChart->setBackgroundBrush(QBrush(QColor(22, 33, 62)));
    m_speedChart->setTitleBrush(QBrush(Qt::white));
    m_speedChart->setTitle("驾驶数据实时监控");

    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0, 120);
    axisY->setTitleText("km/h");
    axisY->setLabelsColor(QColor(189, 195, 199));
    axisY->setGridLineColor(QColor(60, 80, 100));
    m_speedChart->addAxis(axisY, Qt::AlignLeft);

    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0, 60);
    axisX->setLabelsVisible(false);
    m_speedChart->addAxis(axisX, Qt::AlignBottom);
    m_speedSeries->attachAxis(axisX);
    m_speedSeries->attachAxis(axisY);

    m_speedChartView = new QChartView(m_speedChart);
    m_speedChartView->setRenderHint(QPainter::Antialiasing);
    m_speedChartView->setMinimumHeight(200);

    chartLayout->addWidget(m_speedChartView);
    mainLayout->addWidget(chartPanel);

    // ========== 分项得分图表 ==========
    QFrame *breakdownPanel = new QFrame(this);
    breakdownPanel->setObjectName("cardPanel");

    auto *breakdownLayout = new QVBoxLayout(breakdownPanel);

    QLabel *breakdownTitle = new QLabel("分项得分", breakdownPanel);
    breakdownTitle->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; padding: 5px; }");
    breakdownLayout->addWidget(breakdownTitle);

    m_breakdownBarSeries = new QBarSeries();

    QBarSet *safetySet = new QBarSet("安全性");
    QBarSet *ruleSet = new QBarSet("规则遵守");
    QBarSet *smoothSet = new QBarSet("平顺性");
    QBarSet *efficiencySet = new QBarSet("效率");

    *safetySet << 100;
    *ruleSet << 100;
    *smoothSet << 100;
    *efficiencySet << 100;

    safetySet->setColor(QColor(231, 76, 60));
    ruleSet->setColor(QColor(52, 152, 219));
    smoothSet->setColor(QColor(46, 204, 113));
    efficiencySet->setColor(QColor(241, 196, 15));

    m_breakdownBarSeries->append(safetySet);
    m_breakdownBarSeries->append(ruleSet);
    m_breakdownBarSeries->append(smoothSet);
    m_breakdownBarSeries->append(efficiencySet);

    m_breakdownChart = new QChart();
    m_breakdownChart->addSeries(m_breakdownBarSeries);
    m_breakdownChart->setBackgroundBrush(QBrush(QColor(22, 33, 62)));

    QBarCategoryAxis *breakdownAxisX = new QBarCategoryAxis;
    QStringList categories;
    categories << QStringLiteral("当前报告");
    breakdownAxisX->append(categories);
    breakdownAxisX->setLabelsColor(QColor(189, 195, 199));
    m_breakdownChart->addAxis(breakdownAxisX, Qt::AlignBottom);

    QValueAxis *breakdownAxisY = new QValueAxis;
    breakdownAxisY->setRange(0, 100);
    breakdownAxisY->setLabelsColor(QColor(189, 195, 199));
    breakdownAxisY->setGridLineColor(QColor(60, 80, 100));
    m_breakdownChart->addAxis(breakdownAxisY, Qt::AlignLeft);
    m_breakdownBarSeries->attachAxis(breakdownAxisX);
    m_breakdownBarSeries->attachAxis(breakdownAxisY);

    m_breakdownChartView = new QChartView(m_breakdownChart);
    m_breakdownChartView->setRenderHint(QPainter::Antialiasing);
    m_breakdownChartView->setMinimumHeight(200);

    breakdownLayout->addWidget(m_breakdownChartView);
    mainLayout->addWidget(breakdownPanel);

    // ========== 教练建议面板 ==========
    m_coachAdviceWidget = new QFrame(this);
    m_coachAdviceWidget->setObjectName("cardPanel");
    m_coachAdviceWidget->setStyleSheet(R"(
        #cardPanel {
            background-color: rgba(22, 33, 62, 230);
            border-radius: 12px;
            border: 1px solid rgba(255, 255, 255, 30);
        }
    )");

    auto *adviceLayout = new QVBoxLayout(m_coachAdviceWidget);

    QLabel *adviceTitle = new QLabel("AI 教练建议", m_coachAdviceWidget);
    adviceTitle->setStyleSheet("QLabel { color: #F1C40F; font-size: 16px; font-weight: bold; padding: 5px; }");
    adviceLayout->addWidget(adviceTitle);

    m_aiReportLabel = new QLabel("等待生成教练报告...", m_coachAdviceWidget);
    m_aiReportLabel->setStyleSheet(R"(
        QLabel {
            color: #ecf0f1;
            font-size: 13px;
            padding: 10px;
            background-color: rgba(15, 52, 96, 150);
            border-radius: 8px;
        }
    )");
    m_aiReportLabel->setWordWrap(true);
    m_aiReportLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_aiReportLabel->setMinimumHeight(150);
    adviceLayout->addWidget(m_aiReportLabel);

    mainLayout->addWidget(m_coachAdviceWidget);

    // ========== 历史总分折线图 ==========
    QFrame *historyPanel = new QFrame(this);
    historyPanel->setObjectName("cardPanel");

    auto *historyLayout = new QVBoxLayout(historyPanel);

    QLabel *historyTitle = new QLabel("历史总分趋势", historyPanel);
    historyTitle->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; padding: 5px; }");
    historyLayout->addWidget(historyTitle);

    m_historyScoreSeries = new QLineSeries();
    m_historyScoreSeries->setName("总分");
    m_historyScoreSeries->setColor(QColor(241, 196, 15));
    m_historyScoreSeries->setPen(QPen(QColor(241, 196, 15), 3));

    m_historyChart = new QChart();
    m_historyChart->addSeries(m_historyScoreSeries);
    m_historyChart->setBackgroundBrush(QBrush(QColor(22, 33, 62)));
    m_historyChart->setTitle("历史驾驶评分");
    m_historyChart->setTitleBrush(QBrush(Qt::white));

    QValueAxis *historyAxisY = new QValueAxis;
    historyAxisY->setRange(0, 100);
    historyAxisY->setTitleText("分数");
    historyAxisY->setLabelsColor(QColor(189, 195, 199));
    historyAxisY->setGridLineColor(QColor(60, 80, 100));
    m_historyChart->addAxis(historyAxisY, Qt::AlignLeft);

    QValueAxis *historyAxisX = new QValueAxis;
    historyAxisX->setRange(0, 10);
    historyAxisX->setLabelsVisible(false);
    m_historyChart->addAxis(historyAxisX, Qt::AlignBottom);
    m_historyScoreSeries->attachAxis(historyAxisX);
    m_historyScoreSeries->attachAxis(historyAxisY);

    m_historyChartView = new QChartView(m_historyChart);
    m_historyChartView->setRenderHint(QPainter::Antialiasing);
    m_historyChartView->setMinimumHeight(180);

    historyLayout->addWidget(m_historyChartView);
    mainLayout->addWidget(historyPanel);

    // ========== 违规事件列表 ==========
    QFrame *violationPanel = new QFrame(this);
    violationPanel->setObjectName("cardPanel");

    auto *violationLayout2 = new QVBoxLayout(violationPanel);

    QLabel *violationTableTitle = new QLabel("违规事件记录", violationPanel);
    violationTableTitle->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; padding: 5px; }");
    violationLayout2->addWidget(violationTableTitle);

    m_violationTable = new QTableWidget(0, 4, violationPanel);
    QStringList headers;
    headers << "时间" << "类型" << "描述" << "扣分";
    m_violationTable->setHorizontalHeaderLabels(headers);
    m_violationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_violationTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_violationTable->setStyleSheet(R"(
        QTableWidget {
            background-color: rgba(15, 52, 96, 200);
            color: #ecf0f1;
            border: none;
            border-radius: 8px;
        }
        QTableWidget::item { padding: 8px; }
        QHeaderView::section {
            background-color: rgba(52, 73, 94, 200);
            color: #ecf0f1;
            padding: 8px;
            font-weight: bold;
        }
    )");

    violationLayout2->addWidget(m_violationTable);
    mainLayout->addWidget(violationPanel);
}

void DrivingReportWidget::updateMockData()
{
    if (!m_useMockData) return;

    int speed = QRandomGenerator::global()->bounded(40, 90);
    addSpeedData(speed);
}

void DrivingReportWidget::addSpeedData(qreal speed, qint64 timestamp)
{
    if (timestamp < 0) {
        timestamp = m_tick;
    }

    m_speedSeries->append(timestamp, speed);

    if (timestamp > 60) {
        m_speedChart->axisX()->setRange(timestamp - 60, timestamp);
    }

    m_totalSpeed += speed;
    m_speedDataCount++;
    if (speed > m_maxRecordedSpeed) {
        m_maxRecordedSpeed = speed;
    }

    m_currentSpeedLabel->setText(QString::number(static_cast<int>(speed)));
    m_avgSpeedLabel->setText(QString::number(m_totalSpeed / m_speedDataCount, 'f', 1));
    m_maxSpeedLabel->setText(QString::number(m_maxRecordedSpeed, 'f', 1));

    m_tick++;
}

void DrivingReportWidget::setCurrentReport(const ScoreReport& report)
{
    m_currentReport = report;
    m_useMockData = false;
    m_timer->stop();

    m_totalScoreLabel->setText(QString::number(report.totalScore, 'f', 1));
    m_gradeLabel->setText(report.grade);
    m_violationCountLabel->setText(QString::number(report.violations.size()));

    updateBreakdownChart();
    updateViolationTable();
    updateCoachAdvice();

    emit reportUpdated(report);
}

void DrivingReportWidget::addViolationEvent(const ViolationEvent& violation)
{
    m_violations.append(violation);
    m_violationCountLabel->setText(QString::number(m_violations.size()));

    int row = m_violationTable->rowCount();
    m_violationTable->insertRow(row);

    QDateTime dt = QDateTime::fromMSecsSinceEpoch(violation.timestamp);
    m_violationTable->setItem(row, 0, new QTableWidgetItem(dt.toString("hh:mm:ss")));
    m_violationTable->setItem(row, 1, new QTableWidgetItem(
        ScoreReport::violationTypeToString(violation.type)));
    m_violationTable->setItem(row, 2, new QTableWidgetItem(violation.description));
    m_violationTable->setItem(row, 3, new QTableWidgetItem(QString("-%1").arg(violation.penaltyPoints)));

    m_violationTable->scrollToBottom();
}

void DrivingReportWidget::clearData()
{
    m_speedSeries->clear();
    m_violations.clear();
    m_violationTable->setRowCount(0);
    m_tick = 0;
    m_totalSpeed = 0.0;
    m_speedDataCount = 0;
    m_maxRecordedSpeed = 0.0;

    m_currentSpeedLabel->setText("0");
    m_avgSpeedLabel->setText("0.0");
    m_maxSpeedLabel->setText("0.0");
    m_violationCountLabel->setText("0");
}

void DrivingReportWidget::updateBreakdownChart()
{
    QList<QBarSet*> barSets = m_breakdownBarSeries->barSets();
    if (barSets.size() >= 4) {
        barSets[0]->remove(0);
        *barSets[0] << m_currentReport.breakdown.safetyScore;

        barSets[1]->remove(0);
        *barSets[1] << m_currentReport.breakdown.ruleComplianceScore;

        barSets[2]->remove(0);
        *barSets[2] << m_currentReport.breakdown.smoothnessScore;

        barSets[3]->remove(0);
        *barSets[3] << m_currentReport.breakdown.efficiencyScore;
    }
}

void DrivingReportWidget::updateViolationTable()
{
    m_violationTable->setRowCount(0);

    for (const ViolationEvent& v : m_currentReport.violations) {
        int row = m_violationTable->rowCount();
        m_violationTable->insertRow(row);

        QDateTime dt = QDateTime::fromMSecsSinceEpoch(v.timestamp);
        m_violationTable->setItem(row, 0, new QTableWidgetItem(dt.toString("hh:mm:ss")));
        m_violationTable->setItem(row, 1, new QTableWidgetItem(
            ScoreReport::violationTypeToString(v.type)));
        m_violationTable->setItem(row, 2, new QTableWidgetItem(v.description));
        m_violationTable->setItem(row, 3, new QTableWidgetItem(
            QString("-%1").arg(v.penaltyPoints)));
    }
}

void DrivingReportWidget::updateCoachAdvice()
{
    if (m_aiReportLabel) {
        AIAPIClient aiClient;
        QString aiReport = aiClient.generateCoachReport(m_currentReport);
        m_aiReportLabel->setText(aiReport);
    }
}

void DrivingReportWidget::loadHistoryFromSaveLoadManager()
{
    SaveLoadManager& manager = SaveLoadManager::instance();
    QList<ScoreReport> reports = manager.loadHistory();
    loadHistoryReports(reports);
}

void DrivingReportWidget::loadHistoryReports(const QList<ScoreReport>& reports)
{
    m_historyReports = reports;
    m_historyScoreSeries->clear();
    
    int index = 0;
    for (const ScoreReport& report : reports) {
        m_historyScoreSeries->append(index++, report.totalScore);
    }
    
    // 更新图表范围
    if (index > 0) {
        m_historyChart->axisX()->setRange(0, qMax(index, 10));
        m_historyChart->axisY()->setRange(0, 100);
    }
}

} // namespace PhantomDrive
