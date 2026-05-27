#include "UI/ABDrivingReportWidget.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QRandomGenerator>
#include <QStringList>
#include <QtGlobal>
#include <QVBoxLayout>

namespace PhantomDrive {

namespace {

QFrame* makePanel(QWidget* parent)
{
    auto* panel = new QFrame(parent);
    panel->setObjectName("abReportPanel");
    return panel;
}

QLabel* makeTitle(const QString& text, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    label->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; padding: 5px; }");
    return label;
}

QLabel* makeMetricValue(const QString& color, QWidget* parent, int size = 24)
{
    auto* label = new QLabel("--", parent);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setStyleSheet(QString(
        "QLabel { color: %1; font-size: %2px; font-weight: bold; font-family: 'Segoe UI', Arial; }")
        .arg(color)
        .arg(size));
    return label;
}

QLabel* makeMetricCaption(const QString& text, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    label->setStyleSheet("QLabel { color: rgba(255,255,255,150); font-size: 12px; }");
    return label;
}

void replaceSingleBarValue(QBarSeries* series, int index, qreal value)
{
    const QList<QBarSet*> sets = series->barSets();
    if (index < 0 || index >= sets.size()) {
        return;
    }

    QBarSet* set = sets.at(index);
    if (set->count() > 0) {
        set->remove(0);
    }
    *set << value;
}

} // namespace

ABDrivingReportWidget::ABDrivingReportWidget(QWidget* parent)
    : QWidget(parent)
    , m_speedSeries(nullptr)
    , m_speedChart(nullptr)
    , m_speedChartView(nullptr)
    , m_speedAxisX(nullptr)
    , m_speedAxisY(nullptr)
    , m_breakdownBarSeries(nullptr)
    , m_breakdownChart(nullptr)
    , m_breakdownChartView(nullptr)
    , m_historyScoreSeries(nullptr)
    , m_historyChart(nullptr)
    , m_historyChartView(nullptr)
    , m_historyAxisX(nullptr)
    , m_historyAxisY(nullptr)
    , m_currentSpeedLabel(nullptr)
    , m_avgSpeedLabel(nullptr)
    , m_maxSpeedLabel(nullptr)
    , m_totalScoreLabel(nullptr)
    , m_gradeLabel(nullptr)
    , m_violationCountLabel(nullptr)
    , m_aiReportLabel(nullptr)
    , m_violationTable(nullptr)
    , m_timer(new QTimer(this))
    , m_nextMockTimestamp(0)
    , m_useMockData(false)
{
    setupUI();
    applyDarkTheme();

    connect(m_timer, &QTimer::timeout, this, [this]() {
        updateMockData();
    });
}

ABDrivingReportWidget::~ABDrivingReportWidget()
{
    if (m_timer) {
        m_timer->stop();
    }
}

void ABDrivingReportWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QFrame* statsPanel = makePanel(this);
    auto* statsLayout = new QHBoxLayout(statsPanel);
    statsLayout->setSpacing(22);

    auto addMetric = [statsPanel, statsLayout](QLabel*& valueLabel,
                                               const QString& caption,
                                               const QString& color,
                                               int size = 24) {
        auto* metricLayout = new QVBoxLayout();
        valueLabel = makeMetricValue(color, statsPanel, size);
        metricLayout->addWidget(valueLabel);
        metricLayout->addWidget(makeMetricCaption(caption, statsPanel));
        statsLayout->addLayout(metricLayout);
    };

    addMetric(m_currentSpeedLabel, "km/h Current Speed", "#2ECC71", 34);
    addMetric(m_avgSpeedLabel, "km/h Average Speed", "#3498DB");
    addMetric(m_maxSpeedLabel, "km/h Max Speed", "#E74C3C");
    addMetric(m_totalScoreLabel, "Total Score", "#F1C40F");
    addMetric(m_gradeLabel, "Grade", "#9B59B6", 30);
    addMetric(m_violationCountLabel, "Violations", "#E74C3C");
    mainLayout->addWidget(statsPanel);

    QFrame* speedPanel = makePanel(this);
    auto* speedLayout = new QVBoxLayout(speedPanel);
    speedLayout->addWidget(makeTitle("Live Speed Monitor", speedPanel));

    m_speedSeries = new QSplineSeries();
    m_speedSeries->setName("Speed");
    m_speedSeries->setColor(QColor(46, 204, 113));
    m_speedSeries->setPen(QPen(QColor(46, 204, 113), 3));

    m_speedChart = new QChart();
    m_speedChart->addSeries(m_speedSeries);
    m_speedChart->setTitle("Driving Data");
    m_speedChart->legend()->hide();
    m_speedChart->setBackgroundBrush(QBrush(QColor(22, 33, 62)));
    m_speedChart->setTitleBrush(QBrush(Qt::white));

    m_speedAxisY = new QValueAxis();
    m_speedAxisY->setRange(0, 120);
    m_speedAxisY->setTitleText("km/h");
    m_speedAxisY->setLabelsColor(QColor(189, 195, 199));
    m_speedAxisY->setGridLineColor(QColor(60, 80, 100));
    m_speedChart->addAxis(m_speedAxisY, Qt::AlignLeft);

    m_speedAxisX = new QValueAxis();
    m_speedAxisX->setRange(0, 60);
    m_speedAxisX->setLabelsVisible(false);
    m_speedChart->addAxis(m_speedAxisX, Qt::AlignBottom);
    m_speedSeries->attachAxis(m_speedAxisX);
    m_speedSeries->attachAxis(m_speedAxisY);

    m_speedChartView = new QChartView(m_speedChart);
    m_speedChartView->setRenderHint(QPainter::Antialiasing);
    m_speedChartView->setMinimumHeight(200);
    speedLayout->addWidget(m_speedChartView);
    mainLayout->addWidget(speedPanel);

    QFrame* breakdownPanel = makePanel(this);
    auto* breakdownLayout = new QVBoxLayout(breakdownPanel);
    breakdownLayout->addWidget(makeTitle("Score Breakdown", breakdownPanel));

    m_breakdownBarSeries = new QBarSeries();
    auto* safetySet = new QBarSet("Safety");
    auto* ruleSet = new QBarSet("Rules");
    auto* smoothSet = new QBarSet("Smoothness");
    auto* efficiencySet = new QBarSet("Efficiency");
    *safetySet << 0;
    *ruleSet << 0;
    *smoothSet << 0;
    *efficiencySet << 0;
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
    m_breakdownChart->setTitleBrush(QBrush(Qt::white));

    auto* breakdownAxisX = new QBarCategoryAxis();
    breakdownAxisX->append(QStringList() << "Current Report");
    breakdownAxisX->setLabelsColor(QColor(189, 195, 199));
    m_breakdownChart->addAxis(breakdownAxisX, Qt::AlignBottom);

    auto* breakdownAxisY = new QValueAxis();
    breakdownAxisY->setRange(0, 100);
    breakdownAxisY->setLabelsColor(QColor(189, 195, 199));
    breakdownAxisY->setGridLineColor(QColor(60, 80, 100));
    m_breakdownChart->addAxis(breakdownAxisY, Qt::AlignLeft);
    m_breakdownBarSeries->attachAxis(breakdownAxisX);
    m_breakdownBarSeries->attachAxis(breakdownAxisY);

    m_breakdownChartView = new QChartView(m_breakdownChart);
    m_breakdownChartView->setRenderHint(QPainter::Antialiasing);
    m_breakdownChartView->setMinimumHeight(180);
    breakdownLayout->addWidget(m_breakdownChartView);
    mainLayout->addWidget(breakdownPanel);

    QFrame* advicePanel = makePanel(this);
    auto* adviceLayout = new QVBoxLayout(advicePanel);
    adviceLayout->addWidget(makeTitle("AI Coach Advice", advicePanel));
    m_aiReportLabel = new QLabel("Waiting for report data...", advicePanel);
    m_aiReportLabel->setWordWrap(true);
    m_aiReportLabel->setMinimumHeight(80);
    m_aiReportLabel->setStyleSheet(
        "QLabel { color: #ecf0f1; font-size: 13px; line-height: 1.4; padding: 10px; "
        "background-color: rgba(15, 52, 96, 160); border-radius: 8px; }");
    adviceLayout->addWidget(m_aiReportLabel);
    mainLayout->addWidget(advicePanel);

    QFrame* historyPanel = makePanel(this);
    auto* historyLayout = new QVBoxLayout(historyPanel);
    historyLayout->addWidget(makeTitle("History Score Trend", historyPanel));

    m_historyScoreSeries = new QLineSeries();
    m_historyScoreSeries->setName("Score");
    m_historyScoreSeries->setColor(QColor(241, 196, 15));
    m_historyScoreSeries->setPen(QPen(QColor(241, 196, 15), 3));

    m_historyChart = new QChart();
    m_historyChart->addSeries(m_historyScoreSeries);
    m_historyChart->legend()->hide();
    m_historyChart->setBackgroundBrush(QBrush(QColor(22, 33, 62)));

    m_historyAxisX = new QValueAxis();
    m_historyAxisX->setRange(0, 10);
    m_historyAxisX->setLabelFormat("%d");
    m_historyAxisX->setLabelsColor(QColor(189, 195, 199));
    m_historyChart->addAxis(m_historyAxisX, Qt::AlignBottom);

    m_historyAxisY = new QValueAxis();
    m_historyAxisY->setRange(0, 100);
    m_historyAxisY->setLabelsColor(QColor(189, 195, 199));
    m_historyAxisY->setGridLineColor(QColor(60, 80, 100));
    m_historyChart->addAxis(m_historyAxisY, Qt::AlignLeft);
    m_historyScoreSeries->attachAxis(m_historyAxisX);
    m_historyScoreSeries->attachAxis(m_historyAxisY);

    m_historyChartView = new QChartView(m_historyChart);
    m_historyChartView->setRenderHint(QPainter::Antialiasing);
    m_historyChartView->setMinimumHeight(180);
    historyLayout->addWidget(m_historyChartView);
    mainLayout->addWidget(historyPanel);

    QFrame* violationPanel = makePanel(this);
    auto* violationLayout = new QVBoxLayout(violationPanel);
    violationLayout->addWidget(makeTitle("Violation Events", violationPanel));

    m_violationTable = new QTableWidget(0, 6, violationPanel);
    m_violationTable->setHorizontalHeaderLabels(
        QStringList() << "Time" << "Type" << "Description" << "Speed" << "Limit" << "Penalty");
    m_violationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_violationTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_violationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_violationTable->setAlternatingRowColors(true);
    violationLayout->addWidget(m_violationTable);
    mainLayout->addWidget(violationPanel);

    clearData();
}

