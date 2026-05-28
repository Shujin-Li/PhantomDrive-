#pragma once

#include "TrafficObject.h"
#include "PhantomDrive/gamemode/DrivingData.h"

#include <QString>
#include <QVector2D>
#include <QRectF>

namespace PhantomDrive {

class TrafficLightObject : public TrafficObject
{
    Q_OBJECT

public:
    enum class LightColor {
        Red = 0,
        Yellow = 1,
        Green = 2,
        Count
    };

    explicit TrafficLightObject(const QString& id, QObject* parent = nullptr);
    ~TrafficLightObject() override;

    LightColor getCurrentColor() const { return m_currentColor; }
    void setCurrentColor(LightColor color);

    int getRedDurationMs() const { return m_redDurationMs; }
    void setRedDurationMs(int durationMs);

    int getYellowDurationMs() const { return m_yellowDurationMs; }
    void setYellowDurationMs(int durationMs);

    int getGreenDurationMs() const { return m_greenDurationMs; }
    void setGreenDurationMs(int durationMs);

    bool isRed() const { return m_currentColor == LightColor::Red; }
    bool isYellow() const { return m_currentColor == LightColor::Yellow; }
    bool isGreen() const { return m_currentColor == LightColor::Green; }

    qint64 getTimeSinceLastChange() const;
    int getRemainingTimeInCurrentState() const;

    void start();
    void stop();
    bool isRunning() const { return m_isRunning; }

    void update(qint64 elapsedMs) override;
    bool checkRedLightViolation(const QVector2D& vehiclePosition) const;

    int getTotalCycleDurationMs() const;
    int getViolationPenalty() const { return m_violationPenalty; }
    void setViolationPenalty(int penalty) { m_violationPenalty = penalty; }

    int getRedLightViolationCount() const { return m_redLightViolationCount; }
    void markViolation();
    void resetViolationCount() override { m_redLightViolationCount = 0; }

signals:
    void colorChanged(TrafficLightObject::LightColor oldColor, TrafficLightObject::LightColor newColor);
    void redLightViolation(const QString& lightId);
    void cycleCompleted(int cycleCount);
    void timerExpired();

public slots:
    void onVehicleApproaching(const QVector2D& vehiclePosition);
    void reset() override;

private:
    void advanceToNextColor();

    LightColor m_currentColor;
    LightColor m_previousColor;

    int m_redDurationMs;
    int m_yellowDurationMs;
    int m_greenDurationMs;

    qint64 m_lastChangeTime;
    bool m_isRunning;
    bool m_wasRedWhenApproaching;

    int m_violationPenalty;
    int m_redLightViolationCount;
    int m_cycleCount;

    qint64 m_sessionStartTime;
};

}
