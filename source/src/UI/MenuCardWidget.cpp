#include "UI/MenuCardWidget.h"

#include <QEnterEvent>
#include <QEasingCurve>
#include <QFont>
#include <QFontMetricsF>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QRegion>
#include <QResizeEvent>

#include <iterator>

namespace {

using ShapeStyle = PhantomDrive::MenuCardWidget::ShapeStyle;

QPainterPath buildSampledProfilePath(const QRectF& bounds,
                                     const qreal* top,
                                     int topCount,
                                     const qreal* bottom,
                                     int bottomCount)
{
    if (bounds.isEmpty() || topCount < 2 || bottomCount < 2) {
        return QPainterPath();
    }

    QPainterPath path;
    for (int i = 0; i < topCount; ++i) {
        const qreal t = static_cast<qreal>(i) / static_cast<qreal>(topCount - 1);
        const QPointF pt(bounds.left() + bounds.width() * t,
                         bounds.top() + bounds.height() * top[i]);
        if (i == 0) {
            path.moveTo(pt);
        } else {
            path.lineTo(pt);
        }
    }

    for (int i = bottomCount - 1; i >= 0; --i) {
        const qreal t = static_cast<qreal>(i) / static_cast<qreal>(bottomCount - 1);
        path.lineTo(bounds.left() + bounds.width() * t,
                    bounds.top() + bounds.height() * bottom[i]);
    }

    path.closeSubpath();
    return path.simplified();
}

QPainterPath buildDefaultCardPath(const QRectF& bounds, bool compact)
{
    const qreal leadInset = qMin(bounds.width() * 0.09, compact ? 22.0 : 28.0);
    const qreal trailingCut = qMin(bounds.width() * (compact ? 0.18 : 0.14), compact ? 44.0 : 56.0);
    const qreal tailInset = trailingCut * 0.58;

    QPainterPath path;
    path.moveTo(bounds.left() + leadInset, bounds.top());
    path.lineTo(bounds.right() - trailingCut, bounds.top());
    path.lineTo(bounds.right(), bounds.center().y());
    path.lineTo(bounds.right() - tailInset, bounds.bottom());
    path.lineTo(bounds.left() + leadInset, bounds.bottom());
    path.lineTo(bounds.left(), bounds.center().y());
    path.closeSubpath();
    return path.simplified();
}

QPainterPath buildReferenceBeveledPath(const QRectF& bounds, bool compact)
{
    Q_UNUSED(compact);
    static const qreal kTop[] = {
        0.688, 0.218, 0.182, 0.159, 0.141, 0.124, 0.129, 0.076, 0.059, 0.047, 0.006
    };
    static const qreal kBottom[] = {
        0.718, 0.994, 0.818, 0.818, 0.824, 0.824, 0.818, 0.818, 0.812, 0.806, 0.924
    };
    return buildSampledProfilePath(bounds, kTop, std::size(kTop), kBottom, std::size(kBottom));
}

QPainterPath buildReferenceSubmenuBeveledPath(const QRectF& bounds, bool compact)
{
    Q_UNUSED(compact);
    static const qreal kTop[] = {
        0.871, 0.014, 0.034, 0.054, 0.082, 0.109, 0.136, 0.156, 0.190, 0.211, 0.748
    };
    static const qreal kBottom[] = {
        0.898, 0.980, 0.980, 0.980, 0.980, 0.980, 0.980, 0.986, 0.986, 0.986, 0.946
    };
    return buildSampledProfilePath(bounds, kTop, std::size(kTop), kBottom, std::size(kBottom));
}

QPainterPath buildReferenceTrapezoidPath(const QRectF& bounds, bool compact)
{
    Q_UNUSED(compact);
    static const qreal kTop[] = {
        0.656, 0.014, 0.042, 0.085, 0.137, 0.241, 0.269, 0.311, 0.358, 0.406, 0.906
    };
    static const qreal kBottom[] = {
        0.717, 0.835, 0.858, 0.877, 0.901, 0.910, 0.929, 0.953, 0.962, 0.976, 0.929
    };
    return buildSampledProfilePath(bounds, kTop, std::size(kTop), kBottom, std::size(kBottom));
}

QPainterPath buildReferenceBannerPath(const QRectF& bounds, bool compact)
{
    Q_UNUSED(compact);
    static const qreal kTop[] = {
        0.852, 0.363, 0.329, 0.295, 0.287, 0.236, 0.186, 0.160, 0.063, 0.021, 0.127
    };
    static const qreal kBottom[] = {
        0.852, 0.970, 0.949, 0.928, 0.911, 0.882, 0.869, 0.852, 0.827, 0.785, 0.139
    };
    return buildSampledProfilePath(bounds, kTop, std::size(kTop), kBottom, std::size(kBottom));
}

QPainterPath buildCardPath(const QRectF& bounds, ShapeStyle style, bool compact)
{
    switch (style) {
    case ShapeStyle::ReferenceBeveled:
        return buildReferenceBeveledPath(bounds, compact);
    case ShapeStyle::ReferenceSubmenuBeveled:
        return buildReferenceSubmenuBeveledPath(bounds, compact);
    case ShapeStyle::ReferenceTrapezoid:
        return buildReferenceTrapezoidPath(bounds, compact);
    case ShapeStyle::ReferenceBanner:
        return buildReferenceBannerPath(bounds, compact);
    case ShapeStyle::Default:
    default:
        return buildDefaultCardPath(bounds, compact);
    }
}

} // namespace

