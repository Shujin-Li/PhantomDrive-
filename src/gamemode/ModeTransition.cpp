#include "ModeTransition.h"

#include <QDebug>

namespace PhantomDrive {

ModeTransition::ModeTransition(QObject* parent)
    : QObject(parent)
    , m_animation(new QVariantAnimation(this))
    , m_transitionType(TransitionType::Fade)
    , m_progress(0.0)
{
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setDuration(500);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);

    connect(m_animation, &QVariantAnimation::valueChanged,
            this, &ModeTransition::onAnimationValueChanged);
    connect(m_animation, &QVariantAnimation::finished,
            this, &ModeTransition::onAnimationFinished);
}

ModeTransition::~ModeTransition()
{
    stop();
}

void ModeTransition::setDuration(int durationMs)
{
    m_animation->setDuration(durationMs);
}

int ModeTransition::getDuration() const
{
    return m_animation->duration();
}

void ModeTransition::setEasingCurve(QEasingCurve::Type curveType)
{
    m_animation->setEasingCurve(curveType);
}

QEasingCurve::Type ModeTransition::getEasingCurve() const
{
    return m_animation->easingCurve().type();
}

void ModeTransition::setTransitionType(TransitionType type)
{
    m_transitionType = type;
}

TransitionType ModeTransition::getTransitionType() const
{
    return m_transitionType;
}

qreal ModeTransition::getProgress() const
{
    return m_progress;
}

bool ModeTransition::isRunning() const
{
    return m_animation->state() == QVariantAnimation::Running;
}

void ModeTransition::start()
{
    if (m_animation->state() != QVariantAnimation::Running) {
        m_animation->start();
        emit started();
    }
}

void ModeTransition::stop()
{
    if (m_animation->state() == QVariantAnimation::Running) {
        m_animation->stop();
        emit cancelled();
    }
}

void ModeTransition::finish()
{
    m_progress = 1.0;
    emit progressChanged(1.0);
    emit finished();
}

void ModeTransition::paint(QPainter* painter, const QRect& rect, qreal progress) const
{
    switch (m_transitionType) {
        case TransitionType::Fade:
            applyFadeEffect(painter, rect, progress);
            break;
        case TransitionType::SlideLeft:
        case TransitionType::SlideRight:
        case TransitionType::SlideUp:
        case TransitionType::SlideDown:
            applySlideEffect(painter, rect, progress);
            break;
        case TransitionType::Zoom:
            applyZoomEffect(painter, rect, progress);
            break;
        case TransitionType::CrossFade:
            applyCrossFadeEffect(painter, rect, progress);
            break;
    }
}

void ModeTransition::applyFadeEffect(QPainter* painter, const QRect& rect, qreal progress) const
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(progress)
}

void ModeTransition::applySlideEffect(QPainter* painter, const QRect& rect, qreal progress) const
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(progress)
}

void ModeTransition::applyZoomEffect(QPainter* painter, const QRect& rect, qreal progress) const
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(progress)
}

void ModeTransition::applyCrossFadeEffect(QPainter* painter, const QRect& rect, qreal progress) const
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(progress)
}

void ModeTransition::onAnimationValueChanged(const QVariant& value)
{
    m_progress = value.toReal();
    emit progressChanged(m_progress);
}

void ModeTransition::onAnimationFinished()
{
    m_progress = 1.0;
    emit finished();
}

}
