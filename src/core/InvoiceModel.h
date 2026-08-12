#pragma once
#include <QString>
#include <QVector>
#include <QDate>

// A single row extracted from the invoice's items table
struct LineItem {
    QString description;
    double  quantity   = 0.0;
    QString unit;
    double  unitPrice  = 0.0;
    double  total      = 0.0;
};

// Full invoice, built from OCR + extraction + calculation
struct Invoice {
    int      id = -1;               // -1 = not yet saved in DB
    QString  invoiceNumber;
    QDate    invoiceDate;
    QString  supplierName;
    QString  supplierAddress;
    QString  clientName;
    QString  clientAddress;
    QString  clientPhone;
    QString  clientCin;
    QString  paymentTerms;

    QVector<LineItem> lineItems;

    double   subtotal   = 0.0;
    double   taxRate     = 19.0;
    double   taxAmount   = 0.0;
    double   totalAmount = 0.0;
    QString  currency    = "TND";

    QString  sourceImagePath;
};
