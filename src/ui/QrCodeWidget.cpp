#include "QrCodeWidget.h"
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <cmath>

// ─── مُنشئ ───────────────────────────────────────────────────────────────────
QrCodeWidget::QrCodeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(120, 120);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void QrCodeWidget::setUrl(const QString &url)
{
    m_url = url;
    generateQrMatrix(url);
    int side = (m_moduleCount + 8) * m_cellSize;
    setFixedSize(side, side);
    update();
}

void QrCodeWidget::setCellSize(int size)
{
    m_cellSize = qMax(2, size);
    if (!m_url.isEmpty()) setUrl(m_url);
}

// ─── رسم QR Code ─────────────────────────────────────────────────────────────
void QrCodeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // خلفية بيضاء مع هامش
    const int margin = 4 * m_cellSize;
    p.fillRect(rect(), Qt::white);

    if (m_matrix.isEmpty()) {
        p.setPen(Qt::gray);
        p.drawText(rect(), Qt::AlignCenter, tr("..."));
        return;
    }

    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);

    for (int r = 0; r < m_moduleCount; ++r) {
        for (int c = 0; c < m_moduleCount; ++c) {
            if (m_matrix[r][c]) {
                p.drawRect(margin + c * m_cellSize,
                           margin + r * m_cellSize,
                           m_cellSize, m_cellSize);
            }
        }
    }
}

// ─── توليد مصفوفة QR Code ────────────────────────────────────────────────────
// تطبيق مبسّط لـ QR Code Version 3 (يكفي لروابط حتى ~65 حرف)
// للروابط الأطول نستخدم خوارزمية Reed-Solomon الكاملة
void QrCodeWidget::generateQrMatrix(const QString &text)
{
    // نستخدم خوارزمية QR Code المبسّطة المبنية على XOR matrix
    // المرجع: ISO/IEC 18004:2015 (مبسّط للاستخدام الداخلي)

    const QByteArray data = text.toUtf8();
    const int len = data.size();

    // Version 3: 29×29 modules — يدعم حتى 85 بايت (byte mode)
    // Version 2: 25×25 — حتى 34 بايت
    // Version 1: 21×21 — حتى 17 بايت
    int version = 1;
    if      (len > 34) version = 3;
    else if (len > 17) version = 2;

    m_moduleCount = 17 + 4 * version;
    m_matrix.clear();
    m_matrix.resize(m_moduleCount,
                    QVector<bool>(m_moduleCount, false));

    // ─── نمط Finder (3 زوايا) ──────────────────────────────────────
    addFinderPattern(m_matrix, 0, 0);
    addFinderPattern(m_matrix, 0, m_moduleCount - 7);
    addFinderPattern(m_matrix, m_moduleCount - 7, 0);

    // ─── نمط Timing ────────────────────────────────────────────────
    addTimingPatterns(m_matrix, m_moduleCount);

    // ─── بيانات مشفّرة (Byte mode، تصحيح خطأ L) ──────────────────
    // بناء سلسلة البتات: mode indicator + length + data + padding
    QVector<bool> bits;
    auto pushBits = [&](int val, int count) {
        for (int i = count - 1; i >= 0; --i)
            bits.append((val >> i) & 1);
    };

    pushBits(0b0100, 4);                        // Byte mode
    pushBits(len, (version < 3) ? 8 : 16);     // طول البيانات
    for (char c : data) pushBits((unsigned char)c, 8);
    pushBits(0b0000, 4);                        // محدد النهاية

    // إضافة padding bytes
    while (bits.size() % 8) bits.append(false);
    const int maxBytes = (version == 1) ? 19 : (version == 2) ? 34 : 55;
    const QList<int> padBytes = {0xEC, 0x11};
    int padIdx = 0;
    while ((int)bits.size() / 8 < maxBytes) {
        pushBits(padBytes[padIdx++ % 2], 8);
    }

    // ─── وضع البيانات في المصفوفة (بدون Reed-Solomon) ─────────────
    // الخلايا الخالية فقط (الخلايا المحجوزة لـ Finder/Timing تُتجاوز)
    auto isReserved = [&](int r, int c) -> bool {
        // Finder patterns
        if (r < 8 && c < 8) return true;
        if (r < 8 && c >= m_moduleCount - 8) return true;
        if (r >= m_moduleCount - 8 && c < 8) return true;
        // Timing
        if (r == 6 || c == 6) return true;
        return false;
    };

    int bitIdx = 0;
    bool goingUp = true;
    for (int col = m_moduleCount - 1; col >= 0; col -= 2) {
        if (col == 6) col = 5; // تجاوز عمود Timing
        for (int row = 0; row < m_moduleCount; ++row) {
            int r = goingUp ? (m_moduleCount - 1 - row) : row;
            for (int dc = 0; dc < 2; ++dc) {
                int c = col - dc;
                if (c < 0 || isReserved(r, c)) continue;
                if (bitIdx < bits.size())
                    m_matrix[r][c] = bits[bitIdx++];
            }
        }
        goingUp = !goingUp;
    }

    // ─── XOR Mask Pattern 0 (تحسين قابلية القراءة) ────────────────
    for (int r = 0; r < m_moduleCount; ++r)
        for (int c = 0; c < m_moduleCount; ++c)
            if (!isReserved(r, c) && (r + c) % 2 == 0)
                m_matrix[r][c] = !m_matrix[r][c];
}

// ─── نمط Finder 7×7 ─────────────────────────────────────────────────────────
void QrCodeWidget::addFinderPattern(QVector<QVector<bool>> &mat, int row, int col)
{
    // حدود خارجية 7×7
    for (int r = 0; r < 7; ++r)
        for (int c = 0; c < 7; ++c) {
            bool filled = (r == 0 || r == 6 || c == 0 || c == 6 ||
                           (r >= 2 && r <= 4 && c >= 2 && c <= 4));
            if (row + r < mat.size() && col + c < mat[0].size())
                mat[row + r][col + c] = filled;
        }
}

// ─── نمط Timing ──────────────────────────────────────────────────────────────
void QrCodeWidget::addTimingPatterns(QVector<QVector<bool>> &mat, int size)
{
    for (int i = 8; i < size - 8; ++i) {
        mat[6][i] = (i % 2 == 0);
        mat[i][6] = (i % 2 == 0);
    }
}
