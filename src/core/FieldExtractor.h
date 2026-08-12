#pragma once
#include "InvoiceModel.h"

// Extracts the "header" fields of the invoice (number, date, supplier,
// client info, payment terms) from the normalized OCR text using
// pattern matching. Line items are handled separately by TableExtractor.
namespace FieldExtractor {

void extract(const QString &normalizedText, Invoice &invoice);

} // namespace FieldExtractor
