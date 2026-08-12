#include "InvoiceCalculator.h"
#include <QRegularExpression>

namespace InvoiceCalculator {

double cleanNumber(const QString &raw)
{
    QString s = raw.trimmed();
    if (s.isEmpty())
        return 0.0;

    // Remove currency / stray characters, keep digits, spaces, dots, commas.
    s.remove(QRegularExpression("[^0-9.,\\s]"));
    s = s.simplified(); // collapse internal whitespace to single spaces

    const bool hasComma = s.contains(',');
    const bool hasDot   = s.contains('.');
    const bool hasSpace = s.contains(' ');

    QString normalized = s;

    if (hasComma && hasDot) {
        // "1.250,50" -> dots are thousand separators, comma is decimal
        normalized.remove('.');
        normalized.replace(',', '.');
    } else if (hasComma && !hasDot) {
        // "2,15" (decimal) or "5 991,250" (thousands via space + decimal comma)
        normalized.remove(' ');
        normalized.replace(',', '.');
    } else if (hasSpace && !hasComma && !hasDot) {
        // "21 500" -> thousands separated by spaces, no decimals
        normalized.remove(' ');
    } else if (hasDot && !hasComma) {
        // Ambiguous: could be "1.250" (thousands) or "2.15" (decimal).
        // Heuristic: if exactly 3 digits follow the last dot, treat as thousands.
        const int lastDot = normalized.lastIndexOf('.');
        const int digitsAfter = normalized.size() - lastDot - 1;
        if (digitsAfter == 3)
            normalized.remove('.');
        // otherwise leave the dot as a decimal point
    }

    normalized.remove(' ');
    bool ok = false;
    const double value = normalized.toDouble(&ok);
    return ok ? value : 0.0;
}

double extractTaxRate(const QString &normalizedText)
{
    static const QRegularExpression re(
        R"(TVA\D{0,10}(\d{1,2})(?:[.,]\d+)?\s*%)",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch m = re.match(normalizedText);
    if (m.hasMatch())
        return m.captured(1).toDouble();
    return -1.0;
}

void computeTotals(Invoice &invoice)
{
    double subtotal = 0.0;
    for (const LineItem &item : invoice.lineItems)
        subtotal += item.total;

    invoice.subtotal   = subtotal;
    invoice.taxAmount   = subtotal * (invoice.taxRate / 100.0);
    invoice.totalAmount = subtotal + invoice.taxAmount;
}

} // namespace InvoiceCalculator
