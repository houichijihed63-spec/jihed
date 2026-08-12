#include "NewInvoicePage.h"
#include "DropArea.h"
#include "../core/OcrClient.h"
#include "../core/TextNormalizer.h"
#include "../core/FieldExtractor.h"
#include "../core/TableExtractor.h"
#include "../core/InvoiceCalculator.h"
#include "../core/Database.h"
#include "../core/InvoiceModel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QScrollArea>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QPixmap>
#include <QSettings>
#include <QTimer>
#include <QDebug>
#include <QDateTime>

// ─── مُنشئ ───────────────────────────────────────────────────────────────────
NewInvoicePage::NewInvoicePage(QWidget *parent, Database *db)
    : QWidget(parent)
    , m_ocrClient(new OcrClient(this))
{
    if (db) {
        m_db = db;
        m_ownsDb = false;
    } else {
        // لا توجد قاعدة بيانات مشتركة مُمرَّرة: أنشئ وافتح واحدة خاصة بهذه الصفحة.
        m_db = new Database();
        if (!m_db->init())
            qWarning() << "[NewInvoicePage] فشل فتح قاعدة البيانات:" << m_db->lastError();
        m_ownsDb = true;
    }

    buildUi();

    connect(m_ocrClient, &OcrClient::finished, this, &NewInvoicePage::onOcrFinished);
    connect(m_ocrClient, &OcrClient::progress, this, [this](const QString &msg) {
        m_statusLabel->setText(msg);
    });
}

NewInvoicePage::~NewInvoicePage()
{
    if (m_ownsDb)
        delete m_db;
}

