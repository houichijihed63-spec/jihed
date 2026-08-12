#pragma once
#include <QString>

// Cleans up raw OCR text: removes stray bullets, collapses whitespace,
// and fixes a set of OCR misreads that are common on French/Tunisian invoices.
namespace TextNormalizer {

QString normalize(const QString &rawText);

} // namespace TextNormalizer
