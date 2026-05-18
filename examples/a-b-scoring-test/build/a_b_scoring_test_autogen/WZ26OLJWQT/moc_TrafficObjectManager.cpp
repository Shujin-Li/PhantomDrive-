/****************************************************************************
** Meta object code from reading C++ file 'TrafficObjectManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../include/PhantomDrive/gamemode/TrafficObjectManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TrafficObjectManager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive20TrafficObjectManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto PhantomDrive::TrafficObjectManager::qt_create_metaobjectdata<qt_meta_tag_ZN12PhantomDrive20TrafficObjectManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PhantomDrive::TrafficObjectManager",
        "objectRegistered",
        "",
        "objectId",
        "TrafficObjectType",
        "type",
        "objectUnregistered",
        "trafficLightViolated",
        "lightId",
        "speedLimitViolated",
        "signId",
        "speed",
        "limit",
        "pedestrianViolation",
        "crossingId",
        "allObjectsUpdated",
        "managerCleared",
        "clear"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'objectRegistered'
        QtMocHelpers::SignalData<void(const QString &, TrafficObjectType)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { 0x80000000 | 4, 5 },
        }}),
        // Signal 'objectUnregistered'
        QtMocHelpers::SignalData<void(const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'trafficLightViolated'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Signal 'speedLimitViolated'
        QtMocHelpers::SignalData<void(const QString &, qreal, qreal)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 }, { QMetaType::QReal, 11 }, { QMetaType::QReal, 12 },
        }}),
        // Signal 'pedestrianViolation'
        QtMocHelpers::SignalData<void(const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Signal 'allObjectsUpdated'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'managerCleared'
        QtMocHelpers::SignalData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'clear'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TrafficObjectManager, qt_meta_tag_ZN12PhantomDrive20TrafficObjectManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PhantomDrive::TrafficObjectManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive20TrafficObjectManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive20TrafficObjectManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12PhantomDrive20TrafficObjectManagerE_t>.metaTypes,
    nullptr
} };

void PhantomDrive::TrafficObjectManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TrafficObjectManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->objectRegistered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<TrafficObjectType>>(_a[2]))); break;
        case 1: _t->objectUnregistered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->trafficLightViolated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->speedLimitViolated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qreal>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<qreal>>(_a[3]))); break;
        case 4: _t->pedestrianViolation((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->allObjectsUpdated(); break;
        case 6: _t->managerCleared(); break;
        case 7: _t->clear(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TrafficObjectManager::*)(const QString & , TrafficObjectType )>(_a, &TrafficObjectManager::objectRegistered, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObjectManager::*)(const QString & )>(_a, &TrafficObjectManager::objectUnregistered, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObjectManager::*)(const QString & )>(_a, &TrafficObjectManager::trafficLightViolated, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObjectManager::*)(const QString & , qreal , qreal )>(_a, &TrafficObjectManager::speedLimitViolated, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObjectManager::*)(const QString & )>(_a, &TrafficObjectManager::pedestrianViolation, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObjectManager::*)()>(_a, &TrafficObjectManager::allObjectsUpdated, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObjectManager::*)()>(_a, &TrafficObjectManager::managerCleared, 6))
            return;
    }
}

const QMetaObject *PhantomDrive::TrafficObjectManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::TrafficObjectManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive20TrafficObjectManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::TrafficObjectManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::TrafficObjectManager::objectRegistered(const QString & _t1, TrafficObjectType _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void PhantomDrive::TrafficObjectManager::objectUnregistered(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void PhantomDrive::TrafficObjectManager::trafficLightViolated(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void PhantomDrive::TrafficObjectManager::speedLimitViolated(const QString & _t1, qreal _t2, qreal _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3);
}

// SIGNAL 4
void PhantomDrive::TrafficObjectManager::pedestrianViolation(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void PhantomDrive::TrafficObjectManager::allObjectsUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void PhantomDrive::TrafficObjectManager::managerCleared()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
