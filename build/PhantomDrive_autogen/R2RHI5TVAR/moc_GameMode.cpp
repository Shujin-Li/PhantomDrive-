/****************************************************************************
** Meta object code from reading C++ file 'GameMode.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/PhantomDrive/gamemode/GameMode.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GameMode.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive8GameModeE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive8GameModeE = QtMocHelpers::stringData(
    "PhantomDrive::GameMode",
    "modeEntered",
    "",
    "modeExited",
    "modeUpdated",
    "elapsedMs",
    "modePaused",
    "modeResumed",
    "modeAboutToExit",
    "transitionRequired",
    "ModeType",
    "from",
    "to",
    "stateChanged",
    "ModeState",
    "oldState",
    "newState",
    "errorOccurred",
    "errorMessage",
    "onVehicleCollision",
    "objectId",
    "onSpeedLimitChanged",
    "newLimit",
    "onTrafficLightChanged",
    "isRed",
    "onCheckpointReached",
    "checkpointId"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive8GameModeE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   92,    2, 0x06,    1 /* Public */,
       3,    0,   93,    2, 0x06,    2 /* Public */,
       4,    1,   94,    2, 0x06,    3 /* Public */,
       6,    0,   97,    2, 0x06,    5 /* Public */,
       7,    0,   98,    2, 0x06,    6 /* Public */,
       8,    0,   99,    2, 0x06,    7 /* Public */,
       9,    2,  100,    2, 0x06,    8 /* Public */,
      13,    2,  105,    2, 0x06,   11 /* Public */,
      17,    1,  110,    2, 0x06,   14 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      19,    1,  113,    2, 0x0a,   16 /* Public */,
      21,    1,  116,    2, 0x0a,   18 /* Public */,
      23,    1,  119,    2, 0x0a,   20 /* Public */,
      25,    1,  122,    2, 0x0a,   22 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 10,   11,   12,
    QMetaType::Void, 0x80000000 | 14, 0x80000000 | 14,   15,   16,
    QMetaType::Void, QMetaType::QString,   18,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,   20,
    QMetaType::Void, QMetaType::QReal,   22,
    QMetaType::Void, QMetaType::Bool,   24,
    QMetaType::Void, QMetaType::Int,   26,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::GameMode::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive8GameModeE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive8GameModeE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive8GameModeE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<GameMode, std::true_type>,
        // method 'modeEntered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'modeExited'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'modeUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'modePaused'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'modeResumed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'modeAboutToExit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'transitionRequired'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<ModeType, std::false_type>,
        QtPrivate::TypeAndForceComplete<ModeType, std::false_type>,
        // method 'stateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<ModeState, std::false_type>,
        QtPrivate::TypeAndForceComplete<ModeState, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onVehicleCollision'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onSpeedLimitChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'onTrafficLightChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onCheckpointReached'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::GameMode::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GameMode *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->modeEntered(); break;
        case 1: _t->modeExited(); break;
        case 2: _t->modeUpdated((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 3: _t->modePaused(); break;
        case 4: _t->modeResumed(); break;
        case 5: _t->modeAboutToExit(); break;
        case 6: _t->transitionRequired((*reinterpret_cast< std::add_pointer_t<ModeType>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ModeType>>(_a[2]))); break;
        case 7: _t->stateChanged((*reinterpret_cast< std::add_pointer_t<ModeState>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ModeState>>(_a[2]))); break;
        case 8: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->onVehicleCollision((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->onSpeedLimitChanged((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 11: _t->onTrafficLightChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->onCheckpointReached((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (GameMode::*)();
            if (_q_method_type _q_method = &GameMode::modeEntered; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (GameMode::*)();
            if (_q_method_type _q_method = &GameMode::modeExited; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (GameMode::*)(qint64 );
            if (_q_method_type _q_method = &GameMode::modeUpdated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (GameMode::*)();
            if (_q_method_type _q_method = &GameMode::modePaused; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (GameMode::*)();
            if (_q_method_type _q_method = &GameMode::modeResumed; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (GameMode::*)();
            if (_q_method_type _q_method = &GameMode::modeAboutToExit; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (GameMode::*)(ModeType , ModeType );
            if (_q_method_type _q_method = &GameMode::transitionRequired; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (GameMode::*)(ModeState , ModeState );
            if (_q_method_type _q_method = &GameMode::stateChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (GameMode::*)(const QString & );
            if (_q_method_type _q_method = &GameMode::errorOccurred; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::GameMode::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::GameMode::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive8GameModeE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::GameMode::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::GameMode::modeEntered()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PhantomDrive::GameMode::modeExited()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PhantomDrive::GameMode::modeUpdated(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PhantomDrive::GameMode::modePaused()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PhantomDrive::GameMode::modeResumed()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PhantomDrive::GameMode::modeAboutToExit()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void PhantomDrive::GameMode::transitionRequired(ModeType _t1, ModeType _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PhantomDrive::GameMode::stateChanged(ModeState _t1, ModeState _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void PhantomDrive::GameMode::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_WARNING_POP
