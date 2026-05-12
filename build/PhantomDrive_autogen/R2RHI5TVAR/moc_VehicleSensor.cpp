/****************************************************************************
** Meta object code from reading C++ file 'VehicleSensor.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/PhantomDrive/gamemode/VehicleSensor.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VehicleSensor.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive13VehicleSensorE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive13VehicleSensorE = QtMocHelpers::stringData(
    "PhantomDrive::VehicleSensor",
    "sensorDataReady",
    "",
    "DrivingData",
    "data",
    "speedLimitExceeded",
    "currentSpeed",
    "limit",
    "sensorStarted",
    "sensorStopped",
    "errorOccurred",
    "error",
    "updatePosition",
    "position",
    "updateVelocity",
    "velocity",
    "updateRotation",
    "rotation",
    "updateSteeringAngle",
    "steeringAngle",
    "updateAcceleration",
    "acceleration",
    "updateSpeedLimit",
    "zoneId",
    "updateBrakeState",
    "isBraking",
    "updateAcceleratorState",
    "isAccelerating",
    "updateHonkState",
    "isHonking",
    "onSamplingTimer"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive13VehicleSensorE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  110,    2, 0x06,    1 /* Public */,
       5,    2,  113,    2, 0x06,    3 /* Public */,
       8,    0,  118,    2, 0x06,    6 /* Public */,
       9,    0,  119,    2, 0x06,    7 /* Public */,
      10,    1,  120,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      12,    1,  123,    2, 0x0a,   10 /* Public */,
      14,    1,  126,    2, 0x0a,   12 /* Public */,
      16,    1,  129,    2, 0x0a,   14 /* Public */,
      18,    1,  132,    2, 0x0a,   16 /* Public */,
      20,    1,  135,    2, 0x0a,   18 /* Public */,
      22,    2,  138,    2, 0x0a,   20 /* Public */,
      22,    1,  143,    2, 0x2a,   23 /* Public | MethodCloned */,
      24,    1,  146,    2, 0x0a,   25 /* Public */,
      26,    1,  149,    2, 0x0a,   27 /* Public */,
      28,    1,  152,    2, 0x0a,   29 /* Public */,
      30,    0,  155,    2, 0x09,   31 /* Protected */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QReal, QMetaType::QReal,    6,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,

 // slots: parameters
    QMetaType::Void, QMetaType::QVector2D,   13,
    QMetaType::Void, QMetaType::QVector2D,   15,
    QMetaType::Void, QMetaType::QReal,   17,
    QMetaType::Void, QMetaType::QReal,   19,
    QMetaType::Void, QMetaType::QReal,   21,
    QMetaType::Void, QMetaType::QReal, QMetaType::QString,    7,   23,
    QMetaType::Void, QMetaType::QReal,    7,
    QMetaType::Void, QMetaType::Bool,   25,
    QMetaType::Void, QMetaType::Bool,   27,
    QMetaType::Void, QMetaType::Bool,   29,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::VehicleSensor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive13VehicleSensorE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive13VehicleSensorE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive13VehicleSensorE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VehicleSensor, std::true_type>,
        // method 'sensorDataReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DrivingData &, std::false_type>,
        // method 'speedLimitExceeded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'sensorStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sensorStopped'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'updatePosition'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        // method 'updateVelocity'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        // method 'updateRotation'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'updateSteeringAngle'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'updateAcceleration'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'updateSpeedLimit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'updateSpeedLimit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'updateBrakeState'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'updateAcceleratorState'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'updateHonkState'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onSamplingTimer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::VehicleSensor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<VehicleSensor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->sensorDataReady((*reinterpret_cast< std::add_pointer_t<DrivingData>>(_a[1]))); break;
        case 1: _t->speedLimitExceeded((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 2: _t->sensorStarted(); break;
        case 3: _t->sensorStopped(); break;
        case 4: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->updatePosition((*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 6: _t->updateVelocity((*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 7: _t->updateRotation((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 8: _t->updateSteeringAngle((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 9: _t->updateAcceleration((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 10: _t->updateSpeedLimit((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 11: _t->updateSpeedLimit((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 12: _t->updateBrakeState((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 13: _t->updateAcceleratorState((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: _t->updateHonkState((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->onSamplingTimer(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (VehicleSensor::*)(const DrivingData & );
            if (_q_method_type _q_method = &VehicleSensor::sensorDataReady; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (VehicleSensor::*)(qreal , qreal );
            if (_q_method_type _q_method = &VehicleSensor::speedLimitExceeded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (VehicleSensor::*)();
            if (_q_method_type _q_method = &VehicleSensor::sensorStarted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (VehicleSensor::*)();
            if (_q_method_type _q_method = &VehicleSensor::sensorStopped; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (VehicleSensor::*)(const QString & );
            if (_q_method_type _q_method = &VehicleSensor::errorOccurred; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::VehicleSensor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::VehicleSensor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive13VehicleSensorE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::VehicleSensor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::VehicleSensor::sensorDataReady(const DrivingData & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PhantomDrive::VehicleSensor::speedLimitExceeded(qreal _t1, qreal _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::VehicleSensor::sensorStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PhantomDrive::VehicleSensor::sensorStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PhantomDrive::VehicleSensor::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
