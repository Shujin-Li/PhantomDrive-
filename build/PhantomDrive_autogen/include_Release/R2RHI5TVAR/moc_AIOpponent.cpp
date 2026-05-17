/****************************************************************************
** Meta object code from reading C++ file 'AIOpponent.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/PhantomDrive/gamemode/AIOpponent.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AIOpponent.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive10AIOpponentE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive10AIOpponentE = QtMocHelpers::stringData(
    "PhantomDrive::AIOpponent",
    "stateChanged",
    "",
    "AIState",
    "oldState",
    "newState",
    "waypointReached",
    "index",
    "lapCompleted",
    "lapNumber",
    "lapTime",
    "bestLapTimeUpdated",
    "newBestTime",
    "positionChanged",
    "oldPosition",
    "newPosition",
    "collisionOccurred",
    "objectId",
    "point",
    "powerupUsed",
    "slot",
    "powerupType",
    "powerupCollected",
    "overtakeAttempted",
    "targetId",
    "overtakeCompleted",
    "raceFinished",
    "finalPosition",
    "totalTime",
    "decisionMade",
    "PhantomDrive::AIDecision",
    "decision",
    "updated",
    "elapsedMs"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive10AIOpponentE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      13,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   92,    2, 0x06,    1 /* Public */,
       6,    1,   97,    2, 0x06,    4 /* Public */,
       8,    2,  100,    2, 0x06,    6 /* Public */,
      11,    1,  105,    2, 0x06,    9 /* Public */,
      13,    2,  108,    2, 0x06,   11 /* Public */,
      16,    2,  113,    2, 0x06,   14 /* Public */,
      19,    2,  118,    2, 0x06,   17 /* Public */,
      22,    1,  123,    2, 0x06,   20 /* Public */,
      23,    1,  126,    2, 0x06,   22 /* Public */,
      25,    1,  129,    2, 0x06,   24 /* Public */,
      26,    2,  132,    2, 0x06,   26 /* Public */,
      29,    1,  137,    2, 0x06,   29 /* Public */,
      32,    1,  140,    2, 0x06,   31 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    4,    5,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int, QMetaType::QReal,    9,   10,
    QMetaType::Void, QMetaType::QReal,   12,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   14,   15,
    QMetaType::Void, QMetaType::QString, QMetaType::QVector2D,   17,   18,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   20,   21,
    QMetaType::Void, QMetaType::Int,   21,
    QMetaType::Void, QMetaType::QString,   24,
    QMetaType::Void, QMetaType::QString,   24,
    QMetaType::Void, QMetaType::Int, QMetaType::QReal,   27,   28,
    QMetaType::Void, 0x80000000 | 30,   31,
    QMetaType::Void, QMetaType::LongLong,   33,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::AIOpponent::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive10AIOpponentE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive10AIOpponentE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive10AIOpponentE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AIOpponent, std::true_type>,
        // method 'stateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<AIState, std::false_type>,
        QtPrivate::TypeAndForceComplete<AIState, std::false_type>,
        // method 'waypointReached'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'lapCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'bestLapTimeUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'positionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'collisionOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        // method 'powerupUsed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'powerupCollected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'overtakeAttempted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'overtakeCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'raceFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'decisionMade'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const PhantomDrive::AIDecision &, std::false_type>,
        // method 'updated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::AIOpponent::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AIOpponent *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast< std::add_pointer_t<AIState>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<AIState>>(_a[2]))); break;
        case 1: _t->waypointReached((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->lapCompleted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 3: _t->bestLapTimeUpdated((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 4: _t->positionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 5: _t->collisionOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2]))); break;
        case 6: _t->powerupUsed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->powerupCollected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->overtakeAttempted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->overtakeCompleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->raceFinished((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 11: _t->decisionMade((*reinterpret_cast< std::add_pointer_t<PhantomDrive::AIDecision>>(_a[1]))); break;
        case 12: _t->updated((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (AIOpponent::*)(AIState , AIState );
            if (_q_method_type _q_method = &AIOpponent::stateChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(int );
            if (_q_method_type _q_method = &AIOpponent::waypointReached; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(int , qreal );
            if (_q_method_type _q_method = &AIOpponent::lapCompleted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(qreal );
            if (_q_method_type _q_method = &AIOpponent::bestLapTimeUpdated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(int , int );
            if (_q_method_type _q_method = &AIOpponent::positionChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(const QString & , const QVector2D & );
            if (_q_method_type _q_method = &AIOpponent::collisionOccurred; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(int , int );
            if (_q_method_type _q_method = &AIOpponent::powerupUsed; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(int );
            if (_q_method_type _q_method = &AIOpponent::powerupCollected; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(const QString & );
            if (_q_method_type _q_method = &AIOpponent::overtakeAttempted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(const QString & );
            if (_q_method_type _q_method = &AIOpponent::overtakeCompleted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(int , qreal );
            if (_q_method_type _q_method = &AIOpponent::raceFinished; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(const PhantomDrive::AIDecision & );
            if (_q_method_type _q_method = &AIOpponent::decisionMade; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _q_method_type = void (AIOpponent::*)(qint64 );
            if (_q_method_type _q_method = &AIOpponent::updated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::AIOpponent::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::AIOpponent::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive10AIOpponentE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::AIOpponent::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void PhantomDrive::AIOpponent::stateChanged(AIState _t1, AIState _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PhantomDrive::AIOpponent::waypointReached(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::AIOpponent::lapCompleted(int _t1, qreal _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PhantomDrive::AIOpponent::bestLapTimeUpdated(qreal _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PhantomDrive::AIOpponent::positionChanged(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PhantomDrive::AIOpponent::collisionOccurred(const QString & _t1, const QVector2D & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void PhantomDrive::AIOpponent::powerupUsed(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PhantomDrive::AIOpponent::powerupCollected(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void PhantomDrive::AIOpponent::overtakeAttempted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void PhantomDrive::AIOpponent::overtakeCompleted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void PhantomDrive::AIOpponent::raceFinished(int _t1, qreal _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void PhantomDrive::AIOpponent::decisionMade(const PhantomDrive::AIDecision & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void PhantomDrive::AIOpponent::updated(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}
QT_WARNING_POP
