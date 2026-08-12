#pragma once
#include <QWidget>
#include <QString>
#include <QPixmap>

// ويدجت عرض QR Code — يولّد QR Code بدون مكتبات خارجية
// يستخدم QR Code API مجانية محلية مبنية بـ C++ بسيط
// أو خيار احتياطي: يرسم مربعات صغيرة بناءً على مصفوفة البيانات
class QrCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QrCodeWidget(QWidget *parent = nullptr);

    // تحديث الرابط وإعادة رسم QR Code
    void setUrl(const QString &url);
    QString url() const { return m_url; }

    // حجم كل خلية بالبكسل (افتراضي 4)
    void setCellSize(int size);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // توليد مصفوفة QR Code من النص
    // خوارزمية QR Code المبسطة (Version 1 — حتى 25 حرف)
    // للروابط الطويلة: استخدام Version 3
    void generateQrMatrix(const QString &text);

    QString              m_url;
    QVector<QVector<bool>> m_matrix; // true = خلية سوداء
    int                  m_moduleCount = 0;
    int                  m_cellSize    = 4;

    // --- بنية QR Code المبسطة ---
    static QVector<QVector<bool>> encodeQr(const QString &text, int &outSize);
    static void addFinderPattern(QVector<QVector<bool>> &mat, int row, int col);
    static void addTimingPatterns(QVector<QVector<bool>> &mat, int size);
    static QVector<bool> encodeAlphanumeric(const QString &text);
    static QVector<bool> toBits(int value, int bits);
    static QVector<bool> reedSolomon(const QVector<bool> &data);
};