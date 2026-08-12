#pragma once
#include <QString>
#include <QVector>
#include <QDate>
#include "InvoiceModel.h"

// Search/filter criteria used by the dashboard table.
struct InvoiceFilter {
    QString searchText;   // matches phone, name, CIN, or invoice number
    double  amountMin = -1.0;
    double  amountMax = -1.0;
    QDate   dateFrom;
};

struct DashboardStats {
    QDate  lastInvoiceDate;
    double totalAmount = 0.0;
    int    invoiceCount = 0;
};

// Thin wrapper around a local SQLite database (QtSql).
class Database
{
public:
    bool init(const QString &dbPath = QString());

    bool saveInvoice(Invoice &invoice); // fills invoice.id on success
    QVector<Invoice> listInvoices(const InvoiceFilter &filter = {}) const;
    DashboardStats stats() const;
    bool deleteInvoice(int invoiceId);
    bool loadInvoice(int invoiceId, Invoice &out) const;

    QString lastError() const { return m_lastError; }

private:
    bool createSchema();
    mutable QString m_lastError;
    QString m_connectionName = "invoice_extractor_conn";
};
