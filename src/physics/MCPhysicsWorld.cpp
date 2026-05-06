#include "MCPhysicsWorld.h"

namespace PhantomDrive {

MCPhysicsWorld::MCPhysicsWorld(QObject* parent)
    : QObject(parent)
    , m_world(nullptr)
    , m_isRunning(false)
{
}

MCPhysicsWorld::~MCPhysicsWorld()
{
    stop();
}

void MCPhysicsWorld::initialize(int width, int height)
{
    if (m_world) {
        return;
    }

    m_world = new MCWorld();
    m_world->setBounds(MCBBoxF(0, 0, width, height));
    m_world->initialize();

    emit worldInitialized();
}

void MCPhysicsWorld::start()
{
    if (m_world) {
        m_world->start();
        m_isRunning = true;
        emit worldStarted();
    }
}

void MCPhysicsWorld::stop()
{
    if (m_world) {
        m_world->stop();
        m_isRunning = false;
        emit worldStopped();
    }
}

bool MCPhysicsWorld::isRunning() const
{
    return m_isRunning;
}

void MCPhysicsWorld::step(float deltaTime)
{
    if (m_world && m_isRunning) {
        m_world->stepTime(deltaTime);
        emit stepCompleted(deltaTime);
    }
}

MCObject* MCPhysicsWorld::addObject(MCObjectData* data)
{
    if (m_world && data) {
        MCObject* obj = m_world->addObject(data);
        if (obj) {
            emit objectAdded(obj);
        }
        return obj;
    }
    return nullptr;
}

void MCPhysicsWorld::removeObject(MCObject* obj)
{
    if (m_world && obj) {
        m_world->removeObject(obj);
        emit objectRemoved(obj);
    }
}

MCObject* MCPhysicsWorld::findObject(const QString& id) const
{
    if (m_world) {
        return m_world->findObject(id.toStdString());
    }
    return nullptr;
}

QList<MCObject*> MCPhysicsWorld::getAllObjects() const
{
    QList<MCObject*> objects;
    if (m_world) {
        const auto& allObjects = m_world->objects();
        for (auto* obj : allObjects) {
            objects.append(obj);
        }
    }
    return objects;
}

void MCPhysicsWorld::setGravity(float x, float y)
{
    if (m_world) {
        m_world->setGravity(MCVector3dF(x, y, 0));
    }
}

QVector2D MCPhysicsWorld::gravity() const
{
    if (m_world) {
        const auto& g = m_world->gravity();
        return QVector2D(g.i(), g.j());
    }
    return QVector2D();
}

void MCPhysicsWorld::enableCollisionDetection(bool enable)
{
    if (m_world) {
        m_world->setCollidingModeEnabled(enable);
    }
}

bool MCPhysicsWorld::isCollisionDetectionEnabled() const
{
    if (m_world) {
        return m_world->isCollidingModeEnabled();
    }
    return false;
}

void MCPhysicsWorld::setBounds(float x1, float y1, float x2, float y2)
{
    if (m_world) {
        m_world->setBounds(MCBBoxF(x1, y1, x2, y2));
    }
}

}
