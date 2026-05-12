/****************************************************************************
** Meta object code from reading C++ file 'CollisionDetector.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../include/PhantomDrive/gamemode/CollisionDetector.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CollisionDetector.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive17CollisionDetectorE_t {};
} // unnamed namespace

template <> constexpr inline auto PhantomDrive::CollisionDetector::qt_create_metaobjectdata<qt_meta_tag_ZN12PhantomDrive17CollisionDetectorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PhantomDrive::CollisionDetector",
        "collisionDetected",
        "",
        "objectId",
        "QVector2D",
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
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'collisionDetected'
        QtMocHelpers::SignalData<void(const QString &, const QVector2D &, qreal)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { 0x80000000 | 4, 5 }, { QMetaType::QReal, 6 },
        }}),
        // Signal 'collisionEnded'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'nearMissDetected'
        QtMocHelpers::SignalData<void(const QString &, qreal)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::QReal, 9 },
        }}),
        // Signal 'collisionDataRecorded'
        QtMocHelpers::SignalData<void(const DrivingData &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Signal 'detectorEnabled'
        QtMocHelpers::SignalData<void(bool)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 14 },
        }}),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 },
        }}),
        // Slot 'updateVehiclePosition'
        QtMocHelpers::SlotData<void(const QVector2D &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Slot 'updateVehicleRotation'
        QtMocHelpers::SlotData<void(qreal)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 19 },
        }}),
        // Slot 'checkCollisions'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CollisionDetector, qt_meta_tag_ZN12PhantomDrive17CollisionDetectorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PhantomDrive::CollisionDetector::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive17CollisionDetectorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive17CollisionDetectorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12PhantomDrive17CollisionDetectorE_t>.metaTypes,
    nullptr
} };

void PhantomDrive::CollisionDetector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CollisionDetector *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->collisionDetected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QVector2D>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<qreal>>(_a[3]))); break;
        case 1: _t->collisionEnded((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->nearMissDetected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qreal>>(_a[2]))); break;
        case 3: _t->collisionDataRecorded((*reinterpret_cast<std::add_pointer_t<DrivingData>>(_a[1]))); break;
        case 4: _t->detectorEnabled((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->errorOccurred((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->updateVehiclePosition((*reinterpret_cast<std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 7: _t->updateVehicleRotation((*reinterpret_cast<std::add_pointer_t<qreal>>(_a[1]))); break;
        case 8: _t->checkCollisions(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CollisionDetector::*)(const QString & , const QVector2D & , qreal )>(_a, &CollisionDetector::collisionDetected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollisionDetector::*)(const QString & )>(_a, &CollisionDetector::collisionEnded, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollisionDetector::*)(const QString & , qreal )>(_a, &CollisionDetector::nearMissDetected, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollisionDetector::*)(const DrivingData & )>(_a, &CollisionDetector::collisionDataRecorded, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollisionDetector::*)(bool )>(_a, &CollisionDetector::detectorEnabled, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (CollisionDetector::*)(const QString & )>(_a, &CollisionDetector::errorOccurred, 5))
            return;
    }
}

const QMetaObject *PhantomDrive::CollisionDetector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::CollisionDetector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive17CollisionDetectorE_t>.strings))
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
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3);
}

// SIGNAL 1
void PhantomDrive::CollisionDetector::collisionEnded(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void PhantomDrive::CollisionDetector::nearMissDetected(const QString & _t1, qreal _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void PhantomDrive::CollisionDetector::collisionDataRecorded(const DrivingData & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void PhantomDrive::CollisionDetector::detectorEnabled(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void PhantomDrive::CollisionDetector::errorOccurred(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}
QT_WARNING_POP
