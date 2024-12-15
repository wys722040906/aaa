/****************************************************************************
** Meta object code from reading C++ file 'Elevator.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../app/include/core/Elevator.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Elevator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Elevator_t {
    QByteArrayData data[15];
    char stringdata0[178];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Elevator_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Elevator_t qt_meta_stringdata_Elevator = {
    {
QT_MOC_LITERAL(0, 0, 8), // "Elevator"
QT_MOC_LITERAL(1, 9, 12), // "stateChanged"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 5), // "State"
QT_MOC_LITERAL(4, 29, 8), // "newState"
QT_MOC_LITERAL(5, 38, 12), // "floorChanged"
QT_MOC_LITERAL(6, 51, 8), // "newFloor"
QT_MOC_LITERAL(7, 60, 10), // "doorOpened"
QT_MOC_LITERAL(8, 71, 10), // "doorClosed"
QT_MOC_LITERAL(9, 82, 10), // "overloaded"
QT_MOC_LITERAL(10, 93, 19), // "malfunctionOccurred"
QT_MOC_LITERAL(11, 113, 15), // "handleEmergency"
QT_MOC_LITERAL(12, 129, 19), // "simulateMalfunction"
QT_MOC_LITERAL(13, 149, 12), // "addPassenger"
QT_MOC_LITERAL(14, 162, 15) // "removePassenger"

    },
    "Elevator\0stateChanged\0\0State\0newState\0"
    "floorChanged\0newFloor\0doorOpened\0"
    "doorClosed\0overloaded\0malfunctionOccurred\0"
    "handleEmergency\0simulateMalfunction\0"
    "addPassenger\0removePassenger"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Elevator[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x06 /* Public */,
       5,    1,   67,    2, 0x06 /* Public */,
       7,    0,   70,    2, 0x06 /* Public */,
       8,    0,   71,    2, 0x06 /* Public */,
       9,    0,   72,    2, 0x06 /* Public */,
      10,    0,   73,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   74,    2, 0x0a /* Public */,
      12,    0,   75,    2, 0x0a /* Public */,
      13,    0,   76,    2, 0x0a /* Public */,
      14,    0,   77,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Elevator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Elevator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast< State(*)>(_a[1]))); break;
        case 1: _t->floorChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->doorOpened(); break;
        case 3: _t->doorClosed(); break;
        case 4: _t->overloaded(); break;
        case 5: _t->malfunctionOccurred(); break;
        case 6: _t->handleEmergency(); break;
        case 7: _t->simulateMalfunction(); break;
        case 8: _t->addPassenger(); break;
        case 9: _t->removePassenger(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Elevator::*)(State );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Elevator::stateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Elevator::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Elevator::floorChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Elevator::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Elevator::doorOpened)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Elevator::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Elevator::doorClosed)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Elevator::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Elevator::overloaded)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (Elevator::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Elevator::malfunctionOccurred)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Elevator::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Elevator.data,
    qt_meta_data_Elevator,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Elevator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Elevator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Elevator.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Elevator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Elevator::stateChanged(State _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Elevator::floorChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Elevator::doorOpened()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Elevator::doorClosed()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Elevator::overloaded()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void Elevator::malfunctionOccurred()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
