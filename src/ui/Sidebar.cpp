#include "Sidebar.h"
#include "QrCodeWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QFont>
#include <QStyle>
#include <QStyle>
#include <QNetworkAccessManager>

// ─── مُنشئ ───────────────────────────────────────────────────────────────────
Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(220);
    setObjectName(QStringLiteral("Sidebar"));
    buildUi();
}

// ─── بناء الواجهة ────────────────────────────────────────────────────────────
void Sidebar::buildUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 24, 16, 16);
    layout->setSpacing(8);
    setStyleSheet(QStringLiteral(R"(
        QWidget#Sidebar {
            background: #1e293b;
            border-right: 1px solid #334155;
        }
        QPushButton#navBtn {
            text-align: left;
            padding: 10px 14px;
            border: none;
            border-radius: 8px;
            color: #94a3b8;
            font-size: 14px;
            background: transparent;
        }
        QPushButton#navBtn:hover {
            background: #263348;
            color: #e2e8f0;
        }
        QPushButton#navBtn[active="true"] {
            background: #2563eb;
            color: #ffffff;
            font-weight: bold;
        }
        QLabel#companyName {
            color: #f1f5f9;
            font-size: 15px;
            font-weight: bold;
        }
        QLabel#cityLabel {
            color: #64748b;
            font-size: 11px;
        }
        QLabel#qrLabel {
            color: #94a3b8;
            font-size: 11px;
            font-weight: bold;
            margin-top: 8px;
        }
        QLabel#urlLabel {
            color: #3b82f6;
            font-size: 10px;
            word-wrap: true;
        }
    )"));

    // ─── الشعار ────────────────────────────────────────────────────
    m_logoLabel = new QLabel(this);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setFixedHeight(80);
    // تحميل الشعار من الموارد المضمّنة
    QPixmap logo(QStringLiteral(":/images/logo.png"));
    if (!logo.isNull())
        m_logoLabel->setPixmap(logo.scaled(72, 72,
            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        m_logoLabel->setText(QStringLiteral("🏭"));
    layout->addWidget(m_logoLabel);

    // ─── اسم الشركة ────────────────────────────────────────────────
    m_companyLabel = new QLabel(QStringLiteral("ELFOULADH"), this);
    m_companyLabel->setObjectName(QStringLiteral("companyName"));
    m_companyLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_companyLabel);

    m_cityLabel = new QLabel(QStringLiteral("Menzel Bourguiba"), this);
    m_cityLabel->setObjectName(QStringLiteral("cityLabel"));
    m_cityLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_cityLabel);

    layout->addSpacing(16);

    // ─── أزرار التنقل ──────────────────────────────────────────────
    m_btnDashboard = new QPushButton(tr("📊  لوحة التحكم"), this);
    m_btnDashboard->setObjectName(QStringLiteral("navBtn"));
    m_btnDashboard->setProperty("active", true);
    m_btnDashboard->setStyle(style()); // تحديث الخاصية
    m_activeBtn = m_btnDashboard;

    m_btnNew = new QPushButton(tr("➕  فاتورة جديدة"), this);
    m_btnNew->setObjectName(QStringLiteral("navBtn"));

    layout->addWidget(m_btnDashboard);
    layout->addWidget(m_btnNew);
    layout->addStretch();

    // ─── قسم QR Code للهاتف ────────────────────────────────────────
    auto *separator = new QLabel(this);
    separator->setFixedHeight(1);
    separator->setStyleSheet(QStringLiteral("background:#334155;"));
    layout->addWidget(separator);

    m_qrLabel = new QLabel(tr("📱 رفع من الهاتف:"), this);
    m_qrLabel->setObjectName(QStringLiteral("qrLabel"));
    layout->addWidget(m_qrLabel);

    m_qrCode = new QrCodeWidget(this);
    m_qrCode->setCellSize(4);
    m_qrCode->setVisible(false); // يظهر بعد بدء الخادم
    layout->addWidget(m_qrCode, 0, Qt::AlignCenter);

    m_urlLabel = new QLabel(tr("جاري التشغيل..."), this);
    m_urlLabel->setObjectName(QStringLiteral("urlLabel"));
    m_urlLabel->setWordWrap(true);
    m_urlLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_urlLabel);

    layout->addSpacing(8);

    // ربط الأزرار
    connect(m_btnDashboard, &QPushButton::clicked, this, [this] {
        applyActiveStyle(m_btnDashboard);
        emit dashboardRequested();
    });
    connect(m_btnNew, &QPushButton::clicked, this, [this] {
        applyActiveStyle(m_btnNew);
        emit newInvoiceRequested();
    });
}

// ─── تحديث QR Code بعد بدء الخادم ───────────────────────────────────────────
void Sidebar::setServerUrl(const QString &url)
{
    m_urlLabel->setText(url);
    m_qrCode->setUrl(url);
    m_qrCode->setVisible(true);
    m_qrLabel->setText(tr("📱 امسح للرفع من الهاتف:"));
}

// ─── تحديث معلومات الشركة ────────────────────────────────────────────────────
void Sidebar::setCompanyInfo(const QString &name, const QString &city)
{
    m_companyLabel->setText(name);
    m_cityLabel->setText(city);
}

// ─── تحديث زر نشط ────────────────────────────────────────────────────────────
void Sidebar::applyActiveStyle(QPushButton *active)
{
    if (m_activeBtn) {
        m_activeBtn->setProperty("active", false);
        m_activeBtn->style()->unpolish(m_activeBtn);
        m_activeBtn->style()->polish(m_activeBtn);
    }
    active->setProperty("active", true);
    active->style()->unpolish(active);
    active->style()->polish(active);
    m_activeBtn = active;
}