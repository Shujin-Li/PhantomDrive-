#include "UI/MainMenuWidget.h"

#include "UI/MenuCardWidget.h"
#include "UI/StartGameOverlay.h"

#include <QApplication>
#include <QDir>
#include <QEasingCurve>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>

namespace {

constexpr qreal kReferenceDesignWidth = 1672.0;
constexpr qreal kReferenceDesignHeight = 941.0;

QString resolveAssetPath(const QString& relativePath)
{
    QStringList candidates;
    candidates.append(QDir::current().absoluteFilePath(relativePath));

    QDir appDir(QApplication::applicationDirPath());
    for (int depth = 0; depth < 8; ++depth) {
        candidates.append(appDir.absoluteFilePath(relativePath));
        if (!appDir.cdUp()) {
            break;
        }
    }

    for (const QString& candidate : candidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile()) {
            return info.absoluteFilePath();
        }
    }

    return QString();
}

QRectF coverPixmapRect(const QPixmap& pixmap, const QRectF& targetRect)
{
    if (pixmap.isNull() || targetRect.isEmpty()) {
        return targetRect;
    }

    QSizeF scaledSize = pixmap.size();
    scaledSize.scale(targetRect.size(), Qt::KeepAspectRatioByExpanding);
    const QPointF topLeft(targetRect.center().x() - scaledSize.width() / 2.0,
                          targetRect.center().y() - scaledSize.height() / 2.0);
    return QRectF(topLeft, scaledSize);
}

void drawCoverPixmap(QPainter& painter, const QRectF& targetRect, const QPixmap& pixmap)
{
    if (pixmap.isNull() || targetRect.isEmpty()) {
        return;
    }

    const QRectF drawRect = coverPixmapRect(pixmap, targetRect);

    painter.save();
    painter.setClipRect(targetRect);
    painter.drawPixmap(drawRect, pixmap, QRectF(pixmap.rect()));
    painter.restore();
}

QRect mapReferenceRect(const QRectF& designRect,
                       qreal designWidth,
                       qreal designHeight,
                       qreal x,
                       qreal y,
                       qreal width,
                       qreal height)
{
    const qreal scaleX = designRect.width() / designWidth;
    const qreal scaleY = designRect.height() / designHeight;
    return QRect(qRound(designRect.left() + x * scaleX),
                 qRound(designRect.top() + y * scaleY),
                 qRound(width * scaleX),
                 qRound(height * scaleY));
}

QPushButton* createGhostButton(const QString& text, QWidget* parent)
{
    auto* button = new QPushButton(text, parent);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(QStringLiteral(
        "QPushButton{background:rgba(9,18,42,110);color:#F6FBFF;border:2px solid rgba(155,194,255,150);"
        "border-radius:22px;padding:0 22px;font:700 14pt 'Segoe UI';}"
        "QPushButton:hover{background:rgba(18,34,72,170);border-color:rgba(255,255,255,220);}"
        "QPushButton:pressed{background:rgba(11,21,48,210);}"));
    return button;
}

void setReferenceHotspotMode(QPushButton* button, bool enabled)
{
    if (!button) {
        return;
    }

    if (enabled) {
        button->setStyleSheet(QStringLiteral(
            "QPushButton{background:transparent;color:transparent;border:none;padding:0;}"
            "QPushButton:hover{background:transparent;border:none;}"
            "QPushButton:pressed{background:transparent;border:none;}"));
    } else {
        button->setStyleSheet(QStringLiteral(
            "QPushButton{background:rgba(9,18,42,110);color:#F6FBFF;border:2px solid rgba(155,194,255,150);"
            "border-radius:22px;padding:0 22px;font:700 14pt 'Segoe UI';}"
            "QPushButton:hover{background:rgba(18,34,72,170);border-color:rgba(255,255,255,220);}"
            "QPushButton:pressed{background:rgba(11,21,48,210);}"));
    }
}

} // namespace

namespace PhantomDrive {

MainMenuWidget::MainMenuWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_startGameCard = new MenuCardWidget(this);
    m_startGameCard->setTitle(QStringLiteral("START GAME"));
    m_startGameCard->setAccentColors(QColor(QStringLiteral("#FF5F9A")), QColor(QStringLiteral("#FF2B66")));
    m_startGameCard->setShapeStyle(MenuCardWidget::ShapeStyle::ReferenceBanner);
    m_startGameCard->setReferenceMode(true);
    connect(m_startGameCard, &QAbstractButton::clicked, this, &MainMenuWidget::showStartGameOverlay);

    m_garageCard = new MenuCardWidget(this);
    m_garageCard->setTitle(QStringLiteral("GARAGE"));
    m_garageCard->setAccentColors(QColor(QStringLiteral("#34D7FF")), QColor(QStringLiteral("#0075FF")));
    m_garageCard->setShapeStyle(MenuCardWidget::ShapeStyle::ReferenceBeveled);
    m_garageCard->setReferenceMode(true);
    connect(m_garageCard, &QAbstractButton::clicked, this, &MainMenuWidget::garageRequested);

