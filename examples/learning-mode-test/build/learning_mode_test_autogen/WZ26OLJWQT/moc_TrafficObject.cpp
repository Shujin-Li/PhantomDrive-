/****************************************************************************
** Meta object code from reading C++ file 'TrafficObject.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../include/PhantomDrive/gamemode/TrafficObject.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TrafficObject.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive13TrafficObjectE_t {};
} // unnamed namespace

template <> constexpr inline auto PhantomDrive::TrafficObject::qt_create_metaobjectdata<qt_meta_tag_ZN12PhantomDrive13TrafficObjectE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PhantomDrive::TrafficObject",
        "stateChanged",
        "",
        "TrafficObjectState",
        "oldState",
        "newState",
        "positionChanged",
        "QVector2D",
        "newPosition",
        "boundsChanged",
        "QRectF",
        "newBounds",
        "enabledChanged",
        "enabled",
        "violationTriggered",
        "objectId",
        "position",
        "objectUpdated"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stateChanged'
        QtMocHelpers::SignalData<void(TrafficObjectState, TrafficObjectState)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 3, 5 },
        }}),
        // Signal 'positionChanged'
        QtMocHelpers::SignalData<void(const QVector2D &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Signal 'boundsChanged'
        QtMocHelpers::SignalData<void(const QRectF &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 },
        }}),
        // Signal 'enabledChanged'
        QtMocHelpers::SignalData<void(bool)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Signal 'violationTriggered'
        QtMocHelpers::SignalData<void(const QString &, const QVector2D &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 15 }, { 0x80000000 | 7, 16 },
        }}),
        // Signal 'objectUpdated'
        QtMocHelpers::SignalData<void(const QString &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 15 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TrafficObject, qt_meta_tag_ZN12PhantomDrive13TrafficObjectE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PhantomDrive::TrafficObject::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive13TrafficObjectE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive13TrafficObjectE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12PhantomDrive13TrafficObjectE_t>.metaTypes,
    nullptr
} };

void PhantomDrive::TrafficObject::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TrafficObject *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast<std::add_pointer_t<TrafficObjectState>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<TrafficObjectState>>(_a[2]))); break;
        case 1: _t->positionChanged((*reinterpret_cast<std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 2: _t->boundsChanged((*reinterpret_cast<std::add_pointer_t<QRectF>>(_a[1]))); break;
        case 3: _t->enabledChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->violationTriggered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVector2D>>(_a[2]))); break;
        case 5: _t->objectUpdated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TrafficObject::*)(TrafficObjectState , TrafficObjectState )>(_a, &TrafficObject::stateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObject::*)(const QVector2D & )>(_a, &TrafficObject::positionChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObject::*)(const QRectF & )>(_a, &TrafficObject::boundsChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObject::*)(bool )>(_a, &TrafficObject::enabledChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObject::*)(const QString & , const QVector2D & )>(_a, &TrafficObject::violationTriggered, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrafficObject::*)(const QString & )>(_a, &TrafficObject::objectUpdated, 5))
            return;
    }
}

const QMetaObject *PhantomDrive::TrafficObject::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::TrafficObject::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive13TrafficObjectE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::TrafficObject::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::TrafficObject::stateChanged(TrafficObjectState _t1, TrafficObjectState _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void PhantomDrive::TrafficObject::positionChanged(const QVector2D & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void PhantomDrive::TrafficObject::boundsChanged(const QRectF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void PhantomDrive::TrafficObject::enabledChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void PhantomDrive::TrafficObject::violationTriggered(const QString & _t1, const QVector2D & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void PhantomDrive::TrafficObject::objectUpdated(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}
QT_WARNING_POP
