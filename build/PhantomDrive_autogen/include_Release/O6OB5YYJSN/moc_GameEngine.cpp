/****************************************************************************
** Meta object code from reading C++ file 'GameEngine.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/PhantomDrive/core/GameEngine.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GameEngine.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN12PhantomDrive10GameEngineE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive10GameEngineE = QtMocHelpers::stringData(
    "PhantomDrive::GameEngine",
    "engineStarted",
    "",
    "engineStopped",
    "enginePaused",
    "paused",
    "gameModeChanged",
    "mode",
    "playerVehicleUpdated",
    "VehicleState",
    "state",
    "aiVehicleUpdated",
    "aiId",
    "vehicleRegistered",
    "vehicleId",
    "vehicleUnregistered",
    "trackLoaded",
    "trackId",
    "lapCompleted",
    "lapNumber",
    "lapTime",
    "checkpointReached",
    "checkpointId",
    "index",
    "raceFinished",
    "speedDataReady",
    "speed",
    "lapDataReady",
    "currentLap",
    "totalLaps",
    "modeDataReady",
    "speedLimitZoneEntered",
    "limit",
    "zoneId",
    "speedLimitZoneExited",
    "collisionOccurred",
    "objectId",
    "trackBoundsExceeded",
    "onAIDecision",
    "targetPosition",
    "targetSpeed",
    "onAIUpdateRequest",
    "onGameLoopTick",
    "onPlayerSensorDataReady",
    "DrivingData",
    "data",
    "onPlayerCollision",
    "position",
    "impactForce"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive10GameEngineE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      19,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  158,    2, 0x06,    1 /* Public */,
       3,    0,  159,    2, 0x06,    2 /* Public */,
       4,    1,  160,    2, 0x06,    3 /* Public */,
       6,    1,  163,    2, 0x06,    5 /* Public */,
       8,    1,  166,    2, 0x06,    7 /* Public */,
      11,    2,  169,    2, 0x06,    9 /* Public */,
      13,    1,  174,    2, 0x06,   12 /* Public */,
      15,    1,  177,    2, 0x06,   14 /* Public */,
      16,    1,  180,    2, 0x06,   16 /* Public */,
      18,    2,  183,    2, 0x06,   18 /* Public */,
      21,    2,  188,    2, 0x06,   21 /* Public */,
      24,    0,  193,    2, 0x06,   24 /* Public */,
      25,    1,  194,    2, 0x06,   25 /* Public */,
      27,    2,  197,    2, 0x06,   27 /* Public */,
      30,    1,  202,    2, 0x06,   30 /* Public */,
      31,    2,  205,    2, 0x06,   32 /* Public */,
      34,    0,  210,    2, 0x06,   35 /* Public */,
      35,    2,  211,    2, 0x06,   36 /* Public */,
      37,    1,  216,    2, 0x06,   39 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      38,    3,  219,    2, 0x0a,   41 /* Public */,
      41,    1,  226,    2, 0x0a,   45 /* Public */,
      42,    0,  229,    2, 0x08,   47 /* Private */,
      43,    1,  230,    2, 0x08,   48 /* Private */,
      46,    3,  233,    2, 0x08,   50 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 9,   12,   10,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void, QMetaType::QString,   17,
    QMetaType::Void, QMetaType::Int, QMetaType::QReal,   19,   20,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   22,   23,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QReal,   26,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   28,   29,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QReal, QMetaType::QString,   32,   33,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   14,   36,
    QMetaType::Void, QMetaType::QString,   14,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QVector2D, QMetaType::QReal,   12,   39,   40,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 44,   45,
    QMetaType::Void, QMetaType::QString, QMetaType::QVector2D, QMetaType::QReal,   36,   47,   48,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::GameEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive10GameEngineE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive10GameEngineE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive10GameEngineE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<GameEngine, std::true_type>,
        // method 'engineStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'engineStopped'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'enginePaused'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'gameModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'playerVehicleUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const VehicleState &, std::false_type>,
        // method 'aiVehicleUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const VehicleState &, std::false_type>,
        // method 'vehicleRegistered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'vehicleUnregistered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'trackLoaded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'lapCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'checkpointReached'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'raceFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'speedDataReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'lapDataReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'modeDataReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'speedLimitZoneEntered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'speedLimitZoneExited'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'collisionOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'trackBoundsExceeded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onAIDecision'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'onAIUpdateRequest'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onGameLoopTick'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPlayerSensorDataReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DrivingData &, std::false_type>,
        // method 'onPlayerCollision'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::GameEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GameEngine *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->engineStarted(); break;
        case 1: _t->engineStopped(); break;
        case 2: _t->enginePaused((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->gameModeChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->playerVehicleUpdated((*reinterpret_cast< std::add_pointer_t<VehicleState>>(_a[1]))); break;
        case 5: _t->aiVehicleUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<VehicleState>>(_a[2]))); break;
        case 6: _t->vehicleRegistered((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->vehicleUnregistered((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->trackLoaded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->lapCompleted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 10: _t->checkpointReached((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 11: _t->raceFinished(); break;
        case 12: _t->speedDataReady((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 13: _t->lapDataReady((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 14: _t->modeDataReady((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->speedLimitZoneEntered((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 16: _t->speedLimitZoneExited(); break;
        case 17: _t->collisionOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 18: _t->trackBoundsExceeded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 19: _t->onAIDecision((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 20: _t->onAIUpdateRequest((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->onGameLoopTick(); break;
        case 22: _t->onPlayerSensorDataReady((*reinterpret_cast< std::add_pointer_t<DrivingData>>(_a[1]))); break;
        case 23: _t->onPlayerCollision((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (GameEngine::*)();
            if (_q_method_type _q_method = &GameEngine::engineStarted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)();
            if (_q_method_type _q_method = &GameEngine::engineStopped; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(bool );
            if (_q_method_type _q_method = &GameEngine::enginePaused; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(const QString & );
            if (_q_method_type _q_method = &GameEngine::gameModeChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(const VehicleState & );
            if (_q_method_type _q_method = &GameEngine::playerVehicleUpdated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(const QString & , const VehicleState & );
            if (_q_method_type _q_method = &GameEngine::aiVehicleUpdated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(const QString & );
            if (_q_method_type _q_method = &GameEngine::vehicleRegistered; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(const QString & );
            if (_q_method_type _q_method = &GameEngine::vehicleUnregistered; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(const QString & );
            if (_q_method_type _q_method = &GameEngine::trackLoaded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(int , qreal );
            if (_q_method_type _q_method = &GameEngine::lapCompleted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(int , int );
            if (_q_method_type _q_method = &GameEngine::checkpointReached; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)();
            if (_q_method_type _q_method = &GameEngine::raceFinished; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(qreal );
            if (_q_method_type _q_method = &GameEngine::speedDataReady; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(int , int );
            if (_q_method_type _q_method = &GameEngine::lapDataReady; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(const QString & );
            if (_q_method_type _q_method = &GameEngine::modeDataReady; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(qreal , const QString & );
            if (_q_method_type _q_method = &GameEngine::speedLimitZoneEntered; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 15;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)();
            if (_q_method_type _q_method = &GameEngine::speedLimitZoneExited; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 16;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(const QString & , const QString & );
            if (_q_method_type _q_method = &GameEngine::collisionOccurred; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 17;
                return;
            }
        }
        {
            using _q_method_type = void (GameEngine::*)(const QString & );
            if (_q_method_type _q_method = &GameEngine::trackBoundsExceeded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 18;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::GameEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::GameEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive10GameEngineE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::GameEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 24;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::GameEngine::engineStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PhantomDrive::GameEngine::engineStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PhantomDrive::GameEngine::enginePaused(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PhantomDrive::GameEngine::gameModeChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PhantomDrive::GameEngine::playerVehicleUpdated(const VehicleState & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PhantomDrive::GameEngine::aiVehicleUpdated(const QString & _t1, const VehicleState & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void PhantomDrive::GameEngine::vehicleRegistered(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PhantomDrive::GameEngine::vehicleUnregistered(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void PhantomDrive::GameEngine::trackLoaded(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void PhantomDrive::GameEngine::lapCompleted(int _t1, qreal _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void PhantomDrive::GameEngine::checkpointReached(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void PhantomDrive::GameEngine::raceFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void PhantomDrive::GameEngine::speedDataReady(qreal _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void PhantomDrive::GameEngine::lapDataReady(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void PhantomDrive::GameEngine::modeDataReady(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void PhantomDrive::GameEngine::speedLimitZoneEntered(qreal _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void PhantomDrive::GameEngine::speedLimitZoneExited()
{
    QMetaObject::activate(this, &staticMetaObject, 16, nullptr);
}

// SIGNAL 17
void PhantomDrive::GameEngine::collisionOccurred(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void PhantomDrive::GameEngine::trackBoundsExceeded(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}
QT_WARNING_POP
