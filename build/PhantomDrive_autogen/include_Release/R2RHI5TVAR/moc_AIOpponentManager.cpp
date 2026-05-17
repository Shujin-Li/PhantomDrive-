/****************************************************************************
** Meta object code from reading C++ file 'AIOpponentManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/PhantomDrive/gamemode/AIOpponentManager.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AIOpponentManager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive17AIOpponentManagerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive17AIOpponentManagerE = QtMocHelpers::stringData(
    "PhantomDrive::AIOpponentManager",
    "opponentAdded",
    "",
    "opponentId",
    "opponentRemoved",
    "opponentStateChanged",
    "AIState",
    "oldState",
    "newState",
    "opponentFinished",
    "finalPosition",
    "allOpponentsFinished",
    "rankingsUpdated",
    "orderedOpponentIds"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive17AIOpponentManagerE[] = {

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
       1,    1,   50,    2, 0x06,    1 /* Public */,
       4,    1,   53,    2, 0x06,    3 /* Public */,
       5,    3,   56,    2, 0x06,    5 /* Public */,
       9,    2,   63,    2, 0x06,    9 /* Public */,
      11,    0,   68,    2, 0x06,   12 /* Public */,
      12,    1,   69,    2, 0x06,   13 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 6, 0x80000000 | 6,    3,    7,    8,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    3,   10,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QStringList,   13,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::AIOpponentManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive17AIOpponentManagerE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive17AIOpponentManagerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive17AIOpponentManagerE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AIOpponentManager, std::true_type>,
        // method 'opponentAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'opponentRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'opponentStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<AIState, std::false_type>,
        QtPrivate::TypeAndForceComplete<AIState, std::false_type>,
        // method 'opponentFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'allOpponentsFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'rankingsUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<QString> &, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::AIOpponentManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AIOpponentManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->opponentAdded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->opponentRemoved((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->opponentStateChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<AIState>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<AIState>>(_a[3]))); break;
        case 3: _t->opponentFinished((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->allOpponentsFinished(); break;
        case 5: _t->rankingsUpdated((*reinterpret_cast< std::add_pointer_t<QList<QString>>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (AIOpponentManager::*)(const QString & );
            if (_q_method_type _q_method = &AIOpponentManager::opponentAdded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponentManager::*)(const QString & );
            if (_q_method_type _q_method = &AIOpponentManager::opponentRemoved; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponentManager::*)(const QString & , AIState , AIState );
            if (_q_method_type _q_method = &AIOpponentManager::opponentStateChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponentManager::*)(const QString & , int );
            if (_q_method_type _q_method = &AIOpponentManager::opponentFinished; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponentManager::*)();
            if (_q_method_type _q_method = &AIOpponentManager::allOpponentsFinished; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponentManager::*)(const QList<QString> & );
            if (_q_method_type _q_method = &AIOpponentManager::rankingsUpdated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::AIOpponentManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::AIOpponentManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive17AIOpponentManagerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::AIOpponentManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void PhantomDrive::AIOpponentManager::opponentAdded(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PhantomDrive::AIOpponentManager::opponentRemoved(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::AIOpponentManager::opponentStateChanged(const QString & _t1, AIState _t2, AIState _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PhantomDrive::AIOpponentManager::opponentFinished(const QString & _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PhantomDrive::AIOpponentManager::allOpponentsFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PhantomDrive::AIOpponentManager::rankingsUpdated(const QList<QString> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
