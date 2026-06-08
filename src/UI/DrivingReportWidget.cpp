#include "UI/DrivingReportWidget.h"
#include "UI/ThemeManager.h"
#include "core/saveloadmanager.h"
#include "scoring/ScoreReport.h"

#include <QDateTime>
#include <QHeaderView>
#include <QPainter>
#include <QRandomGenerator>
#include <QAbstractItemView>
#include <QResizeEvent>
#include <QPushButton>
#include <QSignalBlocker>

namespace PhantomDrive {

namespace {
constexpr int kChartWindowSeconds = 60;
}

DrivingReportWidget::DrivingReportWidget(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_scrollContent(nullptr)
    , m_historyPlaceholderLabel(nullptr)
    , m_livePlaceholderLabel(nullptr)
    , m_loadingOverlay(nullptr)
    , m_loadingLabel(nullptr)
    , m_loadingDotTimer(nullptr)
    , m_loadingDotCount(0)
    , m_playerSwitchWidget(nullptr)
    , m_player1Button(nullptr)
    , m_player2Button(nullptr)
    , m_avgSpeedLabel(nullptr)
    , m_maxSpeedLabel(nullptr)
    , m_currentSpeedLabel(nullptr)
    , m_totalScoreLabel(nullptr)
    , m_gradeLabel(nullptr)
    , m_violationCountLabel(nullptr)
    , m_safetyValueLabel(nullptr)
    , m_ruleValueLabel(nullptr)
    , m_smoothValueLabel(nullptr)
    , m_efficiencyValueLabel(nullptr)
    , m_safetyBar(nullptr)
    , m_ruleBar(nullptr)
    , m_smoothBar(nullptr)
    , m_efficiencyBar(nullptr)
    , m_speedSeries(nullptr)
    , m_speedChart(nullptr)
    , m_speedChartView(nullptr)
    , m_speedAxisX(nullptr)
    , m_speedAxisY(nullptr)
    , m_coachAdviceWidget(nullptr)
    , m_aiReportLabel(nullptr)
    , m_historyList(nullptr)
    , m_historyScoreSeries(nullptr)
    , m_historySafetySeries(nullptr)
    , m_historyRuleSeries(nullptr)
    , m_historySmoothSeries(nullptr)
    , m_historyEfficiencySeries(nullptr)
    , m_historyChart(nullptr)
    , m_historyChartView(nullptr)
    , m_historyAxisX(nullptr)
    , m_historyAxisY(nullptr)
    , m_violationTable(nullptr)
    , m_timer(nullptr)
    , m_tick(0)
    , m_totalSpeed(0.0)
    , m_speedDataCount(0)
    , m_maxRecordedSpeed(0.0)
    , m_useMockData(true)
    , m_hasPlayerReports(false)
    , m_activePlayerIndex(1)
    , m_applyingPlayerReport(false)
    , m_syncingHistorySelection(false)
{
    setupUI();
    setStyleSheet(ThemeManager::reportPanelQss());

    // ---- loading overlay (shown while data is being computed) ----
    m_loadingOverlay = new QWidget(this);
    m_loadingOverlay->setObjectName(QStringLiteral("loadingOverlay"));
    m_loadingOverlay->setStyleSheet(
        QStringLiteral("QWidget#loadingOverlay {"
                       "  background-color: rgba(6,10,24,220);"
                       "  border-radius: 12px;"
                       "}"));
    m_loadingOverlay->hide();

    QVBoxLayout* overlayLayout = new QVBoxLayout(m_loadingOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);

    m_loadingLabel = new QLabel(QStringLiteral("正在计算驾驶报告…"), m_loadingOverlay);
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setStyleSheet(
        QStringLiteral("color: #00FFAA; font-size: 22px; font-weight: bold;"
                       "font-family: 'Consolas', monospace; letter-spacing: 2px;"));
    overlayLayout->addWidget(m_loadingLabel);

    QLabel* subLabel = new QLabel(QStringLiteral("Scoring · Analyzing · Generating coach advice"), m_loadingOverlay);
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setStyleSheet(QStringLiteral("color: #7090B0; font-size: 13px; margin-top: 8px;"));
    overlayLayout->addWidget(subLabel);

    m_loadingDotTimer = new QTimer(this);
    m_loadingDotTimer->setInterval(400);
    connect(m_loadingDotTimer, &QTimer::timeout, this, [this]() {
        m_loadingDotCount = (m_loadingDotCount + 1) % 4;
        const QString dots = QString(m_loadingDotCount, QChar('.'));
        m_loadingLabel->setText(QStringLiteral("正在计算驾驶报告") + dots);
    });

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DrivingReportWidget::updateMockData);

    updateLiveChartPlaceholder();
    updateHistoryPlaceholder();
    updateSummaryLabels();
}

DrivingReportWidget::~DrivingReportWidget()
{
    if (m_timer) {
        m_timer->stop();
    }
}

QFrame* DrivingReportWidget::createCard(const QString& title, QWidget* content)
{
    QFrame* card = new QFrame(this);
    card->setObjectName(QStringLiteral("neonCard"));

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 16);
    layout->setSpacing(12);

    QLabel* titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("cardTitle"));
    layout->addWidget(titleLabel);

    if (content) {
        layout->addWidget(content, 1);
    }

    return card;
}