// ─── بناء الواجهة ────────────────────────────────────────────────────────────
void NewInvoicePage::buildUi()
{
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(20, 20, 20, 20);
    rootLayout->setSpacing(16);

    // ══════════════════════════════════════════════════════════════
    // العمود الأيسر: رفع الصورة + معاينة + تحكم OCR
    // ══════════════════════════════════════════════════════════════
    auto *leftCol = new QVBoxLayout;
    leftCol->setSpacing(12);

    auto *title = new QLabel(tr("➕ استخراج فاتورة جديدة"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    leftCol->addWidget(title);

    // منطقة السحب والإفلات
    auto *dropGroup = new QGroupBox(tr("صورة الفاتورة"), this);
    auto *dropLayout = new QVBoxLayout(dropGroup);
    m_dropArea = new DropArea(this);
    dropLayout->addWidget(m_dropArea);

    // معاينة الصورة
    m_previewLabel = new QLabel(this);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setMinimumHeight(200);
    m_previewLabel->setObjectName(QStringLiteral("previewBox"));
    m_previewLabel->setText(tr("معاينة الصورة ستظهر هنا"));
    dropLayout->addWidget(m_previewLabel);
    leftCol->addWidget(dropGroup);

    // شريط التقدم والحالة
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0); // وضع التحميل غير المحدد
    m_progressBar->setVisible(false);
    m_progressBar->setFixedHeight(8);
    m_progressBar->setTextVisible(false);
    leftCol->addWidget(m_progressBar);

    m_statusLabel = new QLabel(tr("جاهز — ارفع صورة الفاتورة"), this);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    leftCol->addWidget(m_statusLabel);

    // أزرار التحكم
    auto *btnRow = new QHBoxLayout;
    m_btnExtract = new QPushButton(tr("🔍 استخراج البيانات"), this);
    m_btnExtract->setObjectName(QStringLiteral("primaryButton"));
    m_btnExtract->setEnabled(false);

    m_btnClear = new QPushButton(tr("🗑️ مسح"), this);
    m_btnClear->setObjectName(QStringLiteral("secondaryButton"));

    btnRow->addWidget(m_btnExtract);
    btnRow->addWidget(m_btnClear);
    leftCol->addLayout(btnRow);
    leftCol->addStretch();

    // ══════════════════════════════════════════════════════════════
    // العمود الأيمن: حقول المراجعة + جدول البنود + حفظ
    // ══════════════════════════════════════════════════════════════
    auto *rightScroll = new QScrollArea(this);
    rightScroll->setWidgetResizable(true);
    rightScroll->setFrameShape(QFrame::NoFrame);

    auto *rightWidget = new QWidget(rightScroll);
    auto *rightCol = new QVBoxLayout(rightWidget);
    rightCol->setSpacing(12);

    auto *fieldsGroup = new QGroupBox(tr("بيانات الفاتورة"), rightWidget);
    auto *fieldsGrid = new QVBoxLayout(fieldsGroup);

    auto addField = [&](const QString &label, QLineEdit *&field, const QString &placeholder = QString()) {
        auto *l = new QLabel(label, fieldsGroup);
        field = new QLineEdit(fieldsGroup);
        if (!placeholder.isEmpty())
            field->setPlaceholderText(placeholder);
        fieldsGrid->addWidget(l);
        fieldsGrid->addWidget(field);
    };

    addField(tr("رقم الفاتورة"), m_fieldNumber);
    addField(tr("التاريخ (dd/MM/yyyy)"), m_fieldDate, QStringLiteral("dd/MM/yyyy"));
    addField(tr("المورد"), m_fieldSupplier);
    addField(tr("العميل"), m_fieldClient);
    addField(tr("نسبة TVA %"), m_fieldTax);

    m_fieldTotal = new QLineEdit(fieldsGroup);
    m_fieldTotal->setReadOnly(true);
    auto *totalLabel = new QLabel(tr("المجموع"), fieldsGroup);
    fieldsGrid->addWidget(totalLabel);
    fieldsGrid->addWidget(m_fieldTotal);

    rightCol->addWidget(fieldsGroup);

    // جدول البنود
    auto *itemsGroup = new QGroupBox(tr("بنود الفاتورة"), rightWidget);
    auto *itemsLayout = new QVBoxLayout(itemsGroup);
    m_itemsTable = new QTableWidget(0, 5, itemsGroup);
    m_itemsTable->setHorizontalHeaderLabels(
        {tr("Désignation"), tr("Qté"), tr("Unité"), tr("P.U"), tr("Total")});
    m_itemsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_itemsTable->verticalHeader()->setVisible(false);
    itemsLayout->addWidget(m_itemsTable);

    auto *btnAddRow = new QPushButton(tr("+ إضافة سطر"), itemsGroup);
    itemsLayout->addWidget(btnAddRow, 0, Qt::AlignLeft);
    connect(btnAddRow, &QPushButton::clicked, this, [this] {
        m_itemsTable->insertRow(m_itemsTable->rowCount());
    });

    rightCol->addWidget(itemsGroup);

    m_btnSave = new QPushButton(tr("💾 حفظ الفاتورة"), rightWidget);
    m_btnSave->setObjectName(QStringLiteral("successButton"));
    m_btnSave->setEnabled(false);
    rightCol->addWidget(m_btnSave);

    rightScroll->setWidget(rightWidget);

    rootLayout->addLayout(leftCol, 1);
    rootLayout->addWidget(rightScroll, 1);

    // ─── الإشارات ────────────────────────────────────────────────
    connect(m_dropArea, &DropArea::imageSelected, this, &NewInvoicePage::onImageDropped);
    connect(m_dropArea, &DropArea::invalidFile,   this, &NewInvoicePage::onImageInvalid);
    connect(m_btnExtract, &QPushButton::clicked,  this, &NewInvoicePage::onExtractClicked);
    connect(m_btnSave,    &QPushButton::clicked,  this, &NewInvoicePage::onSaveClicked);
    connect(m_btnClear,   &QPushButton::clicked,  this, &NewInvoicePage::onClearClicked);
}

// ─── استقبال صورة من الهاتف (تلقائي) ────────────────────────────────────────
void NewInvoicePage::loadImageFromData(const QByteArray &imageData,
                                       const QString &fileName)
{
    saveImageTemp(imageData, fileName);

    QPixmap pix;
    pix.loadFromData(imageData);
    if (!pix.isNull()) {
        m_previewLabel->setPixmap(pix.scaled(
            m_previewLabel->width() - 16, m_previewLabel->height() - 16,
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    m_statusLabel->setText(tr("📱 تم استلام الصورة من الهاتف: %1").arg(fileName));
    m_btnExtract->setEnabled(true);

    // بدء الاستخراج تلقائياً بعد قصير (تجربة أفضل)
    QTimer::singleShot(800, this, &NewInvoicePage::onExtractClicked);
}

void NewInvoicePage::saveImageTemp(const QByteArray &data,
                                   const QString &name)
{
    const QString tmpDir = QStandardPaths::writableLocation(
                               QStandardPaths::TempLocation) + QStringLiteral("/invoices");
    QDir().mkpath(tmpDir);
    m_currentImagePath = tmpDir + QStringLiteral("/") + name;
    QFile f(m_currentImagePath);
    if (f.open(QIODevice::WriteOnly))
        f.write(data);
}

// ─── اختيار صورة بالسحب والإفلات / الملف ────────────────────────────────────
void NewInvoicePage::onImageDropped(const QString &filePath)
{
    m_currentImagePath = filePath;
    const QPixmap pix(filePath);
    if (!pix.isNull()) {
        m_previewLabel->setPixmap(pix.scaled(
            m_previewLabel->width() - 16, m_previewLabel->height() - 16,
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_statusLabel->setText(tr("✅ الصورة محمّلة: %1").arg(QFileInfo(filePath).fileName()));
    m_btnExtract->setEnabled(true);
    m_btnSave->setEnabled(false);
}

void NewInvoicePage::onImageInvalid(const QString &reason)
{
    m_statusLabel->setText(QStringLiteral("⚠️ ") + reason);
}

// ─── بدء الاستخراج ───────────────────────────────────────────────────────────
void NewInvoicePage::onExtractClicked()
{
    if (m_currentImagePath.isEmpty()) {
        QMessageBox::information(this, tr("تنبيه"),
                                 tr("يرجى تحميل صورة الفاتورة أولاً."));
        return;
    }

    const QString apiKey = m_ocrClient->effectiveApiKey();
    if (apiKey.isEmpty()) {
        QMessageBox::information(this, tr("مفتاح OCR.space"),
                                 tr("الرجاء إدخال مفتاح OCR.space المجاني من\n"
                                    "https://ocr.space/ocrapi\n\n"
                                    "يُحفظ تلقائياً عبر QSettings عند ضبطه في الإعدادات."));
        return;
    }

    setExtracting(true);
    m_statusLabel->setText(tr("☁️ جاري الاستخراج عبر OCR.space..."));
    m_ocrClient->extractFromImage(m_currentImagePath);
}

// ─── اكتمال الاستخراج ────────────────────────────────────────────────────────
void NewInvoicePage::onOcrFinished(const OcrResult &result)
{
    setExtracting(false);

    if (!result.success) {
        m_statusLabel->setText(tr("❌ خطأ في الاستخراج: %1").arg(result.errorMessage));
        m_btnSave->setEnabled(false);
        QMessageBox::warning(this, tr("خطأ OCR"), result.errorMessage);
        return;
    }

    m_statusLabel->setText(tr("✅ تم الاستخراج — راجع البيانات وعدّلها إن لزم"));
    populateFieldsFromOcr(result);
    m_btnSave->setEnabled(true);
}

// ─── ملء الحقول من نتيجة OCR ─────────────────────────────────────────────────
void NewInvoicePage::populateFieldsFromOcr(const OcrResult &result)
{
    const QString normalized = TextNormalizer::normalize(result.rawText);

    Invoice invoice;
    FieldExtractor::extract(normalized, invoice);

    const double detectedTax = InvoiceCalculator::extractTaxRate(normalized);
    if (detectedTax >= 0.0)
        invoice.taxRate = detectedTax; // وإلا يبقى الافتراضي 19% من InvoiceModel

    invoice.lineItems = TableExtractor::extract(result.words);
    InvoiceCalculator::computeTotals(invoice);

    m_fieldNumber->setText(invoice.invoiceNumber);
    m_fieldDate->setText(invoice.invoiceDate.isValid()
                             ? invoice.invoiceDate.toString(QStringLiteral("dd/MM/yyyy"))
                             : QString());
    m_fieldSupplier->setText(invoice.supplierName);
    m_fieldClient->setText(invoice.clientName);
    m_fieldTax->setText(QString::number(invoice.taxRate));

    m_itemsTable->setRowCount(0);
    for (const LineItem &item : invoice.lineItems) {
        const int row = m_itemsTable->rowCount();
        m_itemsTable->insertRow(row);
        m_itemsTable->setItem(row, 0, new QTableWidgetItem(item.description));
        m_itemsTable->setItem(row, 1, new QTableWidgetItem(QString::number(item.quantity, 'f', 3)));
        m_itemsTable->setItem(row, 2, new QTableWidgetItem(item.unit));
        m_itemsTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.unitPrice, 'f', 3)));
        m_itemsTable->setItem(row, 4, new QTableWidgetItem(QString::number(item.total, 'f', 3)));
    }

    m_fieldTotal->setText(
        QString::number(invoice.totalAmount, 'f', 3) + QStringLiteral(" ") + invoice.currency);
}

// ─── حفظ الفاتورة ────────────────────────────────────────────────────────────
void NewInvoicePage::onSaveClicked()
{
    Invoice inv;
    inv.invoiceNumber = m_fieldNumber->text().trimmed();
    inv.invoiceDate   = QDate::fromString(m_fieldDate->text().trimmed(), QStringLiteral("dd/MM/yyyy"));
    inv.supplierName  = m_fieldSupplier->text().trimmed();
    inv.clientName    = m_fieldClient->text().trimmed();
    inv.sourceImagePath = m_currentImagePath;

    const QString taxStr = m_fieldTax->text().remove('%').trimmed();
    inv.taxRate = taxStr.isEmpty() ? 19.0 : taxStr.toDouble();

    for (int r = 0; r < m_itemsTable->rowCount(); ++r) {
        LineItem item;
        item.description = m_itemsTable->item(r, 0) ? m_itemsTable->item(r, 0)->text() : QString();
        item.quantity    = m_itemsTable->item(r, 1) ? m_itemsTable->item(r, 1)->text().toDouble() : 0.0;
        item.unit        = m_itemsTable->item(r, 2) ? m_itemsTable->item(r, 2)->text() : QString();
        item.unitPrice   = m_itemsTable->item(r, 3) ? m_itemsTable->item(r, 3)->text().toDouble() : 0.0;
        item.total       = m_itemsTable->item(r, 4) ? m_itemsTable->item(r, 4)->text().toDouble() : 0.0;
        inv.lineItems.append(item);
    }

    InvoiceCalculator::computeTotals(inv);

    if (!m_db || !m_db->saveInvoice(inv)) {
        QMessageBox::critical(this, tr("خطأ"),
                              tr("فشل حفظ الفاتورة في قاعدة البيانات.\n%1")
                                  .arg(m_db ? m_db->lastError() : tr("لا توجد قاعدة بيانات متاحة.")));
        return;
    }

    QMessageBox::information(this, tr("تم الحفظ"),
                             tr("✅ تم حفظ الفاتورة رقم %1 بنجاح!").arg(inv.invoiceNumber));

    emit invoiceSaved();
    onClearClicked();
}

// ─── مسح النموذج ─────────────────────────────────────────────────────────────
void NewInvoicePage::onClearClicked()
{
    m_currentImagePath.clear();
    m_dropArea->clearSelection();
    m_previewLabel->setPixmap(QPixmap());
    m_previewLabel->setText(tr("معاينة الصورة ستظهر هنا"));
    m_fieldNumber->clear();
    m_fieldDate->clear();
    m_fieldSupplier->clear();
    m_fieldClient->clear();
    m_fieldTax->clear();
    m_fieldTotal->clear();
    m_itemsTable->setRowCount(0);
    m_btnExtract->setEnabled(false);
    m_btnSave->setEnabled(false);
    m_statusLabel->setText(tr("جاهز — ارفع صورة الفاتورة"));
}

// ─── حالة الاستخراج (جاري/منتهي) ────────────────────────────────────────────
void NewInvoicePage::setExtracting(bool on)
{
    m_progressBar->setVisible(on);
    m_btnExtract->setEnabled(!on);
    m_btnSave->setEnabled(!on ? m_btnSave->isEnabled() : false);
}