    m_recordsGuideCard = new MenuCardWidget(this);
    m_recordsGuideCard->setTitle(QStringLiteral("RECORDS"));
    m_recordsGuideCard->setAccentColors(QColor(QStringLiteral("#3D8EFF")), QColor(QStringLiteral("#1447D4")));
    m_recordsGuideCard->setShapeStyle(MenuCardWidget::ShapeStyle::ReferenceBeveled);
    m_recordsGuideCard->setReferenceMode(true);
    connect(m_recordsGuideCard, &QAbstractButton::clicked, this, &MainMenuWidget::recordsRequested);

    m_trackStudioCard = new MenuCardWidget(this);
    m_trackStudioCard->setTitle(QStringLiteral("TRACK STUDIO"));
    m_trackStudioCard->setAccentColors(QColor(QStringLiteral("#A46BFF")), QColor(QStringLiteral("#5A2EFF")));
    m_trackStudioCard->setShapeStyle(MenuCardWidget::ShapeStyle::ReferenceBeveled);
    m_trackStudioCard->setReferenceMode(true);
    connect(m_trackStudioCard, &QAbstractButton::clicked, this, &MainMenuWidget::trackStudioRequested);

    m_guideButton = createGhostButton(QStringLiteral("GUIDE"), this);
    connect(m_guideButton, &QPushButton::clicked, this, &MainMenuWidget::guideRequested);

    m_exitButton = createGhostButton(QStringLiteral("EXIT"), this);
    connect(m_exitButton, &QPushButton::clicked, this, &MainMenuWidget::exitRequested);

    m_startGameOverlay = new StartGameOverlay(this);
    m_startGameOverlay->hide();
    connect(m_startGameOverlay, &StartGameOverlay::arcadeRequested, this, &MainMenuWidget::arcadeRequested);
    connect(m_startGameOverlay, &StartGameOverlay::coinChallengeRequested, this, &MainMenuWidget::coinChallengeRequested);
    connect(m_startGameOverlay, &StartGameOverlay::learningRequested, this, &MainMenuWidget::learningRequested);
    connect(m_startGameOverlay, &StartGameOverlay::aiDemoRequested, this, &MainMenuWidget::aiDemoRequested);
    connect(m_startGameOverlay, &StartGameOverlay::guideRequested, this, &MainMenuWidget::guideRequested);
    connect(m_startGameOverlay, &StartGameOverlay::backRequested, this, &MainMenuWidget::hideStartGameOverlay);

    m_overlayOpacityEffect = new QGraphicsOpacityEffect(m_startGameOverlay);
    m_overlayOpacityEffect->setOpacity(0.0);
    m_startGameOverlay->setGraphicsEffect(m_overlayOpacityEffect);

    m_overlayAnimation = new QPropertyAnimation(m_overlayOpacityEffect, "opacity", this);
    m_overlayAnimation->setDuration(220);
    m_overlayAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_overlayAnimation, &QPropertyAnimation::finished, this, [this]() {
        if (m_overlayOpacityEffect->opacity() <= 0.01) {
            m_startGameOverlay->hide();
        }
    });
}

void MainMenuWidget::setCoinCount(int coins)
{
    m_coinCount = qMax(0, coins);
    if (m_startGameOverlay) {
        m_startGameOverlay->setCoinCount(m_coinCount);
    }
    update();
}

void MainMenuWidget::setReferenceBackgrounds(const QString& mainPath, const QString& startPath)
{
    const QString resolvedMain = QFileInfo(mainPath).exists() ? mainPath : resolveAssetPath(mainPath);
    m_referenceBackground.load(resolvedMain);

    const bool hasReference = !m_referenceBackground.isNull();
    for (MenuCardWidget* card : { m_startGameCard, m_garageCard, m_trackStudioCard, m_recordsGuideCard }) {
        card->setReferenceMode(hasReference);
    }
    setReferenceHotspotMode(m_guideButton, hasReference);
    setReferenceHotspotMode(m_exitButton, hasReference);

    if (m_startGameOverlay) {
        m_startGameOverlay->setReferenceBackground(startPath);
        m_startGameOverlay->setCoinCount(m_coinCount);
    }
    relayout();
    update();
}

void MainMenuWidget::showPrimaryMenu()
{
    hideStartGameOverlay();
}

void MainMenuWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    relayout();
}

void MainMenuWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    relayout();
}

void MainMenuWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillRect(rect(), QColor(4, 8, 22));

    if (!m_referenceBackground.isNull()) {
        drawCoverPixmap(painter, rect(), m_referenceBackground);
        return;
    }

    QLinearGradient bg(rect().topLeft(), rect().bottomRight());
    bg.setColorAt(0.0, QColor(QStringLiteral("#04142A")));
    bg.setColorAt(0.55, QColor(QStringLiteral("#0F0C2D")));
    bg.setColorAt(1.0, QColor(QStringLiteral("#2A0C2B")));
    painter.fillRect(rect(), bg);

    painter.setPen(QColor(245, 248, 255));
    QFont logoFont(QStringLiteral("Segoe UI"), 28, QFont::Black, true);
    painter.setFont(logoFont);
    painter.drawText(QRectF(width() * 0.18, 48.0, width() * 0.44, 64.0), Qt::AlignCenter, QStringLiteral("PHANTOMDRIVE"));

    painter.setFont(QFont(QStringLiteral("Segoe UI"), 12, QFont::Bold));
    painter.drawText(QRectF(width() - 240.0, 38.0, 180.0, 34.0), Qt::AlignRight | Qt::AlignVCenter,
                     QStringLiteral("COINS  %1").arg(m_coinCount));
}

void MainMenuWidget::relayout()
{
    const qreal w = width();
    const qreal h = height();
    const bool compactLayout = w < 1180.0;
    const int outerMarginX = qMax(24, qRound(w * 0.035));
    const int outerMarginY = qMax(24, qRound(h * 0.045));
    const int interCardGap = qBound(14, qRound(h * 0.02), 28);
    const int buttonHeight = qBound(50, qRound(h * 0.07), 72);

    if (!m_referenceBackground.isNull()) {
        const QRectF designRect = coverPixmapRect(m_referenceBackground, rect());
        m_startGameCard->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 1034.0, 171.0, 565.0, 217.0));
        m_garageCard->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 1072.0, 398.0, 490.0, 128.0));
        m_recordsGuideCard->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 1068.0, 538.0, 494.0, 101.0));
        m_trackStudioCard->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 1068.0, 650.0, 496.0, 108.0));
        m_guideButton->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 18.0, 34.0, 270.0, 68.0));
        m_exitButton->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 34.0, 800.0, 126.0, 70.0));
    } else {
        const int railWidth = compactLayout
            ? qBound(360, qRound(w * 0.56), qRound(w - outerMarginX * 2.0))
            : qBound(360, qRound(w * 0.33), 620);
        const int railX = compactLayout
            ? qRound((w - railWidth) * 0.5)
            : qRound(w - railWidth - outerMarginX);
        const int startHeight = qBound(150, qRound(h * (compactLayout ? 0.20 : 0.24)), 250);
        const int secondaryHeight = qBound(82, qRound(h * 0.115), 142);
        const int top = compactLayout ? qRound(h * 0.19) : qRound(h * 0.18);

        m_startGameCard->setCompact(false);
        m_garageCard->setCompact(!compactLayout);
        m_recordsGuideCard->setCompact(!compactLayout);
        m_trackStudioCard->setCompact(!compactLayout);

        m_startGameCard->setGeometry(railX, top, railWidth, startHeight);
        m_garageCard->setGeometry(railX + qRound(railWidth * 0.05), top + startHeight + interCardGap,
                                  qRound(railWidth * 0.95), secondaryHeight);
        m_recordsGuideCard->setGeometry(railX + qRound(railWidth * 0.035),
                                        top + startHeight + interCardGap * 2 + secondaryHeight,
                                        qRound(railWidth * 0.965), secondaryHeight);
        m_trackStudioCard->setGeometry(railX + qRound(railWidth * 0.02),
                                       top + startHeight + interCardGap * 3 + secondaryHeight * 2,
                                       qRound(railWidth * 0.98), secondaryHeight);
        m_guideButton->setGeometry(outerMarginX, outerMarginY, qBound(150, qRound(w * 0.15), 220), buttonHeight);
        m_exitButton->setGeometry(outerMarginX, height() - outerMarginY - buttonHeight,
                                  qBound(110, qRound(w * 0.10), 148), buttonHeight);
    }

    if (m_startGameOverlay) {
        m_startGameOverlay->setGeometry(rect());
        m_startGameOverlay->raise();
    }
}

void MainMenuWidget::showStartGameOverlay()
{
    if (!m_startGameOverlay) {
        return;
    }
    m_startGameOverlay->setCoinCount(m_coinCount);
    m_startGameOverlay->setGeometry(rect());
    m_startGameOverlay->show();
    m_startGameOverlay->raise();
    m_overlayAnimation->stop();
    m_overlayAnimation->setStartValue(m_overlayOpacityEffect->opacity());
    m_overlayAnimation->setEndValue(1.0);
    m_overlayAnimation->start();
}

void MainMenuWidget::hideStartGameOverlay()
{
    if (!m_startGameOverlay || !m_startGameOverlay->isVisible()) {
        return;
    }
    m_overlayAnimation->stop();
    m_overlayAnimation->setStartValue(m_overlayOpacityEffect->opacity());
    m_overlayAnimation->setEndValue(0.0);
    m_overlayAnimation->start();
}

} // namespace PhantomDrive
