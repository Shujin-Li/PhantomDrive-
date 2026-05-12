/****************************************************************************
** Meta object code from reading C++ file 'CollisionDetector.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/PhantomDrive/gamemode/CollisionDetector.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CollisionDetector.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive17CollisionDetectorE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive17CollisionDetectorE = QtMocHelpers::stringData(
    "PhantomDrive::CollisionDetector",
    "collisionDetected",
    "",
    "objectId",
    "position",
    "impactForce",
    "collisionEnded",
    "nearMissDetected",
    "distance",
    "collisionDataRecorded",
    "DrivingData",
    "data",
    "detectorEnabled",
    "enabled",
    "errorOccurred",
    "error",
    "updateVehiclePosition",
    "updateVehicleRotation",
    "rotation",
    "checkCollisions"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive17CollisionDetectorE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,   68,    2, 0x06,    1 /* Public */,
       6,    1,   75,    2, 0x06,    5 /* Public */,
       7,    2,   78,    2, 0x06,    7 /* Public */,
       9,    1,   83,    2, 0x06,   10 /* Public */,
      12,    1,   86,    2, 0x06,   12 /* Public */,
      14,    1,   89,    2, 0x06,   14 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      16,    1,   92,    2, 0x0a,   16 /* Public */,
      17,    1,   95,    2, 0x0a,   18 /* Public */,
      19,    0,   98,    2, 0x0a,   20 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QVector2D, QMetaType::QReal,    3,    4,    5,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QReal,    3,    8,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void, QMetaType::QString,   15,

 // slots: parameters
    QMetaType::Void, QMetaType::QVector2D,    4,
    QMetaType::Void, QMetaType::QReal,   18,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::CollisionDetector::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive17CollisionDetectorE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive17CollisionDetectorE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive17CollisionDetectorE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CollisionDetector, std::true_type>,
        // method 'collisionDetected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'collisionEnded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'nearMissDetected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'collisionDataRecorded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DrivingData &, std::false_type>,
        // method 'detectorEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'updateVehiclePosition'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        // method 'updateVehicleRotation'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'checkCollisions'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::CollisionDetector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CollisionDetector *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->collisionDetected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 1: _t->collisionEnded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->nearMissDetected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 3: _t->collisionDataRecorded((*reinterpret_cast< std::add_pointer_t<DrivingData>>(_a[1]))); break;
        case 4: _t->detectorEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->updateVehiclePosition((*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 7: _t->updateVehicleRotation((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 8: _t->checkCollisions(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (CollisionDetector::*)(const QString & , const QVector2D & , qreal );
            if (_q_method_type _q_method = &CollisionDetector::collisionDetected; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (CollisionDetector::*)(const QString & );
            if (_q_method_type _q_method = &CollisionDetector::collisionEnded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (CollisionDetector::*)(const QString & , qreal );
            if (_q_method_type _q_method = &CollisionDetector::nearMissDetected; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (CollisionDetector::*)(const DrivingData & );
            if (_q_method_type _q_method = &CollisionDetector::collisionDataRecorded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (CollisionDetector::*)(bool );
            if (_q_method_type _q_method = &CollisionDetector::detectorEnabled; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (CollisionDetector::*)(const QString & );
            if (_q_method_type _q_method = &CollisionDetector::errorOccurred; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::CollisionDetector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::CollisionDetector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive17CollisionDetectorE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::CollisionDetector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void PhantomDrive::CollisionDetector::collisionDetected(const QString & _t1, const QVector2D & _t2, qreal _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PhantomDrive::CollisionDetector::collisionEnded(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::CollisionDetector::nearMissDetected(const QString & _t1, qreal _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PhantomDrive::CollisionDetector::collisionDataRecorded(const DrivingData & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PhantomDrive::CollisionDetector::detectorEnabled(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PhantomDrive::CollisionDetector::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