QWidget* DrivingReportWidget::createSummaryCard(const QString& title, QLabel*& valueLabel, const QString& objectName)
{
    QFrame* card = new QFrame(this);
    card->setObjectName(QStringLiteral("neonCard"));

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(6);

    QLabel* titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("cardTitle"));
    layout->addWidget(titleLabel);

    valueLabel = new QLabel(QStringLiteral("--"), card);
    valueLabel->setObjectName(objectName);
    valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(valueLabel);

    QLabel* unitLabel = new QLabel(card);
    unitLabel->setObjectName(QStringLiteral("unitLabel"));
    if (title.contains(QStringLiteral("速度")) || title.contains(QStringLiteral("Speed"))) {
        unitLabel->setText(QStringLiteral("km/h"));
    } else if (title.contains(QStringLiteral("评分")) || title.contains(QStringLiteral("Score"))) {
        unitLabel->setText(QStringLiteral("0 - 100"));
    } else if (title.contains(QStringLiteral("违规")) || title.contains(QStringLiteral("Violations"))) {
        unitLabel->setText(QStringLiteral("event count"));
    } else {
        unitLabel->setText(QStringLiteral("live summary"));
    }
    layout->addWidget(unitLabel);

    return card;
}

void DrivingReportWidget::setupUI()
{
    setObjectName(QStringLiteral("reportRoot"));
    // No fixed size – the widget fills whatever container it is placed in.
    setMinimumSize(400, 300);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    setupHeaderBar(root);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_scrollContent = new QWidget(m_scrollArea);
    QVBoxLayout* contentLayout = new QVBoxLayout(m_scrollContent);
    contentLayout->setContentsMargins(22, 22, 22, 22);
    contentLayout->setSpacing(18);

    setupSummarySection(contentLayout);
    setupMiddleSection(contentLayout);
    setupBottomSection(contentLayout);

    m_scrollArea->setWidget(m_scrollContent);
    root->addWidget(m_scrollArea);
}

void DrivingReportWidget::setupHeaderBar(QVBoxLayout* rootLayout)
{
    QWidget* bar = new QWidget(this);
    bar->setObjectName(QStringLiteral("reportHeaderBar"));
    bar->setFixedHeight(56);
    bar->setStyleSheet(
        QStringLiteral("QWidget#reportHeaderBar {"
                       "  background-color: rgba(6,10,24,230);"
                       "  border-bottom: 1px solid rgba(0,255,170,60);"
                       "}"));

    QHBoxLayout* barLayout = new QHBoxLayout(bar);
    barLayout->setContentsMargins(20, 0, 20, 0);
    barLayout->setSpacing(12);

    QLabel* titleLabel = new QLabel(QStringLiteral("PhantomDrive  ·  Driving Report"), bar);
    titleLabel->setStyleSheet(
        QStringLiteral("color: #00FFAA; font-size: 16px; font-weight: bold;"
                       "font-family: 'Consolas', monospace; letter-spacing: 1px;"));
    barLayout->addWidget(titleLabel);
    barLayout->addStretch();

    m_playerSwitchWidget = new QWidget(bar);
    m_playerSwitchWidget->setObjectName(QStringLiteral("playerSwitchWidget"));
    m_playerSwitchWidget->setStyleSheet(
        QStringLiteral("QWidget#playerSwitchWidget {"
                       "  background-color: rgba(8,14,32,190);"
                       "  border: 1px solid rgba(0,180,255,80);"
                       "  border-radius: 6px;"
                       "}"
                       "QPushButton {"
                       "  background-color: transparent;"
                       "  color: #8FB5D8;"
                       "  border: none;"
                       "  border-radius: 4px;"
                       "  padding: 0 14px;"
                       "  font-size: 12px;"
                       "  font-weight: bold;"
                       "  font-family: 'Consolas', monospace;"
                       "}"
                       "QPushButton:checked {"
                       "  background-color: rgba(0,255,170,45);"
                       "  color: #00FFAA;"
                       "}"
                       "QPushButton:hover {"
                       "  color: #E8F4FF;"
                       "}"));

    QHBoxLayout* switchLayout = new QHBoxLayout(m_playerSwitchWidget);
    switchLayout->setContentsMargins(3, 3, 3, 3);
    switchLayout->setSpacing(2);

    m_player1Button = new QPushButton(QStringLiteral("Player 1"), m_playerSwitchWidget);
    m_player1Button->setCheckable(true);
    m_player1Button->setFixedHeight(30);
    m_player1Button->setCursor(Qt::PointingHandCursor);
    switchLayout->addWidget(m_player1Button);

    m_player2Button = new QPushButton(QStringLiteral("Player 2"), m_playerSwitchWidget);
    m_player2Button->setCheckable(true);
    m_player2Button->setFixedHeight(30);
    m_player2Button->setCursor(Qt::PointingHandCursor);
    switchLayout->addWidget(m_player2Button);

    connect(m_player1Button, &QPushButton::clicked, this, [this]() {
        applyPlayerReport(1);
    });
    connect(m_player2Button, &QPushButton::clicked, this, [this]() {
        applyPlayerReport(2);
    });

    m_playerSwitchWidget->hide();
    barLayout->addWidget(m_playerSwitchWidget);

    // "New Drive" button
    QPushButton* btnNew = new QPushButton(QStringLiteral("▶  New Drive"), bar);
    btnNew->setObjectName(QStringLiteral("reportActionBtn"));
    btnNew->setFixedHeight(36);
    btnNew->setCursor(Qt::PointingHandCursor);
    btnNew->setStyleSheet(
        QStringLiteral("QPushButton#reportActionBtn {"
                       "  background-color: rgba(0,255,170,30);"
                       "  color: #00FFAA;"
                       "  border: 1px solid #00FFAA;"
                       "  border-radius: 6px;"
                       "  padding: 0 18px;"
                       "  font-size: 13px;"
                       "  font-weight: bold;"
                       "  font-family: 'Consolas', monospace;"
                       "}"
                       "QPushButton#reportActionBtn:hover {"
                       "  background-color: rgba(0,255,170,70);"
                       "}"
                       "QPushButton#reportActionBtn:pressed {"
                       "  background-color: rgba(0,255,170,110);"
                       "}"));
    connect(btnNew, &QPushButton::clicked, this, &DrivingReportWidget::newDriveRequested);
    barLayout->addWidget(btnNew);

    // "Back to Menu" button
    QPushButton* btnBack = new QPushButton(QStringLiteral("⬅  Back to Menu"), bar);
    btnBack->setObjectName(QStringLiteral("reportBackBtn"));
    btnBack->setFixedHeight(36);
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setStyleSheet(
        QStringLiteral("QPushButton#reportBackBtn {"
                       "  background-color: rgba(0,180,255,20);"
                       "  color: #00B4FF;"
                       "  border: 1px solid #00B4FF;"
                       "  border-radius: 6px;"
                       "  padding: 0 18px;"
                       "  font-size: 13px;"
                       "  font-weight: bold;"
                       "  font-family: 'Consolas', monospace;"
                       "}"
                       "QPushButton#reportBackBtn:hover {"
                       "  background-color: rgba(0,180,255,60);"
                       "}"
                       "QPushButton#reportBackBtn:pressed {"
                       "  background-color: rgba(0,180,255,100);"
                       "}"));
    connect(btnBack, &QPushButton::clicked, this, &DrivingReportWidget::backToMenuRequested);
    barLayout->addWidget(btnBack);

    rootLayout->addWidget(bar);
}

