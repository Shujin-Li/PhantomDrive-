#include "UI/DrivingReportWidget.h"
#include "UI/ThemeManager.h"
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
    , m_speedAxisX(nullptr)
    , m_speedAxisY(nullptr)
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
    , m_historySafetySeries(nullptr)
    , m_historyRuleSeries(nullptr)
    , m_historySmoothSeries(nullptr)
    , m_historyEfficiencySeries(nullptr)
    , m_historyChart(nullptr)
    , m_historyChartView(nullptr)
    , m_historyAxisX(nullptr)
    , m_historyAxisY(nullptr)
{
    setupUI();
    setStyleSheet(ThemeManager::darkTheme());

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DrivingReportWidget::updateMockData);
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

    // ========== 椤堕儴缁熻闈㈡澘 ==========
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

    // 褰撳墠閫熷害
    QVBoxLayout *currentSpeedLayout = new QVBoxLayout();
    m_currentSpeedLabel = new QLabel("0", statsPanel);
    m_currentSpeedLabel->setStyleSheet(R"(
        QLabel { color: #2ECC71; font-size: 36px; font-weight: bold; font-family: 'Segoe UI', monospace; }
    )");
    QLabel *currentSpeedUnit = new QLabel(QStringLiteral("km/h Current Speed"), statsPanel);
    currentSpeedUnit->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    currentSpeedLayout->addWidget(m_currentSpeedLabel);
    currentSpeedLayout->addWidget(currentSpeedUnit);
    statsLayout->addLayout(currentSpeedLayout);

    // 骞冲潎閫熷害
    QVBoxLayout *avgSpeedLayout = new QVBoxLayout();
    m_avgSpeedLabel = new QLabel("0.0", statsPanel);
    m_avgSpeedLabel->setStyleSheet(R"(
        QLabel { color: #3498DB; font-size: 24px; font-weight: bold; }
    )");
    QLabel *avgSpeedUnit = new QLabel(QStringLiteral("km/h Average Speed"), statsPanel);
    avgSpeedUnit->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    avgSpeedLayout->addWidget(m_avgSpeedLabel);
    avgSpeedLayout->addWidget(avgSpeedUnit);
    statsLayout->addLayout(avgSpeedLayout);

    // 鏈€楂橀€熷害
    QVBoxLayout *maxSpeedLayout = new QVBoxLayout();
    m_maxSpeedLabel = new QLabel("0.0", statsPanel);
    m_maxSpeedLabel->setStyleSheet(R"(
        QLabel { color: #E74C3C; font-size: 24px; font-weight: bold; }
    )");
    QLabel *maxSpeedUnit = new QLabel(QStringLiteral("km/h Max Speed"), statsPanel);
    maxSpeedUnit->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    maxSpeedLayout->addWidget(m_maxSpeedLabel);
    maxSpeedLayout->addWidget(maxSpeedUnit);
    statsLayout->addLayout(maxSpeedLayout);

    // 鎬诲垎
    QVBoxLayout *scoreLayout = new QVBoxLayout();
    m_totalScoreLabel = new QLabel("--", statsPanel);
    m_totalScoreLabel->setStyleSheet(R"(
        QLabel { color: #F1C40F; font-size: 24px; font-weight: bold; }
    )");
    QLabel *scoreTitle = new QLabel(QStringLiteral("Total Score"), statsPanel);
    scoreTitle->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    scoreLayout->addWidget(m_totalScoreLabel);
    scoreLayout->addWidget(scoreTitle);
    statsLayout->addLayout(scoreLayout);

    // 绛夌骇
    QVBoxLayout *gradeLayout = new QVBoxLayout();
    m_gradeLabel = new QLabel("--", statsPanel);
    m_gradeLabel->setStyleSheet(R"(
        QLabel { color: #9B59B6; font-size: 32px; font-weight: bold; }
    )");
    QLabel *gradeTitle = new QLabel(QStringLiteral("Grade"), statsPanel);
    gradeTitle->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    gradeLayout->addWidget(m_gradeLabel);
    gradeLayout->addWidget(gradeTitle);
    statsLayout->addLayout(gradeLayout);

    // 杩濊鏁?
    QVBoxLayout *violationLayout = new QVBoxLayout();
    m_violationCountLabel = new QLabel("0", statsPanel);
    m_violationCountLabel->setStyleSheet(R"(
        QLabel { color: #E74C3C; font-size: 24px; font-weight: bold; }
    )");
    QLabel *violationTitle = new QLabel(QStringLiteral("Violations"), statsPanel);
    violationTitle->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    violationLayout->addWidget(m_violationCountLabel);
    violationLayout->addWidget(violationTitle);
    statsLayout->addLayout(violationLayout);

    mainLayout->addWidget(statsPanel);

    // ========== 瀹炴椂閫熷害鍥捐〃 ==========
    QFrame *chartPanel = new QFrame(this);
    chartPanel->setObjectName("cardPanel");

    auto *chartLayout = new QVBoxLayout(chartPanel);

    QLabel *chartTitle = new QLabel(QStringLiteral("Live Speed Monitor"), chartPanel);
    chartTitle->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; padding: 5px; }");
    chartLayout->addWidget(chartTitle);

    m_speedSeries = new QSplineSeries();
    m_speedSeries->setName(QStringLiteral("Speed"));
    m_speedSeries->setColor(QColor(46, 204, 113));
    m_speedSeries->setPen(QPen(QColor(46, 204, 113), 3));

    m_speedChart = new QChart();
    m_speedChart->addSeries(m_speedSeries);
    m_speedChart->setBackgroundBrush(QBrush(QColor(22, 33, 62)));
    m_speedChart->setTitleBrush(QBrush(Qt::white));
    m_speedChart->setTitle(QStringLiteral("Driving Data"));

    m_speedAxisY = new QValueAxis;
    m_speedAxisY->setRange(0, 120);
    m_speedAxisY->setTitleText("km/h");
    m_speedAxisY->setLabelsColor(QColor(189, 195, 199));
    m_speedAxisY->setGridLineColor(QColor(60, 80, 100));
    m_speedChart->addAxis(m_speedAxisY, Qt::AlignLeft);

    m_speedAxisX = new QValueAxis;
    m_speedAxisX->setRange(0, 60);
    m_speedAxisX->setLabelsVisible(false);
    m_speedChart->addAxis(m_speedAxisX, Qt::AlignBottom);
    m_speedSeries->attachAxis(m_speedAxisX);
    m_speedSeries->attachAxis(m_speedAxisY);

    m_speedChartView = new QChartView(m_speedChart);
    m_speedChartView->setRenderHint(QPainter::Antialiasing);
    m_speedChartView->setMinimumHeight(200);

    chartLayout->addWidget(m_speedChartView);
    mainLayout->addWidget(chartPanel);

    // ========== 鍒嗛」寰楀垎鍥捐〃 ==========
    QFrame *breakdownPanel = new QFrame(this);
    breakdownPanel->setObjectName("cardPanel");

    auto *breakdownLayout = new QVBoxLayout(breakdownPanel);

    QLabel *breakdownTitle = new QLabel(QStringLiteral("Score Breakdown"), breakdownPanel);
    breakdownTitle->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; padding: 5px; }");
    breakdownLayout->addWidget(breakdownTitle);

    m_breakdownBarSeries = new QBarSeries();

    QBarSet *safetySet = new QBarSet(QStringLiteral("Safety"));
    QBarSet *ruleSet = new QBarSet(QStringLiteral("Rules"));
    QBarSet *smoothSet = new QBarSet(QStringLiteral("Smoothness"));
    QBarSet *efficiencySet = new QBarSet(QStringLiteral("Efficiency"));

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
    categories << QStringLiteral("Current Report");
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

    // ========== 鏁欑粌寤鸿闈㈡澘 ==========
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

    QLabel *adviceTitle = new QLabel(QStringLiteral("AI Coach Advice"), m_coachAdviceWidget);
    adviceTitle->setStyleSheet("QLabel { color: #F1C40F; font-size: 16px; font-weight: bold; padding: 5px; }");
    adviceLayout->addWidget(adviceTitle);

    m_aiReportLabel = new QLabel(QStringLiteral("Waiting for AI coach report..."), m_coachAdviceWidget);
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

    // ========== 鍘嗗彶鎬诲垎鎶樼嚎鍥?==========
    QFrame *historyPanel = new QFrame(this);
    historyPanel->setObjectName("cardPanel");

    auto *historyLayout = new QVBoxLayout(historyPanel);

    QLabel *historyTitle = new QLabel(QStringLiteral("History Score Trend"), historyPanel);
    historyTitle->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; padding: 5px; }");
    historyLayout->addWidget(historyTitle);

    m_historyScoreSeries = new QLineSeries();
    m_historyScoreSeries->setName(QStringLiteral("Total Score"));
    m_historyScoreSeries->setColor(QColor(241, 196, 15));
    m_historyScoreSeries->setPen(QPen(QColor(241, 196, 15), 3));

    m_historySafetySeries = new QLineSeries();
    m_historySafetySeries->setName(QStringLiteral("Safety"));
    m_historySafetySeries->setColor(QColor(231, 76, 60));
    m_historySafetySeries->setPen(QPen(QColor(231, 76, 60), 2));

    m_historyRuleSeries = new QLineSeries();
    m_historyRuleSeries->setName(QStringLiteral("Rules"));
    m_historyRuleSeries->setColor(QColor(52, 152, 219));
    m_historyRuleSeries->setPen(QPen(QColor(52, 152, 219), 2));

    m_historySmoothSeries = new QLineSeries();
    m_historySmoothSeries->setName(QStringLiteral("Smoothness"));
    m_historySmoothSeries->setColor(QColor(46, 204, 113));
    m_historySmoothSeries->setPen(QPen(QColor(46, 204, 113), 2));

    m_historyEfficiencySeries = new QLineSeries();
    m_historyEfficiencySeries->setName(QStringLiteral("Efficiency"));
    m_historyEfficiencySeries->setColor(QColor(155, 89, 182));
    m_historyEfficiencySeries->setPen(QPen(QColor(155, 89, 182), 2));

    m_historyChart = new QChart();
    m_historyChart->addSeries(m_historyScoreSeries);
    m_historyChart->addSeries(m_historySafetySeries);
    m_historyChart->addSeries(m_historyRuleSeries);
    m_historyChart->addSeries(m_historySmoothSeries);
    m_historyChart->addSeries(m_historyEfficiencySeries);
    m_historyChart->setBackgroundBrush(QBrush(QColor(22, 33, 62)));
    m_historyChart->setTitle(QStringLiteral("Driving Score History"));
    m_historyChart->setTitleBrush(QBrush(Qt::white));

    m_historyAxisY = new QValueAxis;
    m_historyAxisY->setRange(0, 100);
    m_historyAxisY->setTitleText(QStringLiteral("Score"));
    m_historyAxisY->setLabelsColor(QColor(189, 195, 199));
    m_historyAxisY->setGridLineColor(QColor(60, 80, 100));
    m_historyChart->addAxis(m_historyAxisY, Qt::AlignLeft);

    m_historyAxisX = new QValueAxis;
    m_historyAxisX->setRange(0, 10);
    m_historyAxisX->setLabelsVisible(false);
    m_historyChart->addAxis(m_historyAxisX, Qt::AlignBottom);

    // 将所有系列绑定到坐标轴
    QList<QAbstractSeries*> allSeries = m_historyChart->series();
    for (QAbstractSeries* s : allSeries) {
        if (auto* lineSeries = qobject_cast<QLineSeries*>(s)) {
            lineSeries->attachAxis(m_historyAxisX);
            lineSeries->attachAxis(m_historyAxisY);
        }
    }

    m_historyChartView = new QChartView(m_historyChart);
    m_historyChartView->setRenderHint(QPainter::Antialiasing);
    m_historyChartView->setMinimumHeight(180);

    historyLayout->addWidget(m_historyChartView);
    mainLayout->addWidget(historyPanel);

    // ========== 杩濊浜嬩欢鍒楄〃 ==========
    QFrame *violationPanel = new QFrame(this);
    violationPanel->setObjectName("cardPanel");

    auto *violationLayout2 = new QVBoxLayout(violationPanel);

    QLabel *violationTableTitle = new QLabel(QStringLiteral("Violation Events"), violationPanel);
    violationTableTitle->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; padding: 5px; }");
    violationLayout2->addWidget(violationTableTitle);

    m_violationTable = new QTableWidget(0, 4, violationPanel);
    QStringList headers;
    headers << QStringLiteral("Time") << QStringLiteral("Type") << QStringLiteral("Description") << QStringLiteral("Penalty");
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
        m_speedAxisX->setRange(timestamp - 60, timestamp);
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
    setMockDataEnabled(false);

    m_totalScoreLabel->setText(QString::number(report.totalScore, 'f', 1));
    m_gradeLabel->setText(report.grade);
    m_violationCountLabel->setText(QString::number(report.violations.size()));
    m_avgSpeedLabel->setText(QString::number(report.metrics.averageSpeed, 'f', 1));
    m_maxSpeedLabel->setText(QString::number(report.metrics.maxSpeed, 'f', 1));

    updateBreakdownChart();
    updateViolationTable();
    updateCoachAdvice();

    emit reportUpdated(report);
}

void DrivingReportWidget::setCoachReportMarkdown(const QString& markdown)
{
    if (m_aiReportLabel) {
        m_aiReportLabel->setText(markdown.trimmed().isEmpty()
            ? QStringLiteral("AI coach report is empty.")
            : markdown);
    }
}

void DrivingReportWidget::setMockDataEnabled(bool enabled)
{
    m_useMockData = enabled;
    if (!m_timer) {
        return;
    }

    if (enabled && !m_timer->isActive()) {
        m_timer->start(1000);
    } else if (!enabled && m_timer->isActive()) {
        m_timer->stop();
    }
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

    // 清除历史图表
    if (m_historyScoreSeries) m_historyScoreSeries->clear();
    if (m_historySafetySeries) m_historySafetySeries->clear();
    if (m_historyRuleSeries) m_historyRuleSeries->clear();
    if (m_historySmoothSeries) m_historySmoothSeries->clear();
    if (m_historyEfficiencySeries) m_historyEfficiencySeries->clear();

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
        QStringList lines;
        if (!m_currentReport.summary.trimmed().isEmpty()) {
            lines << m_currentReport.summary.trimmed();
        }

        for (const CoachAdvice& advice : m_currentReport.coachAdvices) {
            lines << QStringLiteral("[%1/%2] %3")
                         .arg(advice.category, advice.severity, advice.message);
        }

        if (lines.isEmpty()) {
            lines << QStringLiteral("Waiting for AI coach report...");
        }

        m_aiReportLabel->setText(lines.join(QStringLiteral("\n")));
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

    // 直接清除所有历史系列
    m_historyScoreSeries->clear();
    m_historySafetySeries->clear();
    m_historyRuleSeries->clear();
    m_historySmoothSeries->clear();
    m_historyEfficiencySeries->clear();

    int index = 0;
    for (const ScoreReport& report : reports) {
        m_historyScoreSeries->append(index, report.totalScore);
        m_historySafetySeries->append(index, report.breakdown.safetyScore);
        m_historyRuleSeries->append(index, report.breakdown.ruleComplianceScore);
        m_historySmoothSeries->append(index, report.breakdown.smoothnessScore);
        m_historyEfficiencySeries->append(index, report.breakdown.efficiencyScore);
        ++index;
    }

    if (index > 0) {
        m_historyAxisX->setRange(0, qMax(index, 10));
        m_historyAxisY->setRange(0, 100);
        m_historyChart->setTitle(QStringLiteral("Driving Score History"));
    } else {
        // 无历史数据时显示提示
        m_historyChart->setTitle(QStringLiteral("Driving Score History  (No records yet)"));
    }
}

} // namespace PhantomDrive
