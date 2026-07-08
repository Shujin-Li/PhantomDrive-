#include "UI/StartGameOverlay.h"

#include "UI/MenuCardWidget.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QLinearGradient>
#include <QPainter>
#include <QPushButton>
#include <QRadialGradient>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStringList>

namespace {

constexpr qreal kReferenceDesignWidth = 1672.0;
constexpr qreal kReferenceDesignHeight = 941.0;
constexpr qreal kArcadeReferenceX = 34.0;
constexpr qreal kArcadeReferenceY = 172.0;
constexpr qreal kArcadeReferenceWidth = 566.0;
constexpr qreal kArcadeReferenceHeight = 208.0;

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
        "QPushButton{background:rgba(7,18,40,108);color:#F6FBFF;border:2px solid rgba(155,194,255,150);"
        "border-radius:22px;padding:0 24px;font:700 14pt 'Segoe UI';}"
        "QPushButton:hover{background:rgba(18,34,72,165);border-color:rgba(255,255,255,215);}"
        "QPushButton:pressed{background:rgba(12,24,56,205);}"));
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
            "QPushButton{background:rgba(7,18,40,108);color:#F6FBFF;border:2px solid rgba(155,194,255,150);"
            "border-radius:22px;padding:0 24px;font:700 14pt 'Segoe UI';}"
            "QPushButton:hover{background:rgba(18,34,72,165);border-color:rgba(255,255,255,215);}"
            "QPushButton:pressed{background:rgba(12,24,56,205);}"));
    }
}

} // namespace

namespace PhantomDrive {

StartGameOverlay::StartGameOverlay(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_arcadeCard = new MenuCardWidget(this);
    m_arcadeCard->setTitle(QStringLiteral("ARCADE"));
    m_arcadeCard->setAccentColors(QColor(QStringLiteral("#FF4A8E")), QColor(QStringLiteral("#FF94C8")));
    m_arcadeCard->setShapeStyle(MenuCardWidget::ShapeStyle::ReferenceTrapezoid);
    m_arcadeCard->setReferenceMode(true);
    connect(m_arcadeCard, &QAbstractButton::clicked, this, &StartGameOverlay::arcadeRequested);

    m_coinChallengeCard = new MenuCardWidget(this);
    m_coinChallengeCard->setTitle(QStringLiteral("COIN CHALLENGE"));
    m_coinChallengeCard->setAccentColors(QColor(QStringLiteral("#FFBF3A")), QColor(QStringLiteral("#FF7C35")));
    m_coinChallengeCard->setShapeStyle(MenuCardWidget::ShapeStyle::ReferenceSubmenuBeveled);
    m_coinChallengeCard->setReferenceMode(true);
    connect(m_coinChallengeCard, &QAbstractButton::clicked, this, &StartGameOverlay::coinChallengeRequested);

    m_learningCard = new MenuCardWidget(this);
    m_learningCard->setTitle(QStringLiteral("LEARNING"));
    m_learningCard->setAccentColors(QColor(QStringLiteral("#31D5FF")), QColor(QStringLiteral("#006EFF")));
    m_learningCard->setShapeStyle(MenuCardWidget::ShapeStyle::ReferenceSubmenuBeveled);
    m_learningCard->setReferenceMode(true);
    connect(m_learningCard, &QAbstractButton::clicked, this, &StartGameOverlay::learningRequested);

    m_aiDemoCard = new MenuCardWidget(this);
    m_aiDemoCard->setTitle(QStringLiteral("TWO-PLAYER / AI DEMO"));
    m_aiDemoCard->setAccentColors(QColor(QStringLiteral("#8E6BFF")), QColor(QStringLiteral("#5A3DFF")));
    m_aiDemoCard->setShapeStyle(MenuCardWidget::ShapeStyle::ReferenceSubmenuBeveled);
    m_aiDemoCard->setReferenceMode(true);
    connect(m_aiDemoCard, &QAbstractButton::clicked, this, &StartGameOverlay::aiDemoRequested);

    m_backButton = createGhostButton(QStringLiteral("BACK"), this);
    connect(m_backButton, &QPushButton::clicked, this, &StartGameOverlay::backRequested);

    m_guideButton = createGhostButton(QStringLiteral("GUIDE"), this);
    connect(m_guideButton, &QPushButton::clicked, this, &StartGameOverlay::guideRequested);
}

void StartGameOverlay::setCoinCount(int coins)
{
    m_coinCount = qMax(0, coins);
    update();
}

void StartGameOverlay::setReferenceBackground(const QString& path)
{
    const QString resolved = QFileInfo(path).exists() ? path : resolveAssetPath(path);
    m_referenceBackground.load(resolved);
    const bool hasReference = !m_referenceBackground.isNull();
    for (MenuCardWidget* card : { m_arcadeCard, m_coinChallengeCard, m_learningCard, m_aiDemoCard }) {
        card->setReferenceMode(hasReference);
    }
    setReferenceHotspotMode(m_backButton, hasReference);
    setReferenceHotspotMode(m_guideButton, hasReference);
    relayout();
    update();
}

bool StartGameOverlay::hasReferenceBackground() const
{
    return !m_referenceBackground.isNull();
}

void StartGameOverlay::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    relayout();
}

