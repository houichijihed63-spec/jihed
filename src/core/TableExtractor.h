#pragma once
#include <QVector>
#include "OcrClient.h"
#include "InvoiceModel.h"

// Rebuilds the invoice's items table from raw OCR words + coordinates.
// This is the step that fixes wrong totals: instead of trusting a single
// OCR-read "Total" line, every column is reconstructed from word positions.
namespace TableExtractor {

QVector<LineItem> extract(const QVector<OcrWord> &words);

} // namespace TableExtractor
