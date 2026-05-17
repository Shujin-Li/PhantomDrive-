/****************************************************************************
** Meta object code from reading C++ file 'TrafficObject.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/PhantomDrive/gamemode/TrafficObject.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TrafficObject.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive13TrafficObjectE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive13TrafficObjectE = QtMocHelpers::stringData(
    "PhantomDrive::TrafficObject",
    "stateChanged",
    "",
    "TrafficObjectState",
    "oldState",
    "newState",
    "positionChanged",
    "newPosition",
    "boundsChanged",
    "newBounds",
    "enabledChanged",
    "enabled",
    "violationTriggered",
    "objectId",
    "position",
    "objectUpdated"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive13TrafficObjectE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   50,    2, 0x06,    1 /* Public */,
       6,    1,   55,    2, 0x06,    4 /* Public */,
       8,    1,   58,    2, 0x06,    6 /* Public */,
      10,    1,   61,    2, 0x06,    8 /* Public */,
      12,    2,   64,    2, 0x06,   10 /* Public */,
      15,    1,   69,    2, 0x06,   13 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, QMetaType::QVector2D,    7,
    QMetaType::Void, QMetaType::QRectF,    9,
    QMetaType::Void, QMetaType::Bool,   11,
    QMetaType::Void, QMetaType::QString, QMetaType::QVector2D,   13,   14,
    QMetaType::Void, QMetaType::QString,   13,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::TrafficObject::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive13TrafficObjectE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive13TrafficObjectE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive13TrafficObjectE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TrafficObject, std::true_type>,
        // method 'stateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<TrafficObjectState, std::false_type>,
        QtPrivate::TypeAndForceComplete<TrafficObjectState, std::false_type>,
        // method 'positionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        // method 'boundsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRectF &, std::false_type>,
        // method 'enabledChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'violationTriggered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        // method 'objectUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::TrafficObject::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TrafficObject *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast< std::add_pointer_t<TrafficObjectState>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<TrafficObjectState>>(_a[2]))); break;
        case 1: _t->positionChanged((*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 2: _t->boundsChanged((*reinterpret_cast< std::add_pointer_t<QRectF>>(_a[1]))); break;
        case 3: _t->enabledChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->violationTriggered((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2]))); break;
        case 5: _t->objectUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (TrafficObject::*)(TrafficObjectState , TrafficObjectState );
            if (_q_method_type _q_method = &TrafficObject::stateChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (TrafficObject::*)(const QVector2D & );
            if (_q_method_type _q_method = &TrafficObject::positionChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (TrafficObject::*)(const QRectF & );
            if (_q_method_type _q_method = &TrafficObject::boundsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (TrafficObject::*)(bool );
            if (_q_method_type _q_method = &TrafficObject::enabledChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (TrafficObject::*)(const QString & , const QVector2D & );
            if (_q_method_type _q_method = &TrafficObject::violationTriggered; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (TrafficObject::*)(const QString & );
            if (_q_method_type _q_method = &TrafficObject::objectUpdated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::TrafficObject::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::TrafficObject::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive13TrafficObjectE.stringdata0))
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
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PhantomDrive::TrafficObject::positionChanged(const QVector2D & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::TrafficObject::boundsChanged(const QRectF & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PhantomDrive::TrafficObject::enabledChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PhantomDrive::TrafficObject::violationTriggered(const QString & _t1, const QVector2D & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PhantomDrive::TrafficObject::objectUpdated(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
