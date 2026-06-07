#include "UI/InteractiveFeedback.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QFont>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>

using namespace PhantomDrive;

InteractiveFeedback* InteractiveFeedback::s_instance = nullptr;

InteractiveFeedback& InteractiveFeedback::instance(QWidget* parent)
{
    if (!s_instance) {
        s_instance = new InteractiveFeedback(parent);
        s_instance->show();
    }
    return *s_instance;
}

InteractiveFeedback::InteractiveFeedback(QWidget* parent)
    : QWidget(parent)
    , m_containerWidget(nullptr)
    , m_gameView(nullptr)
    , m_queueTimer(new QTimer(this))
    , m_positionTimer(new QTimer(this))
    , m_maxVisibleMessages(MAX_ACTIVE_MESSAGES)
    , m_messageSpacing(10)
    , m_isPaused(false)
    , m_centerMode(false)
{
    setupUI();

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    m_queueTimer->setInterval(QUEUE_INTERVAL_MS);
    connect(m_queueTimer, &QTimer::timeout, this, &InteractiveFeedback::onQueueTimer);
    m_queueTimer->start();

    connect(m_positionTimer, &QTimer::timeout, this, &InteractiveFeedback::updatePosition);
    m_positionTimer->setInterval(100);
    m_positionTimer->start();
}

InteractiveFeedback::~InteractiveFeedback()
{
    clearAll();
}

void InteractiveFeedback::setupUI()
{
    m_containerWidget = new QWidget(this);
    m_containerWidget->setObjectName("feedbackContainer");
    m_containerWidget->setStyleSheet(R"(
        #feedbackContainer {
            background: transparent;
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(m_messageSpacing);
    mainLayout->addWidget(m_containerWidget, 0, Qt::AlignCenter);
}

void InteractiveFeedback::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    updatePosition();
}

void InteractiveFeedback::setGameView(QWidget* gameView)
{
    m_gameView = gameView;
    updatePosition();
}

void InteractiveFeedback::ensureOverlayVisible()
{
    if (!isVisible()) {
        show();
    }
    updatePosition();
    raise();
}

void InteractiveFeedback::updatePosition()
{
    if (!isVisible()) return;

    // Prefer the explicit game-view widget for accurate positioning.
    if (m_gameView) {
        const QRect gvRect = m_gameView->rect();
        const QPoint gvGlobal = m_gameView->mapToGlobal(QPoint(0, 0));

        if (m_centerMode) {
            setFixedSize(460, 260);
            const int x = gvGlobal.x() + (gvRect.width() - width()) / 2;
            const int y = gvGlobal.y() + (gvRect.height() - height()) / 2;
            move(x, y);
        } else {
            setFixedSize(360, qMin(260, qMax(160, gvRect.height() / 3)));
            move(gvGlobal.x() + gvRect.width() - width() - 20,
                 gvGlobal.y() + 72);
        }
        if (m_containerWidget) {
            m_containerWidget->setGeometry(rect());
        }
        return;
    }

    const QWidget* anchor = parentWidget();
    QRect anchorRect;
    QPoint anchorTopLeft;

    if (anchor) {
        anchorRect = anchor->rect();
        anchorTopLeft = anchor->mapToGlobal(QPoint(0, 0));
    } else {
        QScreen* screen = QApplication::primaryScreen();
        if (!screen) return;
        anchorRect = screen->geometry();
        anchorTopLeft = anchorRect.topLeft();
    }

    if (m_centerMode) {
        setFixedSize(460, 260);
        // Position in the game-map area only (left portion, excluding the HUD panel
        // on the right which is ~300 px wide).  This prevents any overlay from
        // covering the HUD data.
        const int hudWidth = 320; // conservative estimate of HUD panel width
        const int mapWidth = qMax(200, anchorRect.width() - hudWidth);
        const int x = anchorTopLeft.x() + (mapWidth - width()) / 2;
        const int y = anchorTopLeft.y() + (anchorRect.height() - height()) / 2;
        move(x, y);
    } else {
        setFixedSize(360, qMin(260, qMax(160, anchorRect.height() / 3)));
        // Side mode: keep in the map area, not over the HUD
        const int hudWidth = 320;
        const int mapWidth = qMax(200, anchorRect.width() - hudWidth);
        move(anchorTopLeft.x() + mapWidth - width() - 20,
             anchorTopLeft.y() + 72);
    }
    if (m_containerWidget) {
        m_containerWidget->setGeometry(rect());
    }
}

void InteractiveFeedback::showFeedback(const QString& message, FeedbackType type)
{
    enqueueMessage(FeedbackMessage(message, type, static_cast<int>(type) + 1, 2000, true));
}

void InteractiveFeedback::showFeedback(const QString& message, int points, FeedbackType type)
{
    QString displayText;
    if (points >= 0) {
        displayText = QString("%1").arg(message);
    } else {
        displayText = QString("%1 %2").arg(message).arg(points);
    }

    FeedbackMessage msg(displayText, type, static_cast<int>(type) + 1, 2500, true);
    enqueueMessage(msg);
}

