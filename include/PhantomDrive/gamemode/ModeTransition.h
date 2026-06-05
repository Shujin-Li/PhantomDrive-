#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QVariantAnimation>
#include <QEasingCurve>
#include <QPainter>
#include <QWidget>

namespace PhantomDrive {

enum class TransitionType {
    Fade,
    SlideLeft,
    SlideRight,
    SlideUp,
    SlideDown,
    Zoom,
    CrossFade
};

class ModeTransition : public QObject
{
    Q_OBJECT

public:
    explicit ModeTransition(QObject* parent = nullptr);
    ~ModeTransition();

    void setDuration(int durationMs);
    int getDuration() const;

    void setEasingCurve(QEasingCurve::Type curveType);
    QEasingCurve::Type getEasingCurve() const;

    void setTransitionType(TransitionType type);
    TransitionType getTransitionType() const;

    qreal getProgress() const;
    bool isRunning() const;

public slots:
    void start();
    void stop();
    void finish();

signals:
    void started();
    void progressChanged(qreal progress);
    void finished();
    void cancelled();

protected:
    virtual void paint(QPainter* painter, const QRect& rect, qreal progress) const;

private slots:
    void onAnimationValueChanged(const QVariant& value);
    void onAnimationFinished();

private:
    void applySlideEffect(QPainter* painter, const QRect& rect, qreal progress) const;
    void applyFadeEffect(QPainter* painter, const QRect& rect, qreal progress) const;
    void applyZoomEffect(QPainter* painter, const QRect& rect, qreal progress) const;
    void applyCrossFadeEffect(QPainter* painter, const QRect& rect, qreal progress) const;

    QVariantAnimation* m_animation;
    TransitionType m_transitionType;
    QEasingCurve m_easingCurve;
    qreal m_progress;
};

}
