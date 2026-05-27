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
    if (m_world) {
        delete m_world;
        m_world = nullptr;
    }
}

void MCPhysicsWorld::initialize(int width, int height)
{
    if (m_world) {
        return;
    }

    m_world = &MCWorld::instance();
    m_world->setDimensions(-width/2, width/2, -height/2, height/2, -100, 100, 1.0f, true, 128);

    emit worldInitialized();
}

void MCPhysicsWorld::start()
{
    if (m_world) {
        m_isRunning = true;
        emit worldStarted();
    }
}

void MCPhysicsWorld::stop()
{
    if (m_world) {
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
        m_world->stepTime(static_cast<int>(deltaTime));
        emit stepCompleted(deltaTime);
    }
}

MCObject* MCPhysicsWorld::addObject(MCObjectData* data)
{
    if (m_world && data) {
        MCObject* obj = new MCObject(data->typeId());
        m_world->addObject(*obj);
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
        m_world->removeObject(*obj);
        emit objectRemoved(obj);
    }
}

MCObject* MCPhysicsWorld::findObject(const QString& id) const
{
    if (m_world) {
        const auto& objects = m_world->objects();
        for (auto* obj : objects) {
            if (obj && obj->typeName() == id.toStdString()) {
                return obj;
            }
        }
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
    Q_UNUSED(enable);
}

bool MCPhysicsWorld::isCollisionDetectionEnabled() const
{
    return true;
}

void MCPhysicsWorld::setBounds(float x1, float y1, float x2, float y2)
{
    if (m_world) {
        m_world->setDimensions(x1, x2, y1, y2, -100, 100, 1.0f, true, 128);
    }
}

}
