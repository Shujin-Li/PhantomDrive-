#pragma once

#include "PhantomDrive_global.h"
#include "physics/Physics.h"

#include <QObject>
#include <QVector2D>
#include <QList>

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT MCPhysicsWorld : public QObject
{
    Q_OBJECT

public:
    explicit MCPhysicsWorld(QObject* parent = nullptr);
    ~MCPhysicsWorld() override;

    // 物理世界控制
    void initialize(int width, int height);
    void start();
    void stop();
    bool isRunning() const;

    // 物理步进
    void step(float deltaTime);

    // 对象管理
    MCObject* addObject(MCObjectData* data);
    void removeObject(MCObject* obj);
    MCObject* findObject(const QString& id) const;
    QList<MCObject*> getAllObjects() const;

    // 重力设置
    void setGravity(float x, float y);
    QVector2D gravity() const;

    // 碰撞检测
    void enableCollisionDetection(bool enable);
    bool isCollisionDetectionEnabled() const;

    // 物理边界
    void setBounds(float x1, float y1, float x2, float y2);

    // 访问底层 MCWorld
    MCWorld* mcWorld() const { return m_world; }

signals:
    void worldInitialized();
    void worldStarted();
    void worldStopped();
    void stepCompleted(float deltaTime);
    void objectAdded(MCObject* obj);
    void objectRemoved(MCObject* obj);
    void collisionOccurred(MCCollisionEvent* event);

private:
    MCWorld* m_world;
    bool m_isRunning;
};

}
