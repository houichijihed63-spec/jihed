#pragma once
#include <QWidget>
#include <QByteArray>
#include <QString>
#include "../core/OcrClient.h"   // for OcrResult (used by value in the slot signature)

class QLabel;
class QPushButton;
class QLineEdit;
class QTableWidget;
class QProgressBar;
class DropArea;
class Database;

// صفحة إضافة فاتورة جديدة:
//  1. رفع صورة (سحب/إفلات أو من الهاتف تلقائياً)
//  2. استخراج OCR عبر OCR.space (OcrClient)
//  3. مراجعة الحقول وتعديلها
//  4. حفظ في قاعدة البيانات
class NewInvoicePage : public QWidget
{
    Q_OBJECT
public:
    // db اختياري: إن مُرِّر (مثلاً m_database من MainWindow) تُستخدم نفس
    // قاعدة البيانات المشتركة؛ وإلا تُنشئ الصفحة وتفتح قاعدتها الخاصة.
    explicit NewInvoicePage(QWidget *parent = nullptr, Database *db = nullptr);
    ~NewInvoicePage() override;

    // يُستدعى من MainWindow عند وصول صورة من الهاتف
    void loadImageFromData(const QByteArray &imageData,
                           const QString &fileName);

signals:
    void invoiceSaved(); // يُطلق بعد الحفظ الناجح في SQLite

private slots:
    void onImageDropped(const QString &filePath);
    void onImageInvalid(const QString &reason);
    void onExtractClicked();
    void onOcrFinished(const OcrResult &result);
    void onSaveClicked();
    void onClearClicked();

private:
    void buildUi();
    void populateFieldsFromOcr(const OcrResult &result);
    void setExtracting(bool on);
    void saveImageTemp(const QByteArray &data, const QString &name);

    // ─── ويدجتات ──────────────────────────────────────────────────
    DropArea      *m_dropArea      = nullptr;
    QLabel        *m_previewLabel  = nullptr;
    QLabel        *m_statusLabel   = nullptr;
    QProgressBar  *m_progressBar   = nullptr;

    // حقول المراجعة
    QLineEdit     *m_fieldNumber   = nullptr; // رقم الفاتورة
    QLineEdit     *m_fieldDate     = nullptr; // التاريخ
    QLineEdit     *m_fieldSupplier = nullptr; // المورد
    QLineEdit     *m_fieldClient   = nullptr; // العميل
    QLineEdit     *m_fieldTax      = nullptr; // نسبة TVA
    QLineEdit     *m_fieldTotal    = nullptr; // المجموع (قراءة فقط)
    QTableWidget  *m_itemsTable    = nullptr; // جدول البنود

    QPushButton   *m_btnExtract    = nullptr;
    QPushButton   *m_btnSave       = nullptr;
    QPushButton   *m_btnClear      = nullptr;

    // ─── خدمات ────────────────────────────────────────────────────
    OcrClient  *m_ocrClient  = nullptr; // OCR.space (إنترنت)
    Database   *m_db         = nullptr; // مؤشر لقاعدة بيانات مشتركة (أو مملوكة محلياً)
    bool        m_ownsDb     = false;   // true إذا أنشأنا Database خاصة بنا

    QString  m_currentImagePath; // مسار الصورة الحالية
};