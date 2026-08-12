#include "FieldExtractor.h"
#include <QRegularExpression>
#include <QStringList>

namespace {

QString firstCapture(const QString &text, const QRegularExpression &re)
{
    const QRegularExpressionMatch m = re.match(text);
    return m.hasMatch() ? m.captured(1).trimmed() : QString();
}

QDate parseDate(const QString &text)
{
    // dd/mm/yyyy or dd-mm-yyyy
    static const QRegularExpression reDMY(R"((\d{1,2})[\/\-](\d{1,2})[\/\-](\d{4}))");
    // yyyy-mm-dd
    static const QRegularExpression reYMD(R"((\d{4})[\/\-](\d{1,2})[\/\-](\d{1,2}))");

    QRegularExpressionMatch m = reDMY.match(text);
    if (m.hasMatch()) {
        return QDate(m.captured(3).toInt(), m.captured(2).toInt(), m.captured(1).toInt());
    }
    m = reYMD.match(text);
    if (m.hasMatch()) {
        return QDate(m.captured(1).toInt(), m.captured(2).toInt(), m.captured(3).toInt());
    }
    return QDate();
}

QString extractLineAfterKeyword(const QStringList &lines, const QStringList &keywords)
{
    for (int i = 0; i < lines.size(); ++i) {
        for (const QString &kw : keywords) {
            if (lines[i].contains(kw, Qt::CaseInsensitive)) {
                // Prefer text on the same line, after the keyword.
                const int idx = lines[i].indexOf(kw, 0, Qt::CaseInsensitive);
                QString rest = lines[i].mid(idx + kw.size()).trimmed();
                rest.remove(QRegularExpression("^[:\\-\\s]+"));
                if (!rest.isEmpty())
                    return rest;
                // Otherwise take the next non-empty line.
                if (i + 1 < lines.size())
                    return lines[i + 1].trimmed();
            }
        }
    }
    return QString();
}

} // namespace

namespace FieldExtractor {

void extract(const QString &normalizedText, Invoice &invoice)
{
    const QStringList lines = normalizedText.split('\n', Qt::SkipEmptyParts);

    // --- Invoice number: "Facture N°", "N°", "Facture" followed by a code ---
    {
        static const QRegularExpression re(
            R"((?:Facture\s*N°|N°|Facture)\s*[:\-]?\s*([A-Za-z0-9\-\/]{2,}))",
            QRegularExpression::CaseInsensitiveOption);
        invoice.invoiceNumber = firstCapture(normalizedText, re);
    }

    // --- Invoice date ---
    invoice.invoiceDate = parseDate(normalizedText);

    // --- Supplier name: line containing Société / SARL / SA ---
    {
        static const QStringList kSupplierMarkers = {"Société", "SARL", "SA", "Ste"};
        for (const QString &line : lines) {
            for (const QString &marker : kSupplierMarkers) {
                if (line.contains(marker, Qt::CaseInsensitive)) {
                    invoice.supplierName = line.trimmed();
                    break;
                }
            }
            if (!invoice.supplierName.isEmpty())
                break;
        }
    }

    // --- Client section ---
    invoice.clientName = extractLineAfterKeyword(
        lines, {"Informations client", "Client"});

    // --- Client phone: sequences of digits/spaces resembling a phone number ---
    {
        static const QRegularExpression re(R"((?:\+?216)?\s?\d{2}\s?\d{3}\s?\d{3})");
        const QRegularExpressionMatch m = re.match(normalizedText);
        if (m.hasMatch())
            invoice.clientPhone = m.captured(0).trimmed();
    }

    // --- Payment terms ---
    invoice.paymentTerms = extractLineAfterKeyword(
        lines, {"Conditions de paiement", "Mode de paiement", "Paiement"});

    // --- Tax rate placeholder is resolved by InvoiceCalculator::extractTaxRate ---
}

} // namespace FieldExtractor
