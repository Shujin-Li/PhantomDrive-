/****************************************************************************
** Meta object code from reading C++ file 'LearningMode.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/PhantomDrive/gamemode/LearningMode.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LearningMode.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive12LearningModeE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive12LearningModeE = QtMocHelpers::stringData(
    "PhantomDrive::LearningMode",
    "trafficLightChanged",
    "",
    "id",
    "isRed",
    "speedLimitExceeded",
    "currentSpeed",
    "limit",
    "violationRecorded",
    "ViolationEvent",
    "violation",
    "pedestrianCollisionDetected",
    "crossingId",
    "scoreChanged",
    "newScore",
    "redLightViolation",
    "trafficLightId",
    "learningSessionCompleted",
    "finalScore",
    "violationCount",
    "aiCoachFeedbackGenerated",
    "feedback",
    "scoreImpact",
    "onTrafficLightChanged",
    "onSpeedLimitChanged",
    "newLimit",
    "onVehicleCollision",
    "objectId"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive12LearningModeE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   80,    2, 0x06,    1 /* Public */,
       5,    2,   85,    2, 0x06,    4 /* Public */,
       8,    1,   90,    2, 0x06,    7 /* Public */,
      11,    1,   93,    2, 0x06,    9 /* Public */,
      13,    1,   96,    2, 0x06,   11 /* Public */,
      15,    1,   99,    2, 0x06,   13 /* Public */,
      17,    2,  102,    2, 0x06,   15 /* Public */,
      20,    2,  107,    2, 0x06,   18 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      23,    1,  112,    2, 0x0a,   21 /* Public */,
      24,    1,  115,    2, 0x0a,   23 /* Public */,
      26,    1,  118,    2, 0x0a,   25 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    4,
    QMetaType::Void, QMetaType::QReal, QMetaType::QReal,    6,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QReal,   14,
    QMetaType::Void, QMetaType::QString,   16,
    QMetaType::Void, QMetaType::QReal, QMetaType::Int,   18,   19,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,   21,   22,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, QMetaType::QReal,   25,
    QMetaType::Void, QMetaType::QString,   27,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::LearningMode::staticMetaObject = { {
    QMetaObject::SuperData::link<GameMode::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive12LearningModeE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive12LearningModeE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive12LearningModeE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LearningMode, std::true_type>,
        // method 'trafficLightChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'speedLimitExceeded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'violationRecorded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const ViolationEvent &, std::false_type>,
        // method 'pedestrianCollisionDetected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'scoreChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'redLightViolation'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'learningSessionCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'aiCoachFeedbackGenerated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onTrafficLightChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onSpeedLimitChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'onVehicleCollision'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::LearningMode::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<LearningMode *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->trafficLightChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 1: _t->speedLimitExceeded((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 2: _t->violationRecorded((*reinterpret_cast< std::add_pointer_t<ViolationEvent>>(_a[1]))); break;
        case 3: _t->pedestrianCollisionDetected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->scoreChanged((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 5: _t->redLightViolation((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->learningSessionCompleted((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->aiCoachFeedbackGenerated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 8: _t->onTrafficLightChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->onSpeedLimitChanged((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 10: _t->onVehicleCollision((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (LearningMode::*)(const QString & , bool );
            if (_q_method_type _q_method = &LearningMode::trafficLightChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (LearningMode::*)(qreal , qreal );
            if (_q_method_type _q_method = &LearningMode::speedLimitExceeded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (LearningMode::*)(const ViolationEvent & );
            if (_q_method_type _q_method = &LearningMode::violationRecorded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (LearningMode::*)(const QString & );
            if (_q_method_type _q_method = &LearningMode::pedestrianCollisionDetected; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (LearningMode::*)(qreal );
            if (_q_method_type _q_method = &LearningMode::scoreChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (LearningMode::*)(const QString & );
            if (_q_method_type _q_method = &LearningMode::redLightViolation; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (LearningMode::*)(qreal , int );
            if (_q_method_type _q_method = &LearningMode::learningSessionCompleted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (LearningMode::*)(const QString & , int );
            if (_q_method_type _q_method = &LearningMode::aiCoachFeedbackGenerated; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::LearningMode::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::LearningMode::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive12LearningModeE.stringdata0))
        return static_cast<void*>(this);
    return GameMode::qt_metacast(_clname);
}

int PhantomDrive::LearningMode::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = GameMode::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::LearningMode::trafficLightChanged(const QString & _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PhantomDrive::LearningMode::speedLimitExceeded(qreal _t1, qreal _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::LearningMode::violationRecorded(const ViolationEvent & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PhantomDrive::LearningMode::pedestrianCollisionDetected(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PhantomDrive::LearningMode::scoreChanged(qreal _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PhantomDrive::LearningMode::redLightViolation(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void PhantomDrive::LearningMode::learningSessionCompleted(qreal _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PhantomDrive::LearningMode::aiCoachFeedbackGenerated(const QString & _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
