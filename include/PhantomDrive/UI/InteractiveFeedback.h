#ifndef INTERACTIVE_FEEDBACK_H
#define INTERACTIVE_FEEDBACK_H

#include <QWidget>
#include <QMap>
#include <QQueue>
#include <QVector>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QString>

#include "PhantomDrive_global.h"

namespace PhantomDrive {

enum class FeedbackType {
    Positive,
    Warning,
    Critical,
    Powerup,
    Milestone,
    Countdown,
    Default
};

struct FeedbackMessage {
    QString text;
    FeedbackType type;
    int priority;
    int durationMs;
    bool autoHide;

    FeedbackMessage(const QString& t = "", FeedbackType ft = FeedbackType::Default,
                    int prio = 0, int durMs = 2000, bool hide = true)
        : text(t), type(ft), priority(prio), durationMs(durMs), autoHide(hide) {}
};

class PHANTOMDRIVE_EXPORT InteractiveFeedback : public QWidget
{
    Q_OBJECT
public:
    static InteractiveFeedback& instance(QWidget* parent = nullptr);

    // Set the game-map widget so notifications are always positioned inside it,
    // never over the HUD panel.
    void setGameView(QWidget* gameView);

    void showFeedback(const QString& message, FeedbackType type = FeedbackType::Default);
    void showFeedback(const QString& message, int points, FeedbackType type = FeedbackType::Warning);

    void showCountdown(int seconds);
    void showGo();
    void showLapCompleted(int lapNumber);
    void showCheckpoint(int checkpointNumber);
    void showSpeedBoost(bool active);
    void showShield(bool active);

    void clearAll();
    void pause();
    void resume();

signals:
    void feedbackShown(const QString& message, FeedbackType type);
    void feedbackHidden(const QString& message);

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onQueueTimer();

private:
    explicit InteractiveFeedback(QWidget* parent = nullptr);
    ~InteractiveFeedback();

    void setupUI();
    void ensureOverlayVisible();
    void updatePosition();
    void enqueueMessage(const FeedbackMessage& msg);
    void displayMessage(const FeedbackMessage& msg);
    void animateMessage(QWidget* label, const FeedbackMessage& msg);
    void relayoutActiveLabels();
    QString getStyleSheetForType(FeedbackType type, const QString& baseText);
    QColor getColorForType(FeedbackType type);
    QString getIconForType(FeedbackType type);

    static InteractiveFeedback* s_instance;

    QWidget* m_containerWidget;
    QWidget* m_gameView;   // optional: if set, notifications are centred inside it
    QVector<QWidget*> m_activeLabels;
    QQueue<FeedbackMessage> m_messageQueue;
    QTimer* m_queueTimer;
    QTimer* m_positionTimer;

    int m_maxVisibleMessages;
    int m_messageSpacing;
    bool m_isPaused;
    bool m_centerMode;

    static const int MAX_ACTIVE_MESSAGES = 3;
    static const int QUEUE_INTERVAL_MS = 100;
};

} // namespace PhantomDrive

#endif // INTERACTIVE_FEEDBACK_H
