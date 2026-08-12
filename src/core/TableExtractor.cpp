#include "TableExtractor.h"
#include "InvoiceCalculator.h"
#include <QMap>
#include <algorithm>

namespace {

enum class Column { Description = 0, Quantity, Unit, UnitPrice, Total, Count };

struct ColumnAnchor {
    Column column;
    double x = -1.0; // left edge of the header word for this column
};

const QMap<Column, QStringList> &columnKeywords()
{
    static const QMap<Column, QStringList> kKeywords = {
        {Column::Description, {"Désignation", "Description", "Article", "Produit"}},
        {Column::Quantity,    {"Qté", "Qte", "Quantité", "Quantity"}},
        {Column::Unit,        {"Unité", "Unite", "Unit"}},
        {Column::UnitPrice,   {"P.U", "Prix", "Unitaire"}},
        {Column::Total,       {"Total", "Montant"}},
    };
    return kKeywords;
}

// Finds the left-most word matching each column's keyword list, restricted
// to a plausible "header band" (first ~15% of the document height).
QVector<ColumnAnchor> findHeaderAnchors(const QVector<OcrWord> &words, double &headerBottomY)
{
    double maxY = 0.0;
    for (const OcrWord &w : words)
        maxY = std::max(maxY, w.bottom());

    const double headerBandLimit = maxY * 0.35; // headers are usually in the upper part
    headerBottomY = 0.0;

    QVector<ColumnAnchor> anchors;
    for (auto it = columnKeywords().constBegin(); it != columnKeywords().constEnd(); ++it) {
        double bestX = -1.0;
        double bestY = -1.0;
        for (const OcrWord &w : words) {
            if (w.top > headerBandLimit)
                continue;
            for (const QString &kw : it.value()) {
                if (w.text.contains(kw, Qt::CaseInsensitive)) {
                    if (bestX < 0.0 || w.left < bestX) {
                        bestX = w.left;
                        bestY = w.bottom();
                    }
                }
            }
        }
        if (bestX >= 0.0) {
            anchors << ColumnAnchor{it.key(), bestX};
            headerBottomY = std::max(headerBottomY, bestY);
        }
    }

    std::sort(anchors.begin(), anchors.end(),
              [](const ColumnAnchor &a, const ColumnAnchor &b) { return a.x < b.x; });
    return anchors;
}

// Boundaries: midpoints between consecutive anchors. boundaries.size() == anchors.size() - 1
QVector<double> computeBoundaries(const QVector<ColumnAnchor> &anchors)
{
    QVector<double> boundaries;
    for (int i = 0; i + 1 < anchors.size(); ++i)
        boundaries << (anchors[i].x + anchors[i + 1].x) / 2.0;
    return boundaries;
}

Column columnForX(double centerX, const QVector<ColumnAnchor> &anchors, const QVector<double> &boundaries)
{
    int idx = 0;
    while (idx < boundaries.size() && centerX > boundaries[idx])
        ++idx;
    if (idx >= anchors.size())
        idx = anchors.size() - 1;
    return anchors[idx].column;
}

double medianHeight(const QVector<OcrWord> &words)
{
    if (words.isEmpty())
        return 20.0;
    QVector<double> heights;
    for (const OcrWord &w : words)
        heights << w.height;
    std::sort(heights.begin(), heights.end());
    return heights[heights.size() / 2];
}

} // namespace

namespace TableExtractor {

QVector<LineItem> extract(const QVector<OcrWord> &words)
{
    QVector<LineItem> items;
    if (words.isEmpty())
        return items;

    double headerBottomY = 0.0;
    QVector<ColumnAnchor> anchors = findHeaderAnchors(words, headerBottomY);

    // Need at least a description column and one numeric column to make sense of a table.
    if (anchors.size() < 2)
        return items;

    const QVector<double> boundaries = computeBoundaries(anchors);

    // Only consider words that sit below the header row.
    QVector<OcrWord> dataWords;
    for (const OcrWord &w : words)
        if (w.top > headerBottomY + 1.0)
            dataWords << w;

    if (dataWords.isEmpty())
        return items;

    std::sort(dataWords.begin(), dataWords.end(),
              [](const OcrWord &a, const OcrWord &b) { return a.top < b.top; });

    const double rowTolerance = medianHeight(dataWords) * 0.65;

    // Cluster words into rows by vertical proximity.
    QVector<QVector<OcrWord>> rows;
    double currentRowTop = -1e9;
    for (const OcrWord &w : dataWords) {
        if (rows.isEmpty() || (w.top - currentRowTop) > rowTolerance) {
            rows << QVector<OcrWord>{};
            currentRowTop = w.top;
        }
        rows.last() << w;
    }

    for (QVector<OcrWord> &row : rows) {
        std::sort(row.begin(), row.end(),
                  [](const OcrWord &a, const OcrWord &b) { return a.left < b.left; });

        QMap<Column, QStringList> byColumn;
        for (const OcrWord &w : row) {
            const Column col = columnForX(w.centerX(), anchors, boundaries);
            byColumn[col] << w.text;
        }

        LineItem item;
        item.description = byColumn.value(Column::Description).join(' ').trimmed();
        item.unit         = byColumn.value(Column::Unit).join(' ').trimmed();
        item.quantity     = InvoiceCalculator::cleanNumber(byColumn.value(Column::Quantity).join(' '));
        item.unitPrice    = InvoiceCalculator::cleanNumber(byColumn.value(Column::UnitPrice).join(' '));
        item.total        = InvoiceCalculator::cleanNumber(byColumn.value(Column::Total).join(' '));

        // Skip rows that are clearly not real items (e.g. "Sous-total", empty rows).
        if (item.description.isEmpty() || item.total <= 0.0)
            continue;

        items << item;
    }

    return items;
}

} // namespace TableExtractor
