/****************************************************************************
** Meta object code from reading C++ file 'TrafficLightObject.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/PhantomDrive/gamemode/TrafficLightObject.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TrafficLightObject.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive18TrafficLightObjectE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive18TrafficLightObjectE = QtMocHelpers::stringData(
    "PhantomDrive::TrafficLightObject",
    "colorChanged",
    "",
    "TrafficLightObject::LightColor",
    "oldColor",
    "newColor",
    "redLightViolation",
    "lightId",
    "cycleCompleted",
    "cycleCount",
    "timerExpired",
    "onVehicleApproaching",
    "vehiclePosition",
    "reset"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive18TrafficLightObjectE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   50,    2, 0x06,    1 /* Public */,
       6,    1,   55,    2, 0x06,    4 /* Public */,
       8,    1,   58,    2, 0x06,    6 /* Public */,
      10,    0,   61,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    1,   62,    2, 0x0a,    9 /* Public */,
      13,    0,   65,    2, 0x0a,   11 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QVector2D,   12,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::TrafficLightObject::staticMetaObject = { {
    QMetaObject::SuperData::link<TrafficObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive18TrafficLightObjectE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive18TrafficLightObjectE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive18TrafficLightObjectE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TrafficLightObject, std::true_type>,
        // method 'colorChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<TrafficLightObject::LightColor, std::false_type>,
        QtPrivate::TypeAndForceComplete<TrafficLightObject::LightColor, std::false_type>,
        // method 'redLightViolation'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'cycleCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'timerExpired'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onVehicleApproaching'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        // method 'reset'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::TrafficLightObject::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TrafficLightObject *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->colorChanged((*reinterpret_cast< std::add_pointer_t<TrafficLightObject::LightColor>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<TrafficLightObject::LightColor>>(_a[2]))); break;
        case 1: _t->redLightViolation((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->cycleCompleted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->timerExpired(); break;
        case 4: _t->onVehicleApproaching((*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 5: _t->reset(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (TrafficLightObject::*)(TrafficLightObject::LightColor , TrafficLightObject::LightColor );
            if (_q_method_type _q_method = &TrafficLightObject::colorChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (TrafficLightObject::*)(const QString & );
            if (_q_method_type _q_method = &TrafficLightObject::redLightViolation; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (TrafficLightObject::*)(int );
            if (_q_method_type _q_method = &TrafficLightObject::cycleCompleted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (TrafficLightObject::*)();
            if (_q_method_type _q_method = &TrafficLightObject::timerExpired; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::TrafficLightObject::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::TrafficLightObject::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive18TrafficLightObjectE.stringdata0))
        return static_cast<void*>(this);
    return TrafficObject::qt_metacast(_clname);
}

int PhantomDrive::TrafficLightObject::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = TrafficObject::qt_metacall(_c, _id, _a);
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
void PhantomDrive::TrafficLightObject::colorChanged(TrafficLightObject::LightColor _t1, TrafficLightObject::LightColor _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PhantomDrive::TrafficLightObject::redLightViolation(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::TrafficLightObject::cycleCompleted(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PhantomDrive::TrafficLightObject::timerExpired()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