void ABDrivingReportWidget::applyDarkTheme()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #101828;
            color: #ecf0f1;
            font-family: 'Segoe UI', Arial, sans-serif;
        }
        QFrame#abReportPanel {
            background-color: rgba(22, 33, 62, 230);
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 30);
        }
        QTableWidget {
            background-color: rgba(15, 52, 96, 200);
            alternate-background-color: rgba(22, 64, 112, 160);
            color: #ecf0f1;
            border: none;
            border-radius: 8px;
            gridline-color: rgba(255, 255, 255, 35);
        }
        QTableWidget::item { padding: 8px; }
        QHeaderView::section {
            background-color: rgba(52, 73, 94, 220);
            color: #ecf0f1;
            padding: 8px;
            font-weight: bold;
            border: none;
        }
    )");
}

void ABDrivingReportWidget::addSpeedData(qreal speed, qint64 timestamp)
{
    if (timestamp < 0) {
        timestamp = m_nextMockTimestamp++;
    } else if (timestamp >= m_nextMockTimestamp) {
        m_nextMockTimestamp = timestamp + 1;
    }

    m_speedSamples.append(SpeedSample{speed, timestamp});
    m_speedSeries->append(timestamp, speed);

    if (timestamp > 60) {
        m_speedAxisX->setRange(timestamp - 60, timestamp);
    } else {
        m_speedAxisX->setRange(0, 60);
    }

    if (speed > m_speedAxisY->max() * 0.9) {
        m_speedAxisY->setRange(0, qMax(120.0, speed + 20.0));
    }

    updateStatsFromSamples();
}

void ABDrivingReportWidget::setReportSummary(const ReportSummary& summary)
{
    m_currentSummary = summary;
    m_violations = summary.violations;
    setMockDataEnabled(false);

    m_totalScoreLabel->setText(QString::number(summary.totalScore));
    m_gradeLabel->setText(summary.grade.trimmed().isEmpty() ? "--" : summary.grade);
    m_violationCountLabel->setText(QString::number(summary.violations.size()));

    if (summary.averageSpeed > 0.0) {
        m_avgSpeedLabel->setText(QString::number(summary.averageSpeed, 'f', 1));
    }
    if (summary.maxSpeed > 0.0) {
        m_maxSpeedLabel->setText(QString::number(summary.maxSpeed, 'f', 1));
    }

    updateBreakdownChart(summary);
    updateViolationTable(summary.violations);
    updateCoachAdvice(summary.coachAdvice);
}

void ABDrivingReportWidget::setHistoryReports(const QList<ReportSummary>& reports)
{
    m_historyReports = reports;
    m_historyScoreSeries->clear();

    int index = 0;
    for (const ReportSummary& report : reports) {
        m_historyScoreSeries->append(index++, report.totalScore);
    }

    m_historyAxisX->setRange(0, qMax(10, index));
    m_historyAxisY->setRange(0, 100);
}

