#include "TextNormalizer.h"
#include <QRegularExpression>
#include <QMap>

namespace TextNormalizer {

QString normalize(const QString &rawText)
{
    QString text = rawText;

    // Remove bullet-style separators produced by OCR ("•", "·", "●")
    text.replace(QRegularExpression("[•·●]"), " ");

    // Common OCR misreads seen on French/Tunisian invoices.
    // Order matters: longer/more specific patterns first.
    static const QVector<QPair<QString, QString>> kFixes = {
        {"Chent", "Client"},
        {"CIient", "Client"},
        {"TuniSiO", "Tunisie"},
        {"TuniSie", "Tunisie"},
        {"Töles", "Tôles"},
        {"T0les", "Tôles"},
        {"Factu re", "Facture"},
        {"N °", "N°"},
        {"N0", "N°"},
    };
    for (const auto &fix : kFixes)
        text.replace(fix.first, fix.second, Qt::CaseInsensitive);

    // Collapse repeated whitespace / blank lines, trim each line.
    QStringList lines = text.split('\n');
    QStringList cleanedLines;
    for (QString line : lines) {
        line = line.simplified(); // collapses internal whitespace, trims ends
        if (!line.isEmpty())
            cleanedLines << line;
    }

    return cleanedLines.join('\n');
}

} // namespace TextNormalizer
