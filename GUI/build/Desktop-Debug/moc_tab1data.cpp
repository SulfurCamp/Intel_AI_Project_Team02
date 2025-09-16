/****************************************************************************
** Meta object code from reading C++ file 'tab1data.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../tab1data.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tab1data.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Tab1Data_t {
    QByteArrayData data[17];
    char stringdata0[196];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Tab1Data_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Tab1Data_t qt_meta_stringdata_Tab1Data = {
    {
QT_MOC_LITERAL(0, 0, 8), // "Tab1Data"
QT_MOC_LITERAL(1, 9, 17), // "menuButtonClicked"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 13), // "droneDetected"
QT_MOC_LITERAL(4, 42, 1), // "x"
QT_MOC_LITERAL(5, 44, 1), // "y"
QT_MOC_LITERAL(6, 46, 1), // "z"
QT_MOC_LITERAL(7, 48, 19), // "on_pPBQuery_clicked"
QT_MOC_LITERAL(8, 68, 27), // "on_pPBServerConnect_toggled"
QT_MOC_LITERAL(9, 96, 7), // "checked"
QT_MOC_LITERAL(10, 104, 18), // "on_pPBSend_clicked"
QT_MOC_LITERAL(11, 123, 10), // "onRecvLine"
QT_MOC_LITERAL(12, 134, 4), // "line"
QT_MOC_LITERAL(13, 139, 17), // "onSocketConnected"
QT_MOC_LITERAL(14, 157, 20), // "onSocketDisconnected"
QT_MOC_LITERAL(15, 178, 13), // "onSocketError"
QT_MOC_LITERAL(16, 192, 3) // "msg"

    },
    "Tab1Data\0menuButtonClicked\0\0droneDetected\0"
    "x\0y\0z\0on_pPBQuery_clicked\0"
    "on_pPBServerConnect_toggled\0checked\0"
    "on_pPBSend_clicked\0onRecvLine\0line\0"
    "onSocketConnected\0onSocketDisconnected\0"
    "onSocketError\0msg"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Tab1Data[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,
       3,    3,   60,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   67,    2, 0x08 /* Private */,
       8,    1,   68,    2, 0x08 /* Private */,
      10,    0,   71,    2, 0x08 /* Private */,
      11,    1,   72,    2, 0x08 /* Private */,
      13,    0,   75,    2, 0x08 /* Private */,
      14,    0,   76,    2, 0x08 /* Private */,
      15,    1,   77,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double,    4,    5,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   16,

       0        // eod
};

void Tab1Data::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Tab1Data *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->menuButtonClicked(); break;
        case 1: _t->droneDetected((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 2: _t->on_pPBQuery_clicked(); break;
        case 3: _t->on_pPBServerConnect_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->on_pPBSend_clicked(); break;
        case 5: _t->onRecvLine((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->onSocketConnected(); break;
        case 7: _t->onSocketDisconnected(); break;
        case 8: _t->onSocketError((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Tab1Data::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Tab1Data::menuButtonClicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Tab1Data::*)(double , double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Tab1Data::droneDetected)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Tab1Data::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Tab1Data.data,
    qt_meta_data_Tab1Data,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Tab1Data::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Tab1Data::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Tab1Data.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Tab1Data::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void Tab1Data::menuButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Tab1Data::droneDetected(double _t1, double _t2, double _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