void DrivingReportWidget::setupSummarySection(QVBoxLayout* rootLayout)
{
    QGridLayout* grid = new QGridLayout();
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(14);

    grid->addWidget(createSummaryCard(QStringLiteral("平均速度 Average Speed"), m_avgSpeedLabel, QStringLiteral("bigNumber")), 0, 0);
    grid->addWidget(createSummaryCard(QStringLiteral("最高速度 Max Speed"), m_maxSpeedLabel, QStringLiteral("bigNumberRed")), 0, 1);
    grid->addWidget(createSummaryCard(QStringLiteral("总评分 Total Score"), m_totalScoreLabel, QStringLiteral("bigNumberYellow")), 0, 2);
    grid->addWidget(createSummaryCard(QStringLiteral("等级 Grade"), m_gradeLabel, QStringLiteral("bigNumberGreen")), 0, 3);

    rootLayout->addLayout(grid);
}

void DrivingReportWidget::setupMiddleSection(QVBoxLayout* rootLayout)
{
    setupCharts();
    styleCharts();

    QHBoxLayout* middle = new QHBoxLayout();
    middle->setSpacing(18);

    QWidget* scoreWidget = new QWidget(this);
    QVBoxLayout* scoreLayout = new QVBoxLayout(scoreWidget);
    scoreLayout->setContentsMargins(0, 0, 0, 0);
    scoreLayout->setSpacing(14);

    auto buildScoreRow = [this, scoreLayout](const QString& name,
                                              QLabel*& valueLabel,
                                              QProgressBar*& bar,
                                              const QString& barObjectName) {
        QWidget* row = new QWidget(this);
        QVBoxLayout* rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(6);

        QHBoxLayout* titleLine = new QHBoxLayout();
        QLabel* title = new QLabel(name, row);
        title->setObjectName(QStringLiteral("unitLabel"));
        valueLabel = new QLabel(QStringLiteral("0"), row);
        valueLabel->setObjectName(QStringLiteral("bigNumberGreen"));
        valueLabel->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: bold; color: #E8F4FF; font-family: 'Consolas', monospace;"));
        titleLine->addWidget(title);
        titleLine->addStretch();
        titleLine->addWidget(valueLabel);
        rowLayout->addLayout(titleLine);

        bar = new QProgressBar(row);
        bar->setObjectName(barObjectName);
        bar->setProperty("textVisible", false);
        bar->setMaximum(100);
        bar->setValue(0);
        bar->setFixedHeight(10);
        rowLayout->addWidget(bar);

        scoreLayout->addWidget(row);
    };

    buildScoreRow(QStringLiteral("安全 Safety"), m_safetyValueLabel, m_safetyBar, QStringLiteral("scoreBarSafety"));
    buildScoreRow(QStringLiteral("规则 Rules"), m_ruleValueLabel, m_ruleBar, QStringLiteral("scoreBarRule"));
    buildScoreRow(QStringLiteral("平顺 Smoothness"), m_smoothValueLabel, m_smoothBar, QStringLiteral("scoreBarSmooth"));
    buildScoreRow(QStringLiteral("效率 Efficiency"), m_efficiencyValueLabel, m_efficiencyBar, QStringLiteral("scoreBarEfficiency"));
    scoreLayout->addStretch();

    middle->addWidget(createCard(QStringLiteral("评分配比 / Score Breakdown"), scoreWidget), 1);

    QWidget* liveChartContainer = new QWidget(this);
    QVBoxLayout* liveLayout = new QVBoxLayout(liveChartContainer);
    liveLayout->setContentsMargins(0, 0, 0, 0);
    liveLayout->setSpacing(8);

    m_livePlaceholderLabel = new QLabel(QStringLiteral("等待实时速度数据… / waiting for live speed samples"), liveChartContainer);
    m_livePlaceholderLabel->setObjectName(QStringLiteral("placeholder"));
    m_livePlaceholderLabel->setAlignment(Qt::AlignCenter);

    liveLayout->addWidget(m_speedChartView, 1);
    liveLayout->addWidget(m_livePlaceholderLabel);

    middle->addWidget(createCard(QStringLiteral("实时速度监控 / Live Speed -60s to Now"), liveChartContainer), 2);

    rootLayout->addLayout(middle);
}

