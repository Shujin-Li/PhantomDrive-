#pragma once

#include "AIOpponent.h"
#include "PhantomDrive_global.h"
#include "ScoreReport.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QVector2D>
#include <QJsonObject>

namespace PhantomDrive {

class AIOpponent;

/**
 * @brief AI 对手管理器
 *
 * 负责管理所有 AI 对手的生命周期，包括：
 * - 创建/销毁 AI 对手
 * - 更新所有 AI 状态
 * - 提供 AI 查询接口
 */
class PHANTOMDRIVE_EXPORT AIOpponentManager : public QObject
{
    Q_OBJECT

public:
    explicit AIOpponentManager(QObject* parent = nullptr);
    ~AIOpponentManager() override;

    // ==================== 生命周期管理 ====================
    virtual AIOpponent* createOpponent(const QString& id, AIStyle style = AIStyle::Normal);
    virtual void destroyOpponent(const QString& id);
    virtual void destroyAllOpponents();

    // ==================== AI 访问 ====================
    virtual AIOpponent* getOpponent(const QString& id) const;
    virtual QList<AIOpponent*> getAllOpponents() const;
    virtual int getOpponentCount() const;
    virtual bool hasOpponent(const QString& id) const;

    // ==================== 查询接口 ====================
    virtual QList<AIOpponent*> getOpponentsByStyle(AIStyle style) const;
    virtual AIOpponent* getNearestOpponent(const QVector2D& position) const;
    virtual QList<AIOpponent*> getOpponentsInRadius(const QVector2D& center, qreal radius) const;
    virtual int getOpponentPosition(const QString& opponentId) const;

    // ==================== 赛道设置 ====================
    virtual void setWaypointsForAll(const QList<Waypoint>& waypoints);
    virtual void setTrackBounds(const QRectF& bounds);

    // ==================== 批量更新 ====================
    virtual void update(qint64 elapsedMs);

    // ==================== 碰撞处理 ====================
    virtual void onPlayerCollision(const QString& opponentId, const QVector2D& point);

    // ==================== 道具交互 ====================
    virtual void notifyPowerupCollected(const QString& opponentId, int powerupType);
    virtual void notifyPowerupUsed(const QString& opponentId, int slot);

    // ==================== 序列化 ====================
    virtual QJsonObject toJson() const;
    virtual void fromJson(const QJsonObject& json);

    // ==================== Q-Learning ====================
    void onQLearningFeedbackReady(const QLearningFeedback& feedback);

signals:
    void opponentAdded(const QString& opponentId);
    void opponentRemoved(const QString& opponentId);
    void opponentStateChanged(const QString& opponentId, AIState oldState, AIState newState);
    void opponentFinished(const QString& opponentId, int finalPosition);
    void allOpponentsFinished();
    void rankingsUpdated(const QList<QString>& orderedOpponentIds);

private:
    QMap<QString, AIOpponent*> m_opponents;
    QList<QString> m_opponentOrder;
    QRectF m_trackBounds;
};

} // namespace PhantomDrive
