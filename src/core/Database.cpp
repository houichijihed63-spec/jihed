#include "Database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QVariant>

bool Database::init(const QString &dbPath)
{
    QString path = dbPath;
    if (path.isEmpty()) {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        path = dir + "/invoices.db";
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(path);
    if (!db.open()) {
        m_lastError = db.lastError().text();
        return false;
    }

    return createSchema();
}

bool Database::createSchema()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);

    const bool ok1 = query.exec(R"(
        CREATE TABLE IF NOT EXISTS invoices (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            invoice_number  TEXT,
            invoice_date    TEXT,
            supplier_name   TEXT,
            supplier_address TEXT,
            client_name     TEXT,
            client_address  TEXT,
            client_phone    TEXT,
            client_cin      TEXT,
            payment_terms   TEXT,
            subtotal        REAL,
            tax_rate        REAL,
            tax_amount      REAL,
            total_amount    REAL,
            currency        TEXT,
            source_image    TEXT
        )
    )");

    const bool ok2 = query.exec(R"(
        CREATE TABLE IF NOT EXISTS line_items (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            invoice_id  INTEGER,
            description TEXT,
            quantity    REAL,
            unit        TEXT,
            unit_price  REAL,
            total       REAL,
            FOREIGN KEY(invoice_id) REFERENCES invoices(id)
        )
    )");

    if (!ok1 || !ok2)
        m_lastError = query.lastError().text();

    return ok1 && ok2;
}

bool Database::saveInvoice(Invoice &invoice)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO invoices
            (invoice_number, invoice_date, supplier_name, supplier_address,
             client_name, client_address, client_phone, client_cin,
             payment_terms, subtotal, tax_rate, tax_amount, total_amount,
             currency, source_image)
        VALUES
            (:num, :date, :supName, :supAddr, :cliName, :cliAddr, :cliPhone,
             :cliCin, :terms, :subtotal, :taxRate, :taxAmount, :total,
             :currency, :image)
    )");

    query.bindValue(":num", invoice.invoiceNumber);
    query.bindValue(":date", invoice.invoiceDate.isValid()
                                  ? invoice.invoiceDate.toString(Qt::ISODate)
                                  : QVariant(QString()));
    query.bindValue(":supName", invoice.supplierName);
    query.bindValue(":supAddr", invoice.supplierAddress);
    query.bindValue(":cliName", invoice.clientName);
    query.bindValue(":cliAddr", invoice.clientAddress);
    query.bindValue(":cliPhone", invoice.clientPhone);
    query.bindValue(":cliCin", invoice.clientCin);
    query.bindValue(":terms", invoice.paymentTerms);
    query.bindValue(":subtotal", invoice.subtotal);
    query.bindValue(":taxRate", invoice.taxRate);
    query.bindValue(":taxAmount", invoice.taxAmount);
    query.bindValue(":total", invoice.totalAmount);
    query.bindValue(":currency", invoice.currency);
    query.bindValue(":image", invoice.sourceImagePath);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        db.rollback();
        return false;
    }

    const int invoiceId = query.lastInsertId().toInt();
    invoice.id = invoiceId;

    QSqlQuery itemQuery(db);
    itemQuery.prepare(R"(
        INSERT INTO line_items (invoice_id, description, quantity, unit, unit_price, total)
        VALUES (:invId, :desc, :qty, :unit, :price, :total)
    )");
    for (const LineItem &item : invoice.lineItems) {
        itemQuery.bindValue(":invId", invoiceId);
        itemQuery.bindValue(":desc", item.description);
        itemQuery.bindValue(":qty", item.quantity);
        itemQuery.bindValue(":unit", item.unit);
        itemQuery.bindValue(":price", item.unitPrice);
        itemQuery.bindValue(":total", item.total);
        if (!itemQuery.exec()) {
            m_lastError = itemQuery.lastError().text();
            db.rollback();
            return false;
        }
    }

    db.commit();
    return true;
}

bool Database::deleteInvoice(int invoiceId)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();
    QSqlQuery q(db);
    q.prepare("DELETE FROM line_items WHERE invoice_id = :id");
    q.bindValue(":id", invoiceId);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        db.rollback();
        return false;
    }
    q.prepare("DELETE FROM invoices WHERE id = :id");
    q.bindValue(":id", invoiceId);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        db.rollback();
        return false;
    }
    db.commit();
    return true;
}