void StartGameOverlay::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    relayout();
}

void StartGameOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillRect(rect(), QColor(6, 10, 26));

    if (!m_referenceBackground.isNull()) {
        drawCoverPixmap(painter, rect(), m_referenceBackground);
        return;
    }

    QLinearGradient bg(rect().topLeft(), rect().bottomRight());
    bg.setColorAt(0.0, QColor(QStringLiteral("#071330")));
    bg.setColorAt(0.5, QColor(QStringLiteral("#120C34")));
    bg.setColorAt(1.0, QColor(QStringLiteral("#1B0D28")));
    painter.fillRect(rect(), bg);

    QRadialGradient glow(QPointF(width() * 0.68, height() * 0.60), width() * 0.45);
    glow.setColorAt(0.0, QColor(255, 76, 186, 130));
    glow.setColorAt(1.0, QColor(255, 76, 186, 0));
    painter.fillRect(rect(), glow);

    painter.setPen(QColor(242, 248, 255));
    painter.setFont(QFont(QStringLiteral("Segoe UI"), 26, QFont::Black, true));
    painter.drawText(QRectF(width() * 0.34, 34.0, width() * 0.32, 54.0), Qt::AlignCenter, QStringLiteral("PHANTOMDRIVE"));

    painter.setFont(QFont(QStringLiteral("Segoe UI"), 13, QFont::Bold));
    painter.drawText(QRectF(38.0, 26.0, 220.0, 36.0), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("COINS  %1").arg(m_coinCount));
}

void StartGameOverlay::relayout()
{
    const qreal w = width();
    const qreal h = height();
    const bool compactLayout = w < 1160.0;
    const int outerMarginX = qMax(26, qRound(w * 0.035));
    const int outerMarginY = qMax(24, qRound(h * 0.045));
    const int interCardGap = qBound(14, qRound(h * 0.02), 28);
    const int buttonHeight = qBound(48, qRound(h * 0.065), 70);

    if (hasReferenceBackground()) {
        const QRectF designRect = coverPixmapRect(m_referenceBackground, rect());
        m_arcadeCard->setGeometry(mapReferenceRect(designRect,
                                                   kReferenceDesignWidth,
                                                   kReferenceDesignHeight,
                                                   kArcadeReferenceX,
                                                   kArcadeReferenceY,
                                                   kArcadeReferenceWidth,
                                                   kArcadeReferenceHeight));
        m_coinChallengeCard->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 50.0, 360.0, 520.0, 164.0));
        m_learningCard->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 38.0, 540.0, 532.0, 116.0));
        m_aiDemoCard->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 30.0, 672.0, 540.0, 190.0));
        m_backButton->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 1225.0, 42.0, 180.0, 68.0));
        m_guideButton->setGeometry(mapReferenceRect(designRect, kReferenceDesignWidth, kReferenceDesignHeight, 1444.0, 42.0, 170.0, 68.0));
        return;
    }

    const int railWidth = compactLayout
        ? qBound(360, qRound(w * 0.56), qRound(w - outerMarginX * 2.0))
        : qBound(360, qRound(w * 0.34), 610);
    const int railX = compactLayout ? qRound((w - railWidth) * 0.5) : outerMarginX;
    const int topInset = compactLayout ? qRound(h * 0.19) : qRound(h * 0.18);
    const int heroHeight = qBound(120, qRound(h * 0.16), 190);
    const int compactHeight = qBound(88, qRound(h * 0.12), 140);

    m_arcadeCard->setCompact(false);
    m_coinChallengeCard->setCompact(false);
    m_learningCard->setCompact(true);
    m_aiDemoCard->setCompact(true);

    m_arcadeCard->setGeometry(railX, topInset, railWidth, heroHeight);
    m_coinChallengeCard->setGeometry(railX, topInset + heroHeight + interCardGap, railWidth, heroHeight);
    m_learningCard->setGeometry(railX + qRound(railWidth * 0.015),
                                topInset + (heroHeight + interCardGap) * 2,
                                qRound(railWidth * 0.985), compactHeight);
    m_aiDemoCard->setGeometry(railX + qRound(railWidth * 0.025),
                              topInset + (heroHeight + interCardGap) * 2 + compactHeight + interCardGap,
                              qRound(railWidth * 0.975), compactHeight);
    m_backButton->setGeometry(width() - outerMarginX - qBound(120, qRound(w * 0.11), 150) * 2 - 16,
                              outerMarginY,
                              qBound(120, qRound(w * 0.11), 150),
                              buttonHeight);
    m_guideButton->setGeometry(width() - outerMarginX - qBound(116, qRound(w * 0.10), 140),
                               outerMarginY,
                               qBound(116, qRound(w * 0.10), 140),
                               buttonHeight);
}

} // namespace PhantomDrive
