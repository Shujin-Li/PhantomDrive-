/****************************************************************************
** Meta object code from reading C++ file 'VehicleSensor.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../include/PhantomDrive/gamemode/VehicleSensor.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VehicleSensor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
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

template <> constexpr inline auto PhantomDrive::VehicleSensor::qt_create_metaobjectdata<qt_meta_tag_ZN12PhantomDrive13VehicleSensorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
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
        "QVector2D",
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
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'sensorDataReady'
        QtMocHelpers::SignalData<void(const DrivingData &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'speedLimitExceeded'
        QtMocHelpers::SignalData<void(qreal, qreal)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 6 }, { QMetaType::QReal, 7 },
        }}),
        // Signal 'sensorStarted'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'sensorStopped'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Slot 'updatePosition'
        QtMocHelpers::SlotData<void(const QVector2D &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 13, 14 },
        }}),
        // Slot 'updateVelocity'
        QtMocHelpers::SlotData<void(const QVector2D &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 13, 16 },
        }}),
        // Slot 'updateRotation'
        QtMocHelpers::SlotData<void(qreal)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 18 },
        }}),
        // Slot 'updateSteeringAngle'
        QtMocHelpers::SlotData<void(qreal)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 20 },
        }}),
        // Slot 'updateAcceleration'
        QtMocHelpers::SlotData<void(qreal)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 22 },
        }}),
        // Slot 'updateSpeedLimit'
        QtMocHelpers::SlotData<void(qreal, const QString &)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 7 }, { QMetaType::QString, 24 },
        }}),
        // Slot 'updateSpeedLimit'
        QtMocHelpers::SlotData<void(qreal)>(23, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QReal, 7 },
        }}),
        // Slot 'updateBrakeState'
        QtMocHelpers::SlotData<void(bool)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 26 },
        }}),
        // Slot 'updateAcceleratorState'
        QtMocHelpers::SlotData<void(bool)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 28 },
        }}),
        // Slot 'updateHonkState'
        QtMocHelpers::SlotData<void(bool)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 30 },
        }}),
        // Slot 'onSamplingTimer'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessProtected, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<VehicleSensor, qt_meta_tag_ZN12PhantomDrive13VehicleSensorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PhantomDrive::VehicleSensor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive13VehicleSensorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive13VehicleSensorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12PhantomDrive13VehicleSensorE_t>.metaTypes,
    nullptr
} };

void PhantomDrive::VehicleSensor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<VehicleSensor *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->sensorDataReady((*reinterpret_cast<std::add_pointer_t<DrivingData>>(_a[1]))); break;
        case 1: _t->speedLimitExceeded((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qreal>>(_a[2]))); break;
        case 2: _t->sensorStarted(); break;
        case 3: _t->sensorStopped(); break;
        case 4: _t->errorOccurred((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->updatePosition((*reinterpret_cast<std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 6: _t->updateVelocity((*reinterpret_cast<std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 7: _t->updateRotation((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1]))); break;
        case 8: _t->updateSteeringAngle((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1]))); break;
        case 9: _t->updateAcceleration((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1]))); break;
        case 10: _t->updateSpeedLimit((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 11: _t->updateSpeedLimit((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1]))); break;
        case 12: _t->updateBrakeState((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 13: _t->updateAcceleratorState((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: _t->updateHonkState((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->onSamplingTimer(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (VehicleSensor::*)(const DrivingData & )>(_a, &VehicleSensor::sensorDataReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (VehicleSensor::*)(qreal , qreal )>(_a, &VehicleSensor::speedLimitExceeded, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (VehicleSensor::*)()>(_a, &VehicleSensor::sensorStarted, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (VehicleSensor::*)()>(_a, &VehicleSensor::sensorStopped, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (VehicleSensor::*)(const QString & )>(_a, &VehicleSensor::errorOccurred, 4))
            return;
    }
}

const QMetaObject *PhantomDrive::VehicleSensor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::VehicleSensor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive13VehicleSensorE_t>.strings))
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
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void PhantomDrive::VehicleSensor::speedLimitExceeded(qreal _t1, qreal _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
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
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}
QT_WARNING_POP