void DrivingReportWidget::setupBottomSection(QVBoxLayout* rootLayout)
{
    QHBoxLayout* bottom = new QHBoxLayout();
    bottom->setSpacing(18);

    m_coachAdviceWidget = new QWidget(this);
    QVBoxLayout* coachLayout = new QVBoxLayout(m_coachAdviceWidget);
    coachLayout->setContentsMargins(0, 0, 0, 0);

    m_aiReportLabel = new QLabel(QStringLiteral("Waiting for AI coach report..."), m_coachAdviceWidget);
    m_aiReportLabel->setObjectName(QStringLiteral("coachText"));
    m_aiReportLabel->setWordWrap(true);
    m_aiReportLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_aiReportLabel->setMinimumHeight(170);
    coachLayout->addWidget(m_aiReportLabel);

    bottom->addWidget(createCard(QStringLiteral("AI 教练建议 / Coach Advice"), m_coachAdviceWidget), 1);

    QWidget* historyContainer = new QWidget(this);
    QVBoxLayout* historyLayout = new QVBoxLayout(historyContainer);
    historyLayout->setContentsMargins(0, 0, 0, 0);
    historyLayout->setSpacing(8);

    m_historyList = new QListWidget(historyContainer);
    m_historyList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_historyList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_historyList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_historyList->setMinimumHeight(150);
    m_historyList->setStyleSheet(QStringLiteral(
        "QListWidget {"
        " background-color: rgba(8,14,32,140);"
        " border: 1px solid rgba(0,180,255,50);"
        " border-radius: 10px;"
        " color: #E8F4FF;"
        " font-size: 12px;"
        " outline: none;"
        "}"
        "QListWidget::item {"
        " padding: 8px 10px;"
        " border-bottom: 1px solid rgba(255,255,255,18);"
        "}"
        "QListWidget::item:selected {"
        " background-color: rgba(0,180,255,55);"
        " color: #FFFFFF;"
        "}"
        "QListWidget::item:hover {"
        " background-color: rgba(255,255,255,18);"
        "}"));
    historyLayout->addWidget(m_historyList);

    historyLayout->addWidget(m_historyChartView, 1);

    m_historyPlaceholderLabel = new QLabel(QStringLiteral("No history records yet"), historyContainer);
    m_historyPlaceholderLabel->setObjectName(QStringLiteral("placeholder"));
    m_historyPlaceholderLabel->setAlignment(Qt::AlignCenter);
    historyLayout->addWidget(m_historyPlaceholderLabel);

    bottom->addWidget(createCard(QStringLiteral("历史评分趋势 / History Score Trend"), historyContainer), 1);

    connect(m_historyList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (m_syncingHistorySelection || row < 0 || row >= m_historyReports.size()) {
            return;
        }
        setCurrentReport(m_historyReports.at(row));
    });

    rootLayout->addLayout(bottom);

    m_violationTable = new QTableWidget(0, 4, this);
    m_violationTable->setAlternatingRowColors(true);
    m_violationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_violationTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_violationTable->setShowGrid(false);
    QStringList headers;
    headers << QStringLiteral("TIME") << QStringLiteral("TYPE") << QStringLiteral("DESCRIPTION") << QStringLiteral("PENALTY");
    m_violationTable->setHorizontalHeaderLabels(headers);
    m_violationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_violationTable->verticalHeader()->setVisible(false);
    m_violationTable->setMinimumHeight(210);

    rootLayout->addWidget(createCard(QStringLiteral("违规事件列表 / Violation Events"), m_violationTable));
}