namespace PhantomDrive {

MenuCardWidget::MenuCardWidget(QWidget* parent)
    : QAbstractButton(parent)
    , m_accentStart(QColor(QStringLiteral("#FF4D9A")))
    , m_accentEnd(QColor(QStringLiteral("#4ED7FF")))
    , m_highlightAnimation(new QPropertyAnimation(this, "highlightProgress", this))
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_highlightAnimation->setDuration(180);
    m_highlightAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void MenuCardWidget::setTitle(const QString& title)
{
    m_title = title;
    update();
}

void MenuCardWidget::setSubtitle(const QString& subtitle)
{
    m_subtitle = subtitle;
    update();
}

void MenuCardWidget::setBadgeText(const QString& badgeText)
{
    m_badgeText = badgeText;
    update();
}

void MenuCardWidget::setAccentColors(const QColor& start, const QColor& end)
{
    m_accentStart = start;
    m_accentEnd = end;
    update();
}

void MenuCardWidget::setReferenceMode(bool enabled)
{
    m_referenceMode = enabled;
    update();
}

void MenuCardWidget::setCompact(bool compact)
{
    m_compact = compact;
    if (!rect().isEmpty()) {
        setMask(QRegion(cardHitPath().toFillPolygon().toPolygon()));
    }
    update();
}

void MenuCardWidget::setShapeStyle(ShapeStyle style)
{
    if (m_shapeStyle == style) {
        return;
    }

    m_shapeStyle = style;
    if (!rect().isEmpty()) {
        setMask(QRegion(cardHitPath().toFillPolygon().toPolygon()));
    }
    update();
}

qreal MenuCardWidget::highlightProgress() const
{
    return m_highlightProgress;
}

void MenuCardWidget::setHighlightProgress(qreal progress)
{
    const qreal clamped = qBound<qreal>(0.0, progress, 1.0);
    if (qFuzzyCompare(m_highlightProgress, clamped)) {
        return;
    }
    m_highlightProgress = clamped;
    update();
}

void MenuCardWidget::enterEvent(QEnterEvent* event)
{
    setHovered(hitButton(event->position().toPoint()));
    QAbstractButton::enterEvent(event);
}

void MenuCardWidget::leaveEvent(QEvent* event)
{
    setHovered(false);
    unsetCursor();
    QAbstractButton::leaveEvent(event);
}

void MenuCardWidget::mouseMoveEvent(QMouseEvent* event)
{
    const bool inside = hitButton(event->position().toPoint());
    setHovered(inside);
    setCursor(inside ? Qt::PointingHandCursor : Qt::ArrowCursor);
    QAbstractButton::mouseMoveEvent(event);
}

void MenuCardWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !hitButton(event->position().toPoint())) {
        event->ignore();
        return;
    }

    QAbstractButton::mousePressEvent(event);
    update();
}

void MenuCardWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QAbstractButton::mouseReleaseEvent(event);
    setHovered(hitButton(event->position().toPoint()));
    update();
}

void MenuCardWidget::resizeEvent(QResizeEvent* event)
{
    QAbstractButton::resizeEvent(event);
    setMask(QRegion(cardHitPath().toFillPolygon().toPolygon()));
}

void MenuCardWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if (m_referenceMode) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const QRectF bounds = rect().adjusted(3.0, 3.0, -3.0, -3.0);
    const qreal cut = qMin(bounds.width() * 0.12, 42.0);
    const QPainterPath cardPath = cardHitPath();
    const qreal pressedProgress = isDown() ? 1.0 : 0.0;

    const QColor borderStart = m_accentStart.lighter(115);
    const QColor borderEnd = m_accentEnd.lighter(105);
    const QColor fillStart = QColor(borderStart.red(), borderStart.green(), borderStart.blue(),
                                    m_referenceMode ? qRound(34 + m_highlightProgress * 40.0 + pressedProgress * 18.0)
                                                    : qRound(118 + m_highlightProgress * 34.0 + pressedProgress * 24.0));
    const QColor fillEnd = QColor(borderEnd.red(), borderEnd.green(), borderEnd.blue(),
                                  m_referenceMode ? qRound(18 + m_highlightProgress * 28.0 + pressedProgress * 12.0)
                                                  : qRound(72 + m_highlightProgress * 28.0 + pressedProgress * 16.0));

    if (m_highlightProgress > 0.01) {
        QLinearGradient glow(bounds.topLeft(), bounds.bottomRight());
        glow.setColorAt(0.0, QColor(borderStart.red(), borderStart.green(), borderStart.blue(),
                                    qRound(76 * m_highlightProgress)));
        glow.setColorAt(1.0, QColor(borderEnd.red(), borderEnd.green(), borderEnd.blue(),
                                    qRound(88 * m_highlightProgress)));
        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawPath(cardPath.simplified());
    }

    QLinearGradient fill(bounds.topLeft(), bounds.bottomRight());
    fill.setColorAt(0.0, fillStart);
    fill.setColorAt(1.0, fillEnd);
    painter.setBrush(fill);
    QLinearGradient border(bounds.topLeft(), bounds.bottomRight());
    border.setColorAt(0.0, QColor(borderStart.red(), borderStart.green(), borderStart.blue(),
                                  m_referenceMode ? qRound(145 + m_highlightProgress * 75.0 + pressedProgress * 24.0)
                                                  : qRound(228 + pressedProgress * 18.0)));
    border.setColorAt(1.0, QColor(borderEnd.red(), borderEnd.green(), borderEnd.blue(),
                                  m_referenceMode ? qRound(155 + m_highlightProgress * 70.0 + pressedProgress * 20.0)
                                                  : qRound(232 + pressedProgress * 18.0)));
    painter.setPen(QPen(QBrush(border), m_referenceMode ? (2.4 + pressedProgress * 0.6)
                                                        : (2.8 + pressedProgress * 0.6)));
    painter.drawPath(cardPath);

    painter.save();
    painter.setClipPath(cardPath);
    const QRectF highlightRect = bounds.adjusted(bounds.width() * 0.12, 10.0, -bounds.width() * 0.18, -10.0);
    painter.setBrush(QColor(255, 255, 255, qRound(18 + m_highlightProgress * 22.0 + pressedProgress * 16.0)));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(highlightRect, 22.0, 22.0);
    painter.restore();

    const qreal leftInset = m_compact ? 28.0 : 36.0;
    const QRectF contentRect = bounds.adjusted(leftInset, 16.0, -(cut + 26.0), -18.0);

    if (!m_badgeText.trimmed().isEmpty()) {
        painter.setFont(QFont(QStringLiteral("Segoe UI"), m_compact ? 10 : 11, QFont::DemiBold));
        painter.setPen(QColor(255, 236, 188, 220));
        painter.drawText(QRectF(contentRect.left(), contentRect.top(), contentRect.width(), 20.0),
                         Qt::AlignLeft | Qt::AlignTop,
                         m_badgeText);
    }

    QFont titleFont(QStringLiteral("Segoe UI"), m_compact ? 17 : 22, QFont::Black);
    titleFont.setItalic(true);
    painter.setFont(titleFont);
    painter.setPen(QColor(248, 252, 255));
    const QRectF titleRect(contentRect.left(),
                           contentRect.top() + (m_badgeText.trimmed().isEmpty() ? 8.0 : 26.0),
                           contentRect.width(),
                           m_compact ? 28.0 : 36.0);
    painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, m_title);

    if (!m_subtitle.trimmed().isEmpty()) {
        painter.setFont(QFont(QStringLiteral("Segoe UI"), m_compact ? 10 : 11, QFont::Bold));
        painter.setPen(QColor(222, 236, 255, 205));
        painter.drawText(QRectF(contentRect.left(),
                                titleRect.bottom() + 4.0,
                                contentRect.width(),
                                22.0),
                         Qt::AlignLeft | Qt::AlignTop,
                         m_subtitle.toUpper());
    }
}

void MenuCardWidget::animateHighlight(qreal target)
{
    m_highlightAnimation->stop();
    m_highlightAnimation->setStartValue(m_highlightProgress);
    m_highlightAnimation->setEndValue(target);
    m_highlightAnimation->start();
}

bool MenuCardWidget::hitButton(const QPoint& pos) const
{
    return cardHitPath().contains(QPointF(pos) + QPointF(0.5, 0.5));
}

QPainterPath MenuCardWidget::cardHitPath() const
{
    const QRectF bounds = rect().adjusted(3.0, 3.0, -3.0, -3.0);
    if (bounds.isEmpty()) {
        return QPainterPath();
    }
    return buildCardPath(bounds, m_shapeStyle, m_compact);
}

void MenuCardWidget::setHovered(bool hovered)
{
    if (m_hovered == hovered) {
        return;
    }

    m_hovered = hovered;
    animateHighlight(m_hovered ? 1.0 : 0.0);
}

} // namespace PhantomDrive
