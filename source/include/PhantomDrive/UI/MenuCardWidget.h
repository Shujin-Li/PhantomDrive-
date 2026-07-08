#pragma once

#include "PhantomDrive_global.h"

#include <QAbstractButton>
#include <QColor>
#include <QPainterPath>
#include <QObject>
#include <QString>

class QPropertyAnimation;
class QEnterEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT MenuCardWidget : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(qreal highlightProgress READ highlightProgress WRITE setHighlightProgress)

public:
    enum class ShapeStyle {
        Default,
        ReferenceBeveled,
        ReferenceSubmenuBeveled,
        ReferenceTrapezoid,
        ReferenceBanner
    };

    explicit MenuCardWidget(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setSubtitle(const QString& subtitle);
    void setBadgeText(const QString& badgeText);
    void setAccentColors(const QColor& start, const QColor& end);
    void setReferenceMode(bool enabled);
    void setCompact(bool compact);
    void setShapeStyle(ShapeStyle style);

    qreal highlightProgress() const;
    void setHighlightProgress(qreal progress);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool hitButton(const QPoint& pos) const override;

private:
    QPainterPath cardHitPath() const;
    void animateHighlight(qreal target);
    void setHovered(bool hovered);

    QString m_title;
    QString m_subtitle;
    QString m_badgeText;
    QColor m_accentStart;
    QColor m_accentEnd;
    ShapeStyle m_shapeStyle = ShapeStyle::Default;
    bool m_referenceMode = false;
    bool m_compact = false;
    bool m_hovered = false;
    qreal m_highlightProgress = 0.0;
    QPropertyAnimation* m_highlightAnimation = nullptr;
};

} // namespace PhantomDrive
