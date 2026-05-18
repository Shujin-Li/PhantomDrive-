/****************************************************************************
** Meta object code from reading C++ file 'IDrivingDataCollector.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../include/PhantomDrive/gamemode/IDrivingDataCollector.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IDrivingDataCollector.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive21IDrivingDataCollectorE_t {};
} // unnamed namespace

template <> constexpr inline auto PhantomDrive::IDrivingDataCollector::qt_create_metaobjectdata<qt_meta_tag_ZN12PhantomDrive21IDrivingDataCollectorE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PhantomDrive::IDrivingDataCollector",
        "dataCollected",
        "",
        "DrivingData",
        "data",
        "violationDetected",
        "ViolationEvent",
        "violation",
        "collectionStarted",
        "collectionStopped",
        "dataBatchReady",
        "QList<DrivingData>",
        "batch"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'dataCollected'
        QtMocHelpers::SignalData<void(const DrivingData &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'violationDetected'
        QtMocHelpers::SignalData<void(const ViolationEvent &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
        // Signal 'collectionStarted'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'collectionStopped'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'dataBatchReady'
        QtMocHelpers::SignalData<void(const QList<DrivingData> &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<IDrivingDataCollector, qt_meta_tag_ZN12PhantomDrive21IDrivingDataCollectorE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PhantomDrive::IDrivingDataCollector::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive21IDrivingDataCollectorE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive21IDrivingDataCollectorE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12PhantomDrive21IDrivingDataCollectorE_t>.metaTypes,
    nullptr
} };

void PhantomDrive::IDrivingDataCollector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<IDrivingDataCollector *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->dataCollected((*reinterpret_cast<std::add_pointer_t<DrivingData>>(_a[1]))); break;
        case 1: _t->violationDetected((*reinterpret_cast<std::add_pointer_t<ViolationEvent>>(_a[1]))); break;
        case 2: _t->collectionStarted(); break;
        case 3: _t->collectionStopped(); break;
        case 4: _t->dataBatchReady((*reinterpret_cast<std::add_pointer_t<QList<DrivingData>>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (IDrivingDataCollector::*)(const DrivingData & )>(_a, &IDrivingDataCollector::dataCollected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (IDrivingDataCollector::*)(const ViolationEvent & )>(_a, &IDrivingDataCollector::violationDetected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (IDrivingDataCollector::*)()>(_a, &IDrivingDataCollector::collectionStarted, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (IDrivingDataCollector::*)()>(_a, &IDrivingDataCollector::collectionStopped, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (IDrivingDataCollector::*)(const QList<DrivingData> & )>(_a, &IDrivingDataCollector::dataBatchReady, 4))
            return;
    }
}

const QMetaObject *PhantomDrive::IDrivingDataCollector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::IDrivingDataCollector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12PhantomDrive21IDrivingDataCollectorE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::IDrivingDataCollector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::IDrivingDataCollector::dataCollected(const DrivingData & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void PhantomDrive::IDrivingDataCollector::violationDetected(const ViolationEvent & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void PhantomDrive::IDrivingDataCollector::collectionStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PhantomDrive::IDrivingDataCollector::collectionStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PhantomDrive::IDrivingDataCollector::dataBatchReady(const QList<DrivingData> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}
QT_WARNING_POP