void DrivingReportWidget::setupCharts()
{
    m_speedSeries = new QSplineSeries();
    m_speedSeries->setName(QStringLiteral("Speed"));
    m_speedSeries->setPen(QPen(QColor(0, 255, 170), 2.5));

    m_speedChart = new QChart();
    m_speedChart->addSeries(m_speedSeries);
    m_speedChart->legend()->hide();

    m_speedAxisX = new QValueAxis();
    m_speedAxisX->setRange(0, kChartWindowSeconds);
    m_speedAxisX->setTitleText(QStringLiteral("Seconds"));

    m_speedAxisY = new QValueAxis();
    m_speedAxisY->setRange(0, 120);
    m_speedAxisY->setTitleText(QStringLiteral("km/h"));

    m_speedChart->addAxis(m_speedAxisX, Qt::AlignBottom);
    m_speedChart->addAxis(m_speedAxisY, Qt::AlignLeft);
    m_speedSeries->attachAxis(m_speedAxisX);
    m_speedSeries->attachAxis(m_speedAxisY);

    m_speedChartView = new QChartView(m_speedChart, this);
    m_speedChartView->setRenderHint(QPainter::Antialiasing);
    m_speedChartView->setMinimumHeight(240);

    m_historyScoreSeries = new QLineSeries();
    m_historyScoreSeries->setName(QStringLiteral("Total"));
    m_historyScoreSeries->setPen(QPen(scoreColor(0), 3));

    m_historySafetySeries = new QLineSeries();
    m_historySafetySeries->setName(QStringLiteral("Safety"));
    m_historySafetySeries->setPen(QPen(scoreColor(1), 2));

    m_historyRuleSeries = new QLineSeries();
    m_historyRuleSeries->setName(QStringLiteral("Rules"));
    m_historyRuleSeries->setPen(QPen(scoreColor(2), 2));

    m_historySmoothSeries = new QLineSeries();
    m_historySmoothSeries->setName(QStringLiteral("Smoothness"));
    m_historySmoothSeries->setPen(QPen(scoreColor(3), 2));

    m_historyEfficiencySeries = new QLineSeries();
    m_historyEfficiencySeries->setName(QStringLiteral("Efficiency"));
    m_historyEfficiencySeries->setPen(QPen(scoreColor(4), 2));

    m_historyChart = new QChart();
    m_historyChart->addSeries(m_historyScoreSeries);
    m_historyChart->addSeries(m_historySafetySeries);
    m_historyChart->addSeries(m_historyRuleSeries);
    m_historyChart->addSeries(m_historySmoothSeries);
    m_historyChart->addSeries(m_historyEfficiencySeries);

    m_historyAxisX = new QValueAxis();
    m_historyAxisX->setRange(1, 5);
    m_historyAxisX->setTitleText(QStringLiteral("Session"));

    m_historyAxisY = new QValueAxis();
    m_historyAxisY->setRange(0, 100);
    m_historyAxisY->setTitleText(QStringLiteral("Score"));

    m_historyChart->addAxis(m_historyAxisX, Qt::AlignBottom);
    m_historyChart->addAxis(m_historyAxisY, Qt::AlignLeft);

    const QList<QLineSeries*> series = {
        m_historyScoreSeries,
        m_historySafetySeries,
        m_historyRuleSeries,
        m_historySmoothSeries,
        m_historyEfficiencySeries
    };
    for (QLineSeries* s : series) {
        s->attachAxis(m_historyAxisX);
        s->attachAxis(m_historyAxisY);
    }

    m_historyChartView = new QChartView(m_historyChart, this);
    m_historyChartView->setRenderHint(QPainter::Antialiasing);
    m_historyChartView->setMinimumHeight(240);
}

void DrivingReportWidget::styleCharts()
{
    auto styleChart = [](QChart* chart) {
        if (!chart) {
            return;
        }
        chart->setBackgroundVisible(false);
        chart->setPlotAreaBackgroundVisible(true);
        chart->setPlotAreaBackgroundBrush(QColor(8, 14, 32, 170));
        chart->setMargins(QMargins(8, 8, 8, 8));
        chart->legend()->setLabelColor(QColor(232, 244, 255));
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->setTitleBrush(QBrush(QColor(232, 244, 255)));
    };

    styleChart(m_speedChart);
    styleChart(m_historyChart);

    auto styleAxis = [](QValueAxis* axis) {
        if (!axis) {
            return;
        }
        axis->setLabelsColor(QColor(180, 210, 240));
        axis->setLinePenColor(QColor(0, 180, 255, 120));
        axis->setGridLineColor(QColor(0, 180, 255, 35));
        axis->setTitleBrush(QBrush(QColor(160, 190, 220)));
    };

    styleAxis(m_speedAxisX);
    styleAxis(m_speedAxisY);
    styleAxis(m_historyAxisX);
    styleAxis(m_historyAxisY);

    m_speedChart->setTitle(QStringLiteral("实时速度曲线 / Live Speed Curve"));
    m_historyChart->setTitle(QStringLiteral("历史评分趋势 / Driving History Trend"));
}

void DrivingReportWidget::updateMockData()
{
    if (!m_useMockData) {
        return;
    }

    const int speed = QRandomGenerator::global()->bounded(35, 92);
    addSpeedData(speed);
}

void DrivingReportWidget::addSpeedData(qreal speed, qint64 timestamp)
{
    if (!m_speedSeries || !m_speedAxisX || !m_speedAxisY) {
        return;
    }

    if (timestamp < 0) {
        timestamp = m_tick;
    }

    m_speedSeries->append(timestamp, speed);

    while (m_speedSeries->count() > 0 && m_speedSeries->at(0).x() < timestamp - kChartWindowSeconds) {
        m_speedSeries->removePoints(0, 1);
    }

    m_speedAxisX->setRange(qMax<qreal>(0.0, timestamp - kChartWindowSeconds), qMax<qreal>(kChartWindowSeconds, timestamp));

    m_totalSpeed += speed;
    ++m_speedDataCount;
    m_maxRecordedSpeed = qMax(m_maxRecordedSpeed, speed);

    if (m_currentSpeedLabel) {
        m_currentSpeedLabel->setText(QString::number(static_cast<int>(speed)));
    }
    if (m_avgSpeedLabel) {
        m_avgSpeedLabel->setText(QString::number(m_totalSpeed / qMax(1, m_speedDataCount), 'f', 1));
    }
    if (m_maxSpeedLabel) {
        m_maxSpeedLabel->setText(QString::number(m_maxRecordedSpeed, 'f', 1));
    }

    ++m_tick;
    updateLiveChartPlaceholder();
}

void DrivingReportWidget::setCurrentReport(const ScoreReport& report)
{
    if (!m_applyingPlayerReport && m_hasPlayerReports) {
        clearPlayerReports();
    }

    m_currentReport = report;
    setMockDataEnabled(false);

    updateSummaryLabels();
    updateBreakdownBars();
    updateViolationTable();
    updateCoachAdvice();
    syncHistorySelection();

    emit reportUpdated(report);
}

void DrivingReportWidget::setReport(const ScoreReport& report)
{
    setCurrentReport(report);
}

