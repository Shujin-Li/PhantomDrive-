#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QProgressBar>
#include <QListWidget>
#include <QHash>
#include <QTableWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QStackedWidget>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "PhantomDrive_global.h"
#include "scoring/ScoreReport.h"
#include "gamemode/DrivingData.h"

class QPushButton;

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT DrivingReportWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrivingReportWidget(QWidget *parent = nullptr);
    ~DrivingReportWidget();

    // ---- live data feed ----
    void addSpeedData(qreal speed, qint64 timestamp = -1);
    void setSessionSpeedSamples(const QList<DrivingData>& samples);
    void addViolationEvent(const ViolationEvent& violation);

    // ---- report data ----
    void setCurrentReport(const ScoreReport& report);
    void setReport(const ScoreReport& report);
    void setCoachReportMarkdown(const QString& markdown);
    void setCoachReportMarkdownForPlayer(int playerIndex, const QString& markdown);
    void setCoachReportMarkdownForPlayer(int playerIndex, const QString& sessionId, const QString& markdown);
    void setPlayerReports(const ScoreReport& p1Report,
                          const QList<DrivingData>& p1Samples,
                          const ScoreReport& p2Report,
                          const QList<DrivingData>& p2Samples);
    void clearPlayerReports();

    // ---- history ----
    void loadHistoryFromSaveLoadManager();
    void loadHistoryReports(const QList<ScoreReport>& reports);

    // ---- loading skeleton ----
    void showLoading();
    void hideLoading();

    // ---- control ----
    void clearData();
    void setMockDataEnabled(bool enabled);

signals:
    void reportUpdated(const ScoreReport& report);
    // Emitted when the user clicks "Back to Menu" in the report panel.
    void backToMenuRequested();
    // Emitted when the user clicks "New Drive" in the report panel.
    void newDriveRequested();

private slots:
    void updateMockData();

private:
    // ---- build helpers ----
    void setupUI();
    void setupHeaderBar(QVBoxLayout* rootLayout);
    void setupSummarySection(QVBoxLayout* rootLayout);
    void setupMiddleSection(QVBoxLayout* rootLayout);
    void setupBottomSection(QVBoxLayout* rootLayout);
    void setupCharts();
    void styleCharts();

    QFrame*  createCard(const QString& title, QWidget* content);
    QWidget* createSummaryCard(const QString& title, QLabel*& valueLabel, const QString& objectName);

    // ---- update helpers ----
    void updateSummaryLabels();
    void updateBreakdownBars();
    void updateViolationTable();
    void updateCoachAdvice();
    void updateHistoryChart();
    void updateHistoryList();
    void updateHistoryPlaceholder();
    void updateLiveChartPlaceholder();
    void applyPlayerReport(int playerIndex);
    void updatePlayerSwitchState();
    void syncHistorySelection();
    QString historyRecordLabel(const ScoreReport& report) const;

    void resizeEvent(QResizeEvent* event) override;

    static QString violationTypeDisplay(ViolationType type);
    static QColor  scoreColor(int index);

    // ---- layout ----
    QScrollArea* m_scrollArea;
    QWidget*     m_scrollContent;

    // ---- placeholders ----
    QLabel* m_historyPlaceholderLabel;
    QLabel* m_livePlaceholderLabel;

    // ---- loading overlay ----
    QWidget* m_loadingOverlay;
    QLabel*  m_loadingLabel;
    QTimer*  m_loadingDotTimer;
    int      m_loadingDotCount;

    // ---- player report switch ----
    QWidget*     m_playerSwitchWidget;
    QPushButton* m_player1Button;
    QPushButton* m_player2Button;

    // ---- summary labels ----
    QLabel* m_avgSpeedLabel;
    QLabel* m_maxSpeedLabel;
    QLabel* m_currentSpeedLabel;
    QLabel* m_totalScoreLabel;
    QLabel* m_gradeLabel;
    QLabel* m_violationCountLabel;

    // ---- score breakdown ----
    QLabel*       m_safetyValueLabel;
    QLabel*       m_ruleValueLabel;
    QLabel*       m_smoothValueLabel;
    QLabel*       m_efficiencyValueLabel;
    QProgressBar* m_safetyBar;
    QProgressBar* m_ruleBar;
    QProgressBar* m_smoothBar;
    QProgressBar* m_efficiencyBar;

    // ---- live speed chart ----
    QSplineSeries* m_speedSeries;
    QChart*        m_speedChart;
    QChartView*    m_speedChartView;
    QValueAxis*    m_speedAxisX;
    QValueAxis*    m_speedAxisY;

    // ---- coach advice ----
    QWidget* m_coachAdviceWidget;
    QTextBrowser* m_aiReportLabel;

    // ---- history chart / list ----
    QListWidget* m_historyList;
    QLineSeries* m_historyScoreSeries;
    QLineSeries* m_historySafetySeries;
    QLineSeries* m_historyRuleSeries;
    QLineSeries* m_historySmoothSeries;
    QLineSeries* m_historyEfficiencySeries;
    QChart*      m_historyChart;
    QChartView*  m_historyChartView;
    QValueAxis*  m_historyAxisX;
    QValueAxis*  m_historyAxisY;

    // ---- violation table ----
    QTableWidget* m_violationTable;

    // ---- state ----
    QTimer*            m_timer;
    int                m_tick;
    qreal              m_totalSpeed;
    int                m_speedDataCount;
    qreal              m_maxRecordedSpeed;
    bool               m_useMockData;
    ScoreReport        m_currentReport;
    QList<ViolationEvent> m_violations;
    QList<ScoreReport> m_historyReports;
    ScoreReport        m_player1Report;
    ScoreReport        m_player2Report;
    QList<DrivingData> m_player1Samples;
    QList<DrivingData> m_player2Samples;
    bool               m_hasPlayerReports;
    int                m_activePlayerIndex;
    bool               m_applyingPlayerReport;
    bool               m_syncingHistorySelection;
    QHash<int, QString> m_aiCoachMarkdownByPlayer;
    QHash<int, QString> m_aiCoachSessionIdByPlayer;
};

} // namespace PhantomDrive
