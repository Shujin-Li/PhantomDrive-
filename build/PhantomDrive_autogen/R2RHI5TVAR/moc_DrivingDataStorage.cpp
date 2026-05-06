/****************************************************************************
** Meta object code from reading C++ file 'DrivingDataStorage.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../include/PhantomDrive/gamemode/DrivingDataStorage.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DrivingDataStorage.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive18DrivingDataStorageE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive18DrivingDataStorageE = QtMocHelpers::stringData(
    "PhantomDrive::DrivingDataStorage",
    "dataAdded",
    "",
    "DrivingData",
    "data",
    "dataBatchAdded",
    "count",
    "bufferFull",
    "bufferCleared",
    "violationAdded",
    "ViolationEvent",
    "violation",
    "exportCompleted",
    "filePath",
    "importCompleted",
    "errorOccurred",
    "error"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive18DrivingDataStorageE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   62,    2, 0x06,    1 /* Public */,
       5,    1,   65,    2, 0x06,    3 /* Public */,
       7,    0,   68,    2, 0x06,    5 /* Public */,
       8,    0,   69,    2, 0x06,    6 /* Public */,
       9,    1,   70,    2, 0x06,    7 /* Public */,
      12,    1,   73,    2, 0x06,    9 /* Public */,
      14,    1,   76,    2, 0x06,   11 /* Public */,
      15,    1,   79,    2, 0x06,   13 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   16,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::DrivingDataStorage::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive18DrivingDataStorageE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive18DrivingDataStorageE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive18DrivingDataStorageE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DrivingDataStorage, std::true_type>,
        // method 'dataAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DrivingData &, std::false_type>,
        // method 'dataBatchAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'bufferFull'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'bufferCleared'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'violationAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const ViolationEvent &, std::false_type>,
        // method 'exportCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'importCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void PhantomDrive::DrivingDataStorage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DrivingDataStorage *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->dataAdded((*reinterpret_cast< std::add_pointer_t<DrivingData>>(_a[1]))); break;
        case 1: _t->dataBatchAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->bufferFull(); break;
        case 3: _t->bufferCleared(); break;
        case 4: _t->violationAdded((*reinterpret_cast< std::add_pointer_t<ViolationEvent>>(_a[1]))); break;
        case 5: _t->exportCompleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->importCompleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (DrivingDataStorage::*)(const DrivingData & );
            if (_q_method_type _q_method = &DrivingDataStorage::dataAdded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataStorage::*)(int );
            if (_q_method_type _q_method = &DrivingDataStorage::dataBatchAdded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataStorage::*)();
            if (_q_method_type _q_method = &DrivingDataStorage::bufferFull; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataStorage::*)();
            if (_q_method_type _q_method = &DrivingDataStorage::bufferCleared; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataStorage::*)(const ViolationEvent & );
            if (_q_method_type _q_method = &DrivingDataStorage::violationAdded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataStorage::*)(const QString & );
            if (_q_method_type _q_method = &DrivingDataStorage::exportCompleted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataStorage::*)(const QString & );
            if (_q_method_type _q_method = &DrivingDataStorage::importCompleted; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (DrivingDataStorage::*)(const QString & );
            if (_q_method_type _q_method = &DrivingDataStorage::errorOccurred; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *PhantomDrive::DrivingDataStorage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::DrivingDataStorage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive18DrivingDataStorageE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PhantomDrive::DrivingDataStorage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void PhantomDrive::DrivingDataStorage::dataAdded(const DrivingData & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PhantomDrive::DrivingDataStorage::dataBatchAdded(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PhantomDrive::DrivingDataStorage::bufferFull()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PhantomDrive::DrivingDataStorage::bufferCleared()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PhantomDrive::DrivingDataStorage::violationAdded(const ViolationEvent & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PhantomDrive::DrivingDataStorage::exportCompleted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void PhantomDrive::DrivingDataStorage::importCompleted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PhantomDrive::DrivingDataStorage::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
