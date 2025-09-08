/****************************************************************************
** Meta object code from reading C++ file 'tab1data.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../tab1data.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tab1data.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8Tab1DataE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN8Tab1DataE = QtMocHelpers::stringData(
    "Tab1Data",
    "on_pPBQuery_clicked",
    "",
    "on_pPBServerConnect_toggled",
    "checked",
    "on_pPBSend_clicked",
    "onRecvLine",
    "line",
    "onSocketConnected",
    "onSocketDisconnected",
    "onSocketError",
    "msg"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN8Tab1DataE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x08,    1 /* Private */,
       3,    1,   57,    2, 0x08,    2 /* Private */,
       5,    0,   60,    2, 0x08,    4 /* Private */,
       6,    1,   61,    2, 0x08,    5 /* Private */,
       8,    0,   64,    2, 0x08,    7 /* Private */,
       9,    0,   65,    2, 0x08,    8 /* Private */,
      10,    1,   66,    2, 0x08,    9 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,

       0        // eod
};

Q_CONSTINIT const QMetaObject Tab1Data::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN8Tab1DataE.offsetsAndSizes,
    qt_meta_data_ZN8Tab1DataE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN8Tab1DataE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Tab1Data, std::true_type>,
        // method 'on_pPBQuery_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_pPBServerConnect_toggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'on_pPBSend_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRecvLine'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'onSocketConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSocketDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSocketError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>
    >,
    nullptr
} };

void Tab1Data::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Tab1Data *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->on_pPBQuery_clicked(); break;
        case 1: _t->on_pPBServerConnect_toggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->on_pPBSend_clicked(); break;
        case 3: _t->onRecvLine((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->onSocketConnected(); break;
        case 5: _t->onSocketDisconnected(); break;
        case 6: _t->onSocketError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *Tab1Data::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Tab1Data::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN8Tab1DataE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Tab1Data::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