void InteractiveFeedback::showCountdown(int seconds)
{
    for (int i = seconds; i > 0; --i) {
        FeedbackMessage msg(QString::number(i), FeedbackType::Countdown,
                          100, 800, true);
        enqueueMessage(msg);
    }
}

void InteractiveFeedback::showGo()
{
    FeedbackMessage msg("GO!", FeedbackType::Milestone, 100, 1500, true);
    enqueueMessage(msg);
}

void InteractiveFeedback::showLapCompleted(int lapNumber)
{
    QString msg = QString("Lap %1 Completed!").arg(lapNumber);
    enqueueMessage(FeedbackMessage(msg, FeedbackType::Milestone, 90, 3000, true));
}

void InteractiveFeedback::showCheckpoint(int checkpointNumber)
{
    QString msg = QString("Checkpoint %1").arg(checkpointNumber);
    enqueueMessage(FeedbackMessage(msg, FeedbackType::Milestone, 80, 1500, true));
}

void InteractiveFeedback::showSpeedBoost(bool active)
{
    if (active) {
        showFeedback("Speed Boost!", FeedbackType::Powerup);
    }
}

void InteractiveFeedback::showShield(bool active)
{
    if (active) {
        showFeedback("Shield Active!", FeedbackType::Powerup);
    }
}

void InteractiveFeedback::clearAll()
{
    m_messageQueue.clear();

    for (QWidget* label : m_activeLabels) {
        label->deleteLater();
    }
    m_activeLabels.clear();
    relayoutActiveLabels();
}

void InteractiveFeedback::pause()
{
    m_isPaused = true;
    m_queueTimer->stop();
}

void InteractiveFeedback::resume()
{
    m_isPaused = false;
    m_queueTimer->start();
}

void InteractiveFeedback::enqueueMessage(const FeedbackMessage& msg)
{
    ensureOverlayVisible();
    m_messageQueue.enqueue(msg);
}

void InteractiveFeedback::onQueueTimer()
{
    if (m_isPaused || m_messageQueue.isEmpty()) return;

    if (m_activeLabels.size() < m_maxVisibleMessages) {
        FeedbackMessage msg = m_messageQueue.dequeue();
        displayMessage(msg);
    }
}

void InteractiveFeedback::displayMessage(const FeedbackMessage& msg)
{
    ensureOverlayVisible();

    QLabel* label = new QLabel(msg.text, m_containerWidget);
    label->setAlignment(Qt::AlignCenter);
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    label->setWordWrap(true);

    QString styleSheet = getStyleSheetForType(msg.type, msg.text);
    label->setStyleSheet(styleSheet);

    label->setFont(QFont("Segoe UI", 18, QFont::Bold));

    label->adjustSize();
    const int maxLabelWidth = qMax(220, width() - 28);
    const int labelWidth = qMin(qMax(label->width() + 44, 180), maxLabelWidth);
    const int labelHeight = msg.text.size() > 28 ? 68 : 54;
    label->setFixedSize(labelWidth, labelHeight);

    auto* opacity = new QGraphicsOpacityEffect(label);
    opacity->setOpacity(0.0);
    label->setGraphicsEffect(opacity);

    label->move((m_containerWidget->width() - labelWidth) / 2, 0);
    label->show();

    m_activeLabels.append(label);
    relayoutActiveLabels();

    animateMessage(label, msg);

    emit feedbackShown(msg.text, msg.type);
}

void InteractiveFeedback::relayoutActiveLabels()
{
    if (!m_containerWidget) {
        return;
    }

    const int count = m_activeLabels.size();
    if (count <= 0) {
        return;
    }

    int totalHeight = 0;
    for (QWidget* w : m_activeLabels) {
        if (w) {
            totalHeight += w->height() + m_messageSpacing;
        }
    }
    totalHeight = qMax(0, totalHeight - m_messageSpacing);
    const int baseY = m_centerMode ? qMax(0, (height() - totalHeight) / 2) : 0;
    int y = baseY;

    for (int i = 0; i < count; ++i) {
        QWidget* w = m_activeLabels.at(i);
        if (!w) {
            continue;
        }
        const int x = (m_containerWidget->width() - w->width()) / 2;
        w->move(x, y);
        y += w->height() + m_messageSpacing;
    }
}

