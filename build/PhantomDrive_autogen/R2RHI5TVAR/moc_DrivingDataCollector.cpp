/****************************************************************************
** Meta object code from reading C++ file 'DrivingDataCollector.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/PhantomDrive/gamemode/DrivingDataCollector.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DrivingDataCollector.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive20DrivingDataCollectorE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive20DrivingDataCollectorE = QtMocHelpers::stringData(
    "PhantomDrive::DrivingDataCollector",
    "collectionStarted",
    "",
    "samplingInterval",
    "collectionStopped",
    "totalDataPoints",
    "storageBufferFull",
    "criticalCollisionDetected",
    "objectId",
    "impactForce",
    "dataExportCompleted",
    "filePath",
    "dataImportCompleted",
    "registerCollidableObject",
    "position",
    "radius",
    "updateCollidableObject",
    "removeCollidableObject",
    "setCurrentSpeedLimit",
    "limit",
    "zoneId",
    "updateCheckpointProgress",
    "checkpointId",
    "lapTime",
    "currentLap",
    "onSensorDataReady",
    "DrivingData",
    "data",
    "onCollisionDetected",
    "onSpeedLimitExceeded",
    "currentSpeed",
    "onStorageBufferFull"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive20DrivingDataCollectorE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  116,    2, 0x06,    1 /* Public */,
       4,    1,  119,    2, 0x06,    3 /* Public */,
       6,    0,  122,    2, 0x06,    5 /* Public */,
       7,    2,  123,    2, 0x06,    6 /* Public */,
      10,    1,  128,    2, 0x06,    9 /* Public */,
      12,    1,  131,    2, 0x06,   11 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      13,    3,  134,    2, 0x0a,   13 /* Public */,
      13,    2,  141,    2, 0x2a,   17 /* Public | MethodCloned */,
      16,    2,  146,    2, 0x0a,   20 /* Public */,
      17,    1,  151,    2, 0x0a,   23 /* Public */,
      18,    2,  154,    2, 0x0a,   25 /* Public */,
      18,    1,  159,    2, 0x2a,   28 /* Public | MethodCloned */,
      21,    3,  162,    2, 0x0a,   30 /* Public */,
      25,    1,  169,    2, 0x09,   34 /* Protected */,
      28,    3,  172,    2, 0x09,   36 /* Protected */,
      29,    2,  179,    2, 0x09,   40 /* Protected */,
      31,    0,  184,    2, 0x09,   43 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QReal,    8,    9,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QString,   11,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QVector2D, QMetaType::QReal,    8,   14,   15,
    QMetaType::Void, QMetaType::QString, QMetaType::QVector2D,    8,   14,
    QMetaType::Void, QMetaType::QString, QMetaType::QVector2D,    8,   14,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::QReal, QMetaType::QString,   19,   20,
    QMetaType::Void, QMetaType::QReal,   19,
    QMetaType::Void, QMetaType::Int, QMetaType::QReal, QMetaType::Int,   22,   23,   24,
    QMetaType::Void, 0x80000000 | 26,   27,
    QMetaType::Void, QMetaType::QString, QMetaType::QVector2D, QMetaType::QReal,    8,   14,    9,
    QMetaType::Void, QMetaType::QReal, QMetaType::QReal,   30,   19,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::DrivingDataCollector::staticMetaObject = { {
    QMetaObject::SuperData::link<IDrivingDataCollector::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive20DrivingDataCollectorE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive20DrivingDataCollectorE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive20DrivingDataCollectorE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DrivingDataCollector, std::true_type>,
        // method 'collectionStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'collectionStopped'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'storageBufferFull'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'criticalCollisionDetected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'dataExportCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'dataImportCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'registerCollidableObject'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'registerCollidableObject'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        // method 'updateCollidableObject'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        // method 'removeCollidableObject'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setCurrentSpeedLimit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setCurrentSpeedLimit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'updateCheckpointProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint32, std::false_type>,
        // method 'onSensorDataReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DrivingData &, std::false_type>,
        // method 'onCollisionDetected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'onSpeedLimitExceeded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'onStorageBufferFull'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::DrivingDataCollector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DrivingDataCollector *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->collectionStarted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->collectionStopped((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->storageBufferFull(); break;
        case 3: _t->criticalCollisionDetected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 4: _t->dataExportCompleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->dataImportCompleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->registerCollidableObject((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 7: _t->registerCollidableObject((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2]))); break;
        case 8: _t->updateCollidableObject((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2]))); break;
        case 9: _t->removeCollidableObject((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->setCurrentSpeedLimit((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 11: _t->setCurrentSpeedLimit((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 12: _t->updateCheckpointProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qint32>>(_a[3]))); break;
        case 13: _t->onSensorDataReady((*reinterpret_cast< std::add_pointer_t<DrivingData>>(_a[1]))); break;
        case 14: _t->onCollisionDetected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[3]))); break;
        case 15: _t->onSpeedLimitExceeded((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qreal>>(_a[2]))); break;
        case 16: _t->onStorageBufferFull(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (DrivingDataCollector::*)(int );
            if (_q_method_type _q_method = &DrivingDataCollector::collectionStarted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataCollector::*)(int );
            if (_q_method_type _q_method = &DrivingDataCollector::collectionStopped; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataCollector::*)();
            if (_q_method_type _q_method = &DrivingDataCollector::storageBufferFull; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataCollector::*)(const QString & , qreal );
            if (_q_method_type _q_method = &DrivingDataCollector::criticalCollisionDetected; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataCollector::*)(const QString & );
            if (_q_method_type _q_method = &DrivingDataCollector::dataExportCompleted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataCollector::*)(const QString & );
            if (_q_method_type _q_method = &DrivingDataCollector::dataImportCompleted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::DrivingDataCollector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::DrivingDataCollector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive20DrivingDataCollectorE.stringdata0))
        return static_cast<void*>(this);
    return IDrivingDataCollector::qt_metacast(_clname);
}

int PhantomDrive::DrivingDataCollector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = IDrivingDataCollector::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::DrivingDataCollector::collectionStarted(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PhantomDrive::DrivingDataCollector::collectionStopped(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::DrivingDataCollector::storageBufferFull()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PhantomDrive::DrivingDataCollector::criticalCollisionDetected(const QString & _t1, qreal _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PhantomDrive::DrivingDataCollector::dataExportCompleted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PhantomDrive::DrivingDataCollector::dataImportCompleted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