bool Database::loadInvoice(int invoiceId, Invoice &out) const
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);
    q.prepare(
        "SELECT id, invoice_number, invoice_date, supplier_name, supplier_address,"
        " client_name, client_address, client_phone, client_cin,"
        " payment_terms, subtotal, tax_rate, tax_amount, total_amount,"
        " currency, source_image"
        " FROM invoices"
        " WHERE id = :id"
    );
    q.bindValue(":id", invoiceId);
    if (!q.exec() || !q.next()) {
        const_cast<Database *>(this)->m_lastError = q.lastError().text();
        return false;
    }
    out.id = q.value("id").toInt();
    out.invoiceNumber = q.value("invoice_number").toString();
    out.invoiceDate = QDate::fromString(q.value("invoice_date").toString(), Qt::ISODate);
    out.supplierName = q.value("supplier_name").toString();
    out.supplierAddress = q.value("supplier_address").toString();
    out.clientName = q.value("client_name").toString();
    out.clientAddress = q.value("client_address").toString();
    out.clientPhone = q.value("client_phone").toString();
    out.clientCin = q.value("client_cin").toString();
    out.paymentTerms = q.value("payment_terms").toString();
    out.subtotal = q.value("subtotal").toDouble();
    out.taxRate = q.value("tax_rate").toDouble();
    out.taxAmount = q.value("tax_amount").toDouble();
    out.totalAmount = q.value("total_amount").toDouble();
    out.currency = q.value("currency").toString();
    out.sourceImagePath = q.value("source_image").toString();

    // load line items
    QSqlQuery iq(db);
    iq.prepare("SELECT description, quantity, unit, unit_price, total FROM line_items WHERE invoice_id = :id ORDER BY id");
    iq.bindValue(":id", invoiceId);
    if (iq.exec()) {
        out.lineItems.clear();
        while (iq.next()) {
            LineItem it;
            it.description = iq.value(0).toString();
            it.quantity = iq.value(1).toDouble();
            it.unit = iq.value(2).toString();
            it.unitPrice = iq.value(3).toDouble();
            it.total = iq.value(4).toDouble();
            out.lineItems.append(it);
        }
    }

    return true;
}

QVector<Invoice> Database::listInvoices(const InvoiceFilter &filter) const
{
    QVector<Invoice> result;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);

    QString sql = R"(
        SELECT id, invoice_number, invoice_date, client_name, client_phone,
               client_cin, total_amount, currency
        FROM invoices
        WHERE 1=1
    )";

    if (!filter.searchText.isEmpty()) {
        sql += " AND (client_name LIKE :search OR client_phone LIKE :search "
               "OR client_cin LIKE :search OR invoice_number LIKE :search) ";
    }
    if (filter.amountMin >= 0.0)
        sql += " AND total_amount >= :amountMin ";
    if (filter.amountMax >= 0.0)
        sql += " AND total_amount <= :amountMax ";
    if (filter.dateFrom.isValid())
        sql += " AND invoice_date >= :dateFrom ";

    sql += " ORDER BY invoice_date DESC, id DESC";

    QSqlQuery query(db);
    query.prepare(sql);
    if (!filter.searchText.isEmpty())
        query.bindValue(":search", "%" + filter.searchText + "%");
    if (filter.amountMin >= 0.0)
        query.bindValue(":amountMin", filter.amountMin);
    if (filter.amountMax >= 0.0)
        query.bindValue(":amountMax", filter.amountMax);
    if (filter.dateFrom.isValid())
        query.bindValue(":dateFrom", filter.dateFrom.toString(Qt::ISODate));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return result;
    }

    while (query.next()) {
        Invoice inv;
        inv.id             = query.value("id").toInt();
        inv.invoiceNumber  = query.value("invoice_number").toString();
        inv.invoiceDate    = QDate::fromString(query.value("invoice_date").toString(), Qt::ISODate);
        inv.clientName     = query.value("client_name").toString();
        inv.clientPhone    = query.value("client_phone").toString();
        inv.clientCin      = query.value("client_cin").toString();
        inv.totalAmount    = query.value("total_amount").toDouble();
        inv.currency       = query.value("currency").toString();
        result << inv;
    }
    return result;
}

DashboardStats Database::stats() const
{
    DashboardStats s;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);

    if (query.exec("SELECT COUNT(*), COALESCE(SUM(total_amount), 0), MAX(invoice_date) FROM invoices")
        && query.next()) {
        s.invoiceCount     = query.value(0).toInt();
        s.totalAmount       = query.value(1).toDouble();
        s.lastInvoiceDate   = QDate::fromString(query.value(2).toString(), Qt::ISODate);
    }
    return s;
}