void InteractiveFeedback::animateMessage(QWidget* label, const FeedbackMessage& msg)
{
    QParallelAnimationGroup* group = new QParallelAnimationGroup(label);
    auto* opacity = qobject_cast<QGraphicsOpacityEffect*>(label->graphicsEffect());

    QPropertyAnimation* scaleUp = new QPropertyAnimation(label, "size");
    scaleUp->setDuration(150);
    scaleUp->setStartValue(QSize(10, 10));
    scaleUp->setEndValue(label->size());
    scaleUp->setEasingCurve(QEasingCurve::OutBack);
    group->addAnimation(scaleUp);

    if (opacity) {
        QPropertyAnimation* opacityIn = new QPropertyAnimation(opacity, "opacity");
        opacityIn->setDuration(150);
        opacityIn->setStartValue(0.0);
        opacityIn->setEndValue(1.0);
        group->addAnimation(opacityIn);
    }

    group->start();

    QTimer::singleShot(msg.durationMs, this, [this, label, msg]() {
        if (!m_activeLabels.contains(label)) return;

        QParallelAnimationGroup* fadeGroup = new QParallelAnimationGroup(label);
        auto* opacity = qobject_cast<QGraphicsOpacityEffect*>(label->graphicsEffect());

        if (opacity) {
            QPropertyAnimation* fadeOut = new QPropertyAnimation(opacity, "opacity");
            fadeOut->setDuration(300);
            fadeOut->setStartValue(1.0);
            fadeOut->setEndValue(0.0);
            fadeGroup->addAnimation(fadeOut);
        }

        QPropertyAnimation* moveUp = new QPropertyAnimation(label, "pos");
        moveUp->setDuration(300);
        QPoint currentPos = label->pos();
        moveUp->setStartValue(currentPos);
        moveUp->setEndValue(QPoint(currentPos.x(), currentPos.y() - 30));
        moveUp->setEasingCurve(QEasingCurve::OutQuad);
        fadeGroup->addAnimation(moveUp);

        fadeGroup->start();

        connect(fadeGroup, &QParallelAnimationGroup::finished, this, [this, label, msg, fadeGroup]() {
            fadeGroup->deleteLater();
            m_activeLabels.removeOne(label);
            label->deleteLater();
            relayoutActiveLabels();
            emit feedbackHidden(msg.text);
        });
    });
}

QString InteractiveFeedback::getStyleSheetForType(FeedbackType type, const QString& baseText)
{
    QColor bgColor;
    QColor textColor;
    QString borderColor;

    switch (type) {
    case FeedbackType::Positive:
        bgColor = QColor(46, 204, 113, 230);
        textColor = Qt::white;
        borderColor = "#27AE60";
        break;
    case FeedbackType::Warning:
        bgColor = QColor(243, 156, 18, 230);
        textColor = Qt::white;
        borderColor = "#F39C12";
        break;
    case FeedbackType::Critical:
        bgColor = QColor(231, 76, 60, 230);
        textColor = Qt::white;
        borderColor = "#E74C3C";
        break;
    case FeedbackType::Powerup:
        bgColor = QColor(155, 89, 182, 230);
        textColor = Qt::white;
        borderColor = "#9B59B6";
        break;
    case FeedbackType::Milestone:
        bgColor = QColor(52, 152, 219, 230);
        textColor = Qt::white;
        borderColor = "#3498DB";
        break;
    case FeedbackType::Countdown:
        bgColor = QColor(44, 62, 80, 240);
        textColor = QColor(241, 196, 15);
        borderColor = "#F1C40F";
        return QString(R"(
            QLabel {
                background-color: rgba(%1, %2, %3, %4);
                color: #%5;
                border: 3px solid %6;
                border-radius: 10px;
                padding: 5px 15px;
                font-size: 48px;
                font-weight: bold;
                font-family: 'Segoe UI', 'Arial', sans-serif;
            }
        )").arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue())
           .arg(bgColor.alpha()).arg(textColor.name().mid(1))
           .arg(borderColor);
    default:
        bgColor = QColor(44, 62, 80, 220);
        textColor = Qt::white;
        borderColor = "#34495E";
        break;
    }

    return QString(R"(
        QLabel {
            background-color: rgba(%1, %2, %3, %4);
            color: #%5;
            border: 2px solid %6;
            border-radius: 8px;
            padding: 8px 20px;
            font-size: 18px;
            font-weight: bold;
            font-family: 'Segoe UI', 'Arial', sans-serif;
        }
    )").arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue())
       .arg(bgColor.alpha()).arg(textColor.name().mid(1))
       .arg(borderColor);
}

QColor InteractiveFeedback::getColorForType(FeedbackType type)
{
    switch (type) {
    case FeedbackType::Positive: return QColor(46, 204, 113);
    case FeedbackType::Warning: return QColor(243, 156, 18);
    case FeedbackType::Critical: return QColor(231, 76, 60);
    case FeedbackType::Powerup: return QColor(155, 89, 182);
    case FeedbackType::Milestone: return QColor(52, 152, 219);
    case FeedbackType::Countdown: return QColor(241, 196, 15);
    default: return QColor(189, 195, 199);
    }
}

QString InteractiveFeedback::getIconForType(FeedbackType type)
{
    switch (type) {
    case FeedbackType::Positive: return "★";
    case FeedbackType::Warning: return "⚠";
    case FeedbackType::Critical: return "✖";
    case FeedbackType::Powerup: return "◆";
    case FeedbackType::Milestone: return "►";
    default: return "";
    }
}
