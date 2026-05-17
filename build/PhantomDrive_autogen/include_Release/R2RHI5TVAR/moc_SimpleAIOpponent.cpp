/****************************************************************************
** Meta object code from reading C++ file 'SimpleAIOpponent.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/PhantomDrive/gamemode/SimpleAIOpponent.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SimpleAIOpponent.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12PhantomDrive16SimpleAIOpponentE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12PhantomDrive16SimpleAIOpponentE = QtMocHelpers::stringData(
    "PhantomDrive::SimpleAIOpponent"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12PhantomDrive16SimpleAIOpponentE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject PhantomDrive::SimpleAIOpponent::staticMetaObject = { {
    QMetaObject::SuperData::link<AIOpponent::staticMetaObject>(),
    qt_meta_stringdata_ZN12PhantomDrive16SimpleAIOpponentE.offsetsAndSizes,
    qt_meta_data_ZN12PhantomDrive16SimpleAIOpponentE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12PhantomDrive16SimpleAIOpponentE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SimpleAIOpponent, std::true_type>
    >,
    nullptr
} };

void PhantomDrive::SimpleAIOpponent::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SimpleAIOpponent *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *PhantomDrive::SimpleAIOpponent::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhantomDrive::SimpleAIOpponent::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12PhantomDrive16SimpleAIOpponentE.stringdata0))
        return static_cast<void*>(this);
    return AIOpponent::qt_metacast(_clname);
}

int PhantomDrive::SimpleAIOpponent::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AIOpponent::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
