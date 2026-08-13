/****************************************************************************
** Meta object code from reading C++ file 'NewInvoicePage.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/ui/NewInvoicePage.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NewInvoicePage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
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
struct qt_meta_tag_ZN14NewInvoicePageE_t {};
} // unnamed namespace

template <> constexpr inline auto NewInvoicePage::qt_create_metaobjectdata<qt_meta_tag_ZN14NewInvoicePageE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "NewInvoicePage",
        "invoiceSaved",
        "",
        "onImageDropped",
        "filePath",
        "onImageInvalid",
        "reason",
        "onExtractClicked",
        "onOcrFinished",
        "OcrResult",
        "result",
        "onSaveClicked",
        "onClearClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'invoiceSaved'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onImageDropped'
        QtMocHelpers::SlotData<void(const QString &)>(3, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 4 },
        }}),
        // Slot 'onImageInvalid'
        QtMocHelpers::SlotData<void(const QString &)>(5, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Slot 'onExtractClicked'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onOcrFinished'
        QtMocHelpers::SlotData<void(const OcrResult &)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 9, 10 },
        }}),
        // Slot 'onSaveClicked'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onClearClicked'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<NewInvoicePage, qt_meta_tag_ZN14NewInvoicePageE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject NewInvoicePage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14NewInvoicePageE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14NewInvoicePageE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14NewInvoicePageE_t>.metaTypes,
    nullptr
} };

void NewInvoicePage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<NewInvoicePage *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->invoiceSaved(); break;
        case 1: _t->onImageDropped((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->onImageInvalid((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->onExtractClicked(); break;
        case 4: _t->onOcrFinished((*reinterpret_cast<std::add_pointer_t<OcrResult>>(_a[1]))); break;
        case 5: _t->onSaveClicked(); break;
        case 6: _t->onClearClicked(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (NewInvoicePage::*)()>(_a, &NewInvoicePage::invoiceSaved, 0))
            return;
    }
}

const QMetaObject *NewInvoicePage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NewInvoicePage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14NewInvoicePageE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int NewInvoicePage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void NewInvoicePage::invoiceSaved()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
