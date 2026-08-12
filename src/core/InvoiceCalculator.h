#pragma once
#include <QString>
#include "InvoiceModel.h"

namespace InvoiceCalculator {

// Parses a French/Tunisian formatted number into a plain double.
// Handles:
//   - spaces as thousands separators   "21 500,000" -> 21500.000
//   - comma as decimal separator        "2,15"       -> 2.15
//   - dots as thousands separators      "1.250,50"   -> 1250.50
//   - plain numbers                     "2150"       -> 2150
double cleanNumber(const QString &raw);

// Looks for "TVA 19%" / "TVA" style patterns in the normalized text.
// Returns -1.0 if nothing was found (caller should fall back to a default).
double extractTaxRate(const QString &normalizedText);

// Recomputes subtotal / tax / total from the line items instead of trusting
// the totals printed on the invoice (which OCR frequently misreads).
void computeTotals(Invoice &invoice);

} // namespace InvoiceCalculator
