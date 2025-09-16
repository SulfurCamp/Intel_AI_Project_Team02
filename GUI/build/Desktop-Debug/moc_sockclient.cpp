/****************************************************************************
** Meta object code from reading C++ file 'sockclient.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../sockclient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sockclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SockClient_t {
    QByteArrayData data[16];
    char stringdata0[223];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SockClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SockClient_t qt_meta_stringdata_SockClient = {
    {
QT_MOC_LITERAL(0, 0, 10), // "SockClient"
QT_MOC_LITERAL(1, 11, 17), // "socketRevcDataSig"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 12), // "connectedSig"
QT_MOC_LITERAL(4, 43, 15), // "disconnectedSig"
QT_MOC_LITERAL(5, 59, 8), // "errorSig"
QT_MOC_LITERAL(6, 68, 18), // "socketReadDataSlot"
QT_MOC_LITERAL(7, 87, 15), // "socketErrorSlot"
QT_MOC_LITERAL(8, 103, 23), // "socketConnectServerSlot"
QT_MOC_LITERAL(9, 127, 19), // "connectToServerSlot"
QT_MOC_LITERAL(10, 147, 5), // "bool&"
QT_MOC_LITERAL(11, 153, 18), // "connectWithAddress"
QT_MOC_LITERAL(12, 172, 2), // "ip"
QT_MOC_LITERAL(13, 175, 4), // "port"
QT_MOC_LITERAL(14, 180, 22), // "socketClosedServerSlot"
QT_MOC_LITERAL(15, 203, 19) // "socketWriteDataSlot"

    },
    "SockClient\0socketRevcDataSig\0\0"
    "connectedSig\0disconnectedSig\0errorSig\0"
    "socketReadDataSlot\0socketErrorSlot\0"
    "socketConnectServerSlot\0connectToServerSlot\0"
    "bool&\0connectWithAddress\0ip\0port\0"
    "socketClosedServerSlot\0socketWriteDataSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SockClient[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       3,    0,   72,    2, 0x06 /* Public */,
       4,    0,   73,    2, 0x06 /* Public */,
       5,    1,   74,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   77,    2, 0x08 /* Private */,
       7,    0,   78,    2, 0x08 /* Private */,
       8,    0,   79,    2, 0x08 /* Private */,
       9,    1,   80,    2, 0x0a /* Public */,
      11,    2,   83,    2, 0x0a /* Public */,
      14,    0,   88,    2, 0x0a /* Public */,
      15,    1,   89,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::UShort,   12,   13,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,

       0        // eod
};

void SockClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SockClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->socketRevcDataSig((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->connectedSig(); break;
        case 2: _t->disconnectedSig(); break;
        case 3: _t->errorSig((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->socketReadDataSlot(); break;
        case 5: _t->socketErrorSlot(); break;
        case 6: _t->socketConnectServerSlot(); break;
        case 7: _t->connectToServerSlot((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->connectWithAddress((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 9: _t->socketClosedServerSlot(); break;
        case 10: _t->socketWriteDataSlot((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SockClient::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SockClient::socketRevcDataSig)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SockClient::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SockClient::connectedSig)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SockClient::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SockClient::disconnectedSig)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SockClient::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SockClient::errorSig)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SockClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_SockClient.data,
    qt_meta_data_SockClient,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SockClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SockClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SockClient.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SockClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void SockClient::socketRevcDataSig(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SockClient::connectedSig()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SockClient::disconnectedSig()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SockClient::errorSig(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