void ABDrivingReportWidget::clearData()
{
    m_speedSamples.clear();
    m_violations.clear();
    m_historyReports.clear();
    m_currentSummary = ReportSummary();
    m_nextMockTimestamp = 0;

    if (m_speedSeries) {
        m_speedSeries->clear();
    }
    if (m_historyScoreSeries) {
        m_historyScoreSeries->clear();
    }
    if (m_violationTable) {
        m_violationTable->setRowCount(0);
    }

    m_speedAxisX->setRange(0, 60);
    m_speedAxisY->setRange(0, 120);
    m_historyAxisX->setRange(0, 10);
    m_historyAxisY->setRange(0, 100);

    m_currentSpeedLabel->setText("0");
    m_avgSpeedLabel->setText("0.0");
    m_maxSpeedLabel->setText("0.0");
    m_totalScoreLabel->setText("--");
    m_gradeLabel->setText("--");
    m_violationCountLabel->setText("0");
    m_aiReportLabel->setText("Waiting for report data...");
    resetBreakdownChart();
}

void ABDrivingReportWidget::setMockDataEnabled(bool enabled)
{
    m_useMockData = enabled;
    if (enabled && !m_timer->isActive()) {
        m_timer->start(1000);
    } else if (!enabled && m_timer->isActive()) {
        m_timer->stop();
    }
}

void ABDrivingReportWidget::updateMockData()
{
    if (!m_useMockData) {
        return;
    }

    const qreal speed = QRandomGenerator::global()->bounded(35, 96);
    addSpeedData(speed);
}

void ABDrivingReportWidget::updateStatsFromSamples()
{
    if (m_speedSamples.isEmpty()) {
        return;
    }

    qreal total = 0.0;
    qreal maxSpeed = 0.0;
    for (const SpeedSample& sample : m_speedSamples) {
        total += sample.speed;
        maxSpeed = qMax(maxSpeed, sample.speed);
    }

    const qreal currentSpeed = m_speedSamples.last().speed;
    m_currentSpeedLabel->setText(QString::number(static_cast<int>(currentSpeed)));
    m_avgSpeedLabel->setText(QString::number(total / m_speedSamples.size(), 'f', 1));
    m_maxSpeedLabel->setText(QString::number(maxSpeed, 'f', 1));
}

void ABDrivingReportWidget::updateBreakdownChart(const ReportSummary& summary)
{
    int totalPenalty = 0;
    for (const ViolationItem& violation : summary.violations) {
        totalPenalty += qMax(0, violation.penalty);
    }

    const qreal base = boundedScore(summary.totalScore);
    const qreal safety = boundedScore(base - summary.violations.size() * 3.0);
    const qreal rules = boundedScore(base - totalPenalty * 0.6);
    const qreal smoothness = boundedScore(base + (summary.averageSpeed > 0.0 && summary.averageSpeed < 90.0 ? 4.0 : -4.0));
    const qreal efficiency = boundedScore(base + (summary.maxSpeed > 0.0 ? qMin(summary.maxSpeed / 30.0, 5.0) : 0.0));

    replaceSingleBarValue(m_breakdownBarSeries, 0, safety);
    replaceSingleBarValue(m_breakdownBarSeries, 1, rules);
    replaceSingleBarValue(m_breakdownBarSeries, 2, smoothness);
    replaceSingleBarValue(m_breakdownBarSeries, 3, efficiency);
}

void ABDrivingReportWidget::updateViolationTable(const QList<ViolationItem>& violations)
{
    m_violationTable->setRowCount(0);

    for (const ViolationItem& violation : violations) {
        const int row = m_violationTable->rowCount();
        m_violationTable->insertRow(row);
        m_violationTable->setItem(row, 0, new QTableWidgetItem(formattedTime(violation.timestamp)));
        m_violationTable->setItem(row, 1, new QTableWidgetItem(violation.type));
        m_violationTable->setItem(row, 2, new QTableWidgetItem(violation.description));
        m_violationTable->setItem(row, 3, new QTableWidgetItem(QString::number(violation.speed, 'f', 1)));
        m_violationTable->setItem(row, 4, new QTableWidgetItem(
            violation.limit > 0.0 ? QString::number(violation.limit, 'f', 1) : "--"));
        m_violationTable->setItem(row, 5, new QTableWidgetItem(QString("-%1").arg(violation.penalty)));
    }

    m_violationTable->scrollToBottom();
}

void ABDrivingReportWidget::updateCoachAdvice(const QString& advice)
{
    m_aiReportLabel->setText(advice.trimmed().isEmpty()
        ? "No coach advice provided."
        : advice.trimmed());
}

void ABDrivingReportWidget::resetBreakdownChart()
{
    for (int i = 0; i < 4; ++i) {
        replaceSingleBarValue(m_breakdownBarSeries, i, 0.0);
    }
}

qreal ABDrivingReportWidget::boundedScore(qreal value)
{
    return qBound(0.0, value, 100.0);
}

QString ABDrivingReportWidget::formattedTime(qint64 timestamp)
{
    if (timestamp <= 0) {
        return "--";
    }
    return QDateTime::fromMSecsSinceEpoch(timestamp).toString("hh:mm:ss");
}

} // namespace PhantomDrive