void DrivingReportWidget::setPlayerReports(const ScoreReport& p1Report,
                                           const QList<DrivingData>& p1Samples,
                                           const ScoreReport& p2Report,
                                           const QList<DrivingData>& p2Samples)
{
    m_player1Report = p1Report;
    m_player1Samples = p1Samples;
    m_player2Report = p2Report;
    m_player2Samples = p2Samples;
    m_hasPlayerReports = true;
    m_activePlayerIndex = 1;

    updatePlayerSwitchState();
    applyPlayerReport(m_activePlayerIndex);
}

void DrivingReportWidget::clearPlayerReports()
{
    if (m_applyingPlayerReport) {
        return;
    }

    m_hasPlayerReports = false;
    m_activePlayerIndex = 1;
    m_player1Report = ScoreReport();
    m_player2Report = ScoreReport();
    m_player1Samples.clear();
    m_player2Samples.clear();

    updatePlayerSwitchState();
}

void DrivingReportWidget::setCoachReportMarkdown(const QString& markdown)
{
    if (!m_aiReportLabel) {
        return;
    }

    const QString text = markdown.trimmed();
    m_aiReportLabel->setText(text.isEmpty()
        ? QStringLiteral("Waiting for AI coach report...")
        : text);
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

void DrivingReportWidget::setSessionSpeedSamples(const QList<DrivingData>& samples)
{
    if (!m_speedSeries || !m_speedAxisX || !m_speedAxisY) {
        return;
    }

    m_speedSeries->clear();
    m_tick = 0;
    m_totalSpeed = 0.0;
    m_speedDataCount = 0;
    m_maxRecordedSpeed = 0.0;

    if (samples.isEmpty()) {
        updateLiveChartPlaceholder();
        return;
    }

    const qint64 baseTs = samples.first().timestamp;
    for (const DrivingData& sample : samples) {
        const qint64 relSec = qMax<qint64>(0, (sample.timestamp - baseTs) / 1000);
        addSpeedData(sample.speed, relSec);
    }

    updateLiveChartPlaceholder();
}

void DrivingReportWidget::addViolationEvent(const ViolationEvent& violation)
{
    m_violations.append(violation);
    if (m_currentReport.violations.isEmpty()) {
        m_currentReport.violations = m_violations;
    }
    updateViolationTable();
}

void DrivingReportWidget::clearData()
{
    clearPlayerReports();

    if (m_speedSeries) m_speedSeries->clear();
    m_violations.clear();
    m_historyReports.clear();
    if (m_historyList) m_historyList->clear();
    if (m_historyScoreSeries) m_historyScoreSeries->clear();
    if (m_historySafetySeries) m_historySafetySeries->clear();
    if (m_historyRuleSeries) m_historyRuleSeries->clear();
    if (m_historySmoothSeries) m_historySmoothSeries->clear();
    if (m_historyEfficiencySeries) m_historyEfficiencySeries->clear();
    if (m_violationTable) m_violationTable->setRowCount(0);

    m_tick = 0;
    m_totalSpeed = 0.0;
    m_speedDataCount = 0;
    m_maxRecordedSpeed = 0.0;
    m_currentReport = ScoreReport();

    updateSummaryLabels();
    updateBreakdownBars();
    updateCoachAdvice();
    updateHistoryChart();
    updateLiveChartPlaceholder();
    syncHistorySelection();
}

void DrivingReportWidget::applyPlayerReport(int playerIndex)
{
    if (!m_hasPlayerReports || m_applyingPlayerReport) {
        updatePlayerSwitchState();
        return;
    }

    const int normalizedPlayerIndex = (playerIndex == 2) ? 2 : 1;
    m_activePlayerIndex = normalizedPlayerIndex;
    updatePlayerSwitchState();

    const ScoreReport& report = (normalizedPlayerIndex == 1) ? m_player1Report : m_player2Report;
    const QList<DrivingData>& samples = (normalizedPlayerIndex == 1) ? m_player1Samples : m_player2Samples;

    m_applyingPlayerReport = true;
    setSessionSpeedSamples(samples);
    setCurrentReport(report);
    m_applyingPlayerReport = false;
    updatePlayerSwitchState();
}

void DrivingReportWidget::updatePlayerSwitchState()
{
    if (m_playerSwitchWidget) {
        m_playerSwitchWidget->setVisible(m_hasPlayerReports);
    }

    if (m_player1Button) {
        const bool isActive = m_hasPlayerReports && m_activePlayerIndex == 1;
        m_player1Button->setChecked(isActive);
    }
    if (m_player2Button) {
        const bool isActive = m_hasPlayerReports && m_activePlayerIndex == 2;
        m_player2Button->setChecked(isActive);
    }
}

void DrivingReportWidget::updateBreakdownBars()
{
    const auto clampScore = [](qreal value) {
        return qBound(0, qRound(value), 100);
    };

    const int safety = clampScore(m_currentReport.breakdown.safetyScore);
    const int rule = clampScore(m_currentReport.breakdown.ruleComplianceScore);
    const int smooth = clampScore(m_currentReport.breakdown.smoothnessScore);
    const int efficiency = clampScore(m_currentReport.breakdown.efficiencyScore);

    if (m_safetyValueLabel) m_safetyValueLabel->setText(QString::number(safety));
    if (m_ruleValueLabel) m_ruleValueLabel->setText(QString::number(rule));
    if (m_smoothValueLabel) m_smoothValueLabel->setText(QString::number(smooth));
    if (m_efficiencyValueLabel) m_efficiencyValueLabel->setText(QString::number(efficiency));

    if (m_safetyBar) {
        m_safetyBar->setObjectName(QStringLiteral("scoreBar"));
        m_safetyBar->setStyleSheet(QStringLiteral("QProgressBar#scoreBar { background-color: rgba(8,14,32,180); border: none; border-radius: 5px; height: 10px; } QProgressBar#scoreBar::chunk { background-color: #FF4466; border-radius: 5px; }"));
        m_safetyBar->setValue(safety);
    }
    if (m_ruleBar) {
        m_ruleBar->setObjectName(QStringLiteral("scoreBar"));
        m_ruleBar->setStyleSheet(QStringLiteral("QProgressBar#scoreBar { background-color: rgba(8,14,32,180); border: none; border-radius: 5px; height: 10px; } QProgressBar#scoreBar::chunk { background-color: #00B4FF; border-radius: 5px; }"));
        m_ruleBar->setValue(rule);
    }
    if (m_smoothBar) {
        m_smoothBar->setObjectName(QStringLiteral("scoreBar"));
        m_smoothBar->setStyleSheet(QStringLiteral("QProgressBar#scoreBar { background-color: rgba(8,14,32,180); border: none; border-radius: 5px; height: 10px; } QProgressBar#scoreBar::chunk { background-color: #00FFAA; border-radius: 5px; }"));
        m_smoothBar->setValue(smooth);
    }
    if (m_efficiencyBar) {
        m_efficiencyBar->setObjectName(QStringLiteral("scoreBar"));
        m_efficiencyBar->setStyleSheet(QStringLiteral("QProgressBar#scoreBar { background-color: rgba(8,14,32,180); border: none; border-radius: 5px; height: 10px; } QProgressBar#scoreBar::chunk { background-color: #FFD700; border-radius: 5px; }"));
        m_efficiencyBar->setValue(efficiency);
    }
}

void DrivingReportWidget::updateViolationTable()
{
    const QList<ViolationEvent> source = !m_currentReport.violations.isEmpty()
        ? m_currentReport.violations
        : m_violations;

    m_violationTable->setRowCount(0);

    if (source.isEmpty()) {
        m_violationTable->insertRow(0);
        m_violationTable->setSpan(0, 0, 1, 4);
        QTableWidgetItem* item = new QTableWidgetItem(QStringLiteral("No violations. Clean drive, keep it up."));
        item->setTextAlignment(Qt::AlignCenter);
        m_violationTable->setItem(0, 0, item);
        return;
    }

    for (const ViolationEvent& v : source) {
        const int row = m_violationTable->rowCount();
        m_violationTable->insertRow(row);

        const QDateTime dt = QDateTime::fromMSecsSinceEpoch(v.timestamp);
        m_violationTable->setItem(row, 0, new QTableWidgetItem(dt.isValid() ? dt.toString(QStringLiteral("hh:mm:ss")) : QStringLiteral("--:--:--")));
        m_violationTable->setItem(row, 1, new QTableWidgetItem(violationTypeDisplay(v.type)));
        m_violationTable->setItem(row, 2, new QTableWidgetItem(v.description.isEmpty() ? QStringLiteral("Violation event") : v.description));
        m_violationTable->setItem(row, 3, new QTableWidgetItem(QStringLiteral("-%1").arg(v.penaltyPoints)));
    }
}

void DrivingReportWidget::updateCoachAdvice()
{
    if (!m_aiReportLabel) {
        return;
    }

    QStringList lines;
    if (!m_currentReport.summary.trimmed().isEmpty()) {
        lines << m_currentReport.summary.trimmed();
    }

    for (const CoachAdvice& advice : m_currentReport.coachAdvices) {
        lines << QStringLiteral("[%1 / %2] %3")
                     .arg(advice.category.isEmpty() ? QStringLiteral("Coach") : advice.category,
                          advice.severity.isEmpty() ? QStringLiteral("info") : advice.severity,
                          advice.message);
    }

    if (lines.isEmpty()) {
        lines << QStringLiteral("Waiting for AI coach report...");
    }

    m_aiReportLabel->setText(lines.join(QStringLiteral("\n\n")));
}

void DrivingReportWidget::updateHistoryChart()
{
    if (!m_historyScoreSeries || !m_historySafetySeries || !m_historyRuleSeries
        || !m_historySmoothSeries || !m_historyEfficiencySeries
        || !m_historyAxisX || !m_historyAxisY) {
        return;
    }

    m_historyScoreSeries->clear();
    m_historySafetySeries->clear();
    m_historyRuleSeries->clear();
    m_historySmoothSeries->clear();
    m_historyEfficiencySeries->clear();

    int x = 1;
    for (const ScoreReport& report : m_historyReports) {
        m_historyScoreSeries->append(x, report.totalScore);
        m_historySafetySeries->append(x, report.breakdown.safetyScore);
        m_historyRuleSeries->append(x, report.breakdown.ruleComplianceScore);
        m_historySmoothSeries->append(x, report.breakdown.smoothnessScore);
        m_historyEfficiencySeries->append(x, report.breakdown.efficiencyScore);
        ++x;
    }

    if (m_historyReports.isEmpty()) {
        m_historyAxisX->setRange(1, 5);
    } else {
        m_historyAxisX->setRange(1, qMax(5, m_historyReports.size()));
    }
    m_historyAxisY->setRange(0, 100);

    updateHistoryPlaceholder();
}

void DrivingReportWidget::updateHistoryList()
{
    if (!m_historyList) {
        return;
    }

    QSignalBlocker blocker(m_historyList);
    m_historyList->clear();

    for (const ScoreReport& report : m_historyReports) {
        m_historyList->addItem(historyRecordLabel(report));
    }
}

void DrivingReportWidget::updateHistoryPlaceholder()
{
    if (!m_historyPlaceholderLabel) {
        return;
    }
    m_historyPlaceholderLabel->setVisible(m_historyReports.isEmpty());
    if (m_historyList) {
        m_historyList->setVisible(!m_historyReports.isEmpty());
    }
}

void DrivingReportWidget::updateLiveChartPlaceholder()
{
    if (!m_livePlaceholderLabel) {
        return;
    }
    m_livePlaceholderLabel->setVisible(!m_speedSeries || m_speedSeries->count() == 0);
}

void DrivingReportWidget::updateSummaryLabels()
{
    if (m_totalScoreLabel) {
        // Show 0.0 as a valid score; only show "--" when no report has been set.
        m_totalScoreLabel->setText(m_currentReport.sessionId.isEmpty()
            ? QStringLiteral("--")
            : QString::number(m_currentReport.totalScore, 'f', 1));
    }
    if (m_gradeLabel) {
        m_gradeLabel->setText(m_currentReport.grade.isEmpty() ? QStringLiteral("--") : m_currentReport.grade);
    }
    if (m_violationCountLabel) {
        const int count = !m_currentReport.violations.isEmpty() ? m_currentReport.violations.size() : m_violations.size();
        m_violationCountLabel->setText(QString::number(count));
    }

    if (m_currentReport.metrics.averageSpeed > 0.0) {
        m_avgSpeedLabel->setText(QString::number(m_currentReport.metrics.averageSpeed, 'f', 1));
    } else if (m_speedDataCount == 0) {
        m_avgSpeedLabel->setText(QStringLiteral("--"));
    }

    if (m_currentReport.metrics.maxSpeed > 0.0) {
        m_maxSpeedLabel->setText(QString::number(m_currentReport.metrics.maxSpeed, 'f', 1));
    } else if (m_maxRecordedSpeed <= 0.0) {
        m_maxSpeedLabel->setText(QStringLiteral("--"));
    }

    if (m_currentSpeedLabel && m_speedSeries && m_speedSeries->count() == 0) {
        m_currentSpeedLabel->setText(QStringLiteral("--"));
    }
}

QString DrivingReportWidget::violationTypeDisplay(ViolationType type)
{
    switch (type) {
    case ViolationType::RedLight:
        return QStringLiteral("Red Light");
    case ViolationType::SpeedOverLimit:
        return QStringLiteral("Speeding");
    case ViolationType::PedestrianCollision:
        return QStringLiteral("Pedestrian");
    case ViolationType::WrongWay:
        return QStringLiteral("Wrong Way");
    case ViolationType::Collision:
        return QStringLiteral("Collision");
    default:
        return QStringLiteral("Info");
    }
}

QColor DrivingReportWidget::scoreColor(int index)
{
    switch (index) {
    case 0: return QColor(255, 215, 0);
    case 1: return QColor(255, 68, 102);
    case 2: return QColor(0, 180, 255);
    case 3: return QColor(0, 255, 170);
    default: return QColor(180, 79, 255);
    }
}

void DrivingReportWidget::loadHistoryFromSaveLoadManager()
{
    SaveLoadManager& manager = SaveLoadManager::instance();
    loadHistoryReports(manager.loadHistory());
}

void DrivingReportWidget::loadHistoryReports(const QList<ScoreReport>& reports)
{
    m_historyReports = reports;
    updateHistoryList();
    updateHistoryChart();
    syncHistorySelection();
}

void DrivingReportWidget::syncHistorySelection()
{
    if (!m_historyList) {
        return;
    }

    QSignalBlocker blocker(m_historyList);
    m_syncingHistorySelection = true;

    int matchingRow = -1;
    if (!m_currentReport.sessionId.isEmpty()) {
        for (int i = 0; i < m_historyReports.size(); ++i) {
            if (m_historyReports.at(i).sessionId == m_currentReport.sessionId) {
                matchingRow = i;
                break;
            }
        }
    }

    m_historyList->setCurrentRow(matchingRow);
    if (matchingRow >= 0) {
        m_historyList->scrollToItem(m_historyList->item(matchingRow));
    }

    m_syncingHistorySelection = false;
}

QString DrivingReportWidget::historyRecordLabel(const ScoreReport& report) const
{
    QString title;
    if (report.generatedAt.isValid()) {
        title = report.generatedAt.toLocalTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    } else if (!report.sessionId.isEmpty()) {
        title = report.sessionId;
    } else {
        title = QStringLiteral("Unknown session");
    }

    return QStringLiteral("%1 | Score %2 | Grade %3 | Violations %4")
        .arg(title,
             QString::number(report.totalScore, 'f', 1),
             report.grade.isEmpty() ? QStringLiteral("--") : report.grade,
             QString::number(report.violations.size()));
}

void DrivingReportWidget::showLoading()
{
    if (!m_loadingOverlay) {
        return;
    }
    m_loadingDotCount = 0;
    if (m_loadingLabel) {
        m_loadingLabel->setText(QStringLiteral("正在计算驾驶报告…"));
    }
    m_loadingOverlay->setGeometry(rect());
    m_loadingOverlay->raise();
    m_loadingOverlay->show();
    if (m_loadingDotTimer) {
        m_loadingDotTimer->start();
    }
}

void DrivingReportWidget::hideLoading()
{
    if (m_loadingDotTimer) {
        m_loadingDotTimer->stop();
    }
    if (m_loadingOverlay) {
        m_loadingOverlay->hide();
    }
}

void DrivingReportWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_loadingOverlay) {
        m_loadingOverlay->setGeometry(rect());
    }
}

} // namespace PhantomDrive
