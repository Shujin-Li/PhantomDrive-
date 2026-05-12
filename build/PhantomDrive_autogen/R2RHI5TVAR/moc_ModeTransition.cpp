/****************************************************************************
** Meta object code from reading C++ file 'ModeTransition.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/PhantomDrive/gamemode/ModeTransition.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ModeTransition.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive14ModeTransitionE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive14ModeTransitionE = QtMocHelpers::stringData(
    "PhantomDrive::ModeTransition",
    "started",
    "",
    "progressChanged",
    "progress",
    "finished",
    "cancelled",
    "start",
    "stop",
    "finish",
    "onAnimationValueChanged",
    "QVariant",
    "value",
    "onAnimationFinished"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive14ModeTransitionE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x06,    1 /* Public */,
       3,    1,   69,    2, 0x06,    2 /* Public */,
       5,    0,   72,    2, 0x06,    4 /* Public */,
       6,    0,   73,    2, 0x06,    5 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       7,    0,   74,    2, 0x0a,    6 /* Public */,
       8,    0,   75,    2, 0x0a,    7 /* Public */,
       9,    0,   76,    2, 0x0a,    8 /* Public */,
      10,    1,   77,    2, 0x08,    9 /* Private */,
      13,    0,   80,    2, 0x08,   11 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QReal,    4,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::ModeTransition::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive14ModeTransitionE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive14ModeTransitionE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive14ModeTransitionE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ModeTransition, std::true_type>,
        // method 'started'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'progressChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'finished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cancelled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'start'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stop'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'finish'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAnimationValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVariant &, std::false_type>,
        // method 'onAnimationFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::ModeTransition::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ModeTransition *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->started(); break;
        case 1: _t->progressChanged((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 2: _t->finished(); break;
        case 3: _t->cancelled(); break;
        case 4: _t->start(); break;
        case 5: _t->stop(); break;
        case 6: _t->finish(); break;
        case 7: _t->onAnimationValueChanged((*reinterpret_cast< std::add_pointer_t<QVariant>>(_a[1]))); break;
        case 8: _t->onAnimationFinished(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (ModeTransition::*)();
            if (_q_method_type _q_method = &ModeTransition::started; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (ModeTransition::*)(qreal );
            if (_q_method_type _q_method = &ModeTransition::progressChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (ModeTransition::*)();
            if (_q_method_type _q_method = &ModeTransition::finished; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (ModeTransition::*)();
            if (_q_method_type _q_method = &ModeTransition::cancelled; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::ModeTransition::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::ModeTransition::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive14ModeTransitionE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::ModeTransition::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::ModeTransition::started()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PhantomDrive::ModeTransition::progressChanged(qreal _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::ModeTransition::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PhantomDrive::ModeTransition::cancelled()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
