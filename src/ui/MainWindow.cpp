#include "MainWindow.h"
#include "Sidebar.h"
#include "DashboardPage.h"
#include "NewInvoicePage.h"
#include "../core/HttpServer.h"
#include "../core/Database.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QStyle>
#include <QDebug>

// ─── مُنشئ ───────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database(std::make_unique<Database>())
{
    setWindowTitle(tr("الفولاذ — استخراج الفواتير"));
    setMinimumSize(1100, 680);
    resize(1280, 760);

    // فتح قاعدة البيانات (وإنشاء الجداول إن لزم) قبل استخدامها في الصفحات.
    if (!m_database->init()) {
        QMessageBox::critical(this, tr("خطأ قاعدة البيانات"),
                              tr("تعذّر فتح قاعدة البيانات:\n%1").arg(m_database->lastError()));
    }

    buildUi();
    startHttpServer();

    // بيانات الشركة الافتراضية
    m_sidebar->setCompanyInfo(
        QStringLiteral("ELFOULADH"),
        QStringLiteral("Menzel Bourguiba")
        );
}

MainWindow::~MainWindow() = default;

// ─── بناء الواجهة ────────────────────────────────────────────────────────────
void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *root    = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // الشريط الجانبي
    m_sidebar = new Sidebar(this);
    root->addWidget(m_sidebar);

    // منطقة المحتوى
    m_stack = new QStackedWidget(this);
    root->addWidget(m_stack, 1);

    // الصفحتان — تتشاركان نفس قاعدة البيانات (m_database)
    m_dashboardPage  = new DashboardPage(m_database.get(), this);
    m_newInvoicePage = new NewInvoicePage(this, m_database.get());
    m_stack->addWidget(m_dashboardPage);   // index 0
    m_stack->addWidget(m_newInvoicePage);  // index 1
    m_stack->setCurrentIndex(0);

    setCentralWidget(central);

    // ربط إشارات الشريط الجانبي
    connect(m_sidebar, &Sidebar::dashboardRequested,
            this,      &MainWindow::showDashboard);
    connect(m_sidebar, &Sidebar::newInvoiceRequested,
            this,      &MainWindow::showNewInvoice);

    // بعد حفظ الفاتورة → العودة للوحة التحكم + تحديث الإحصائيات
    connect(m_newInvoicePage, &NewInvoicePage::invoiceSaved,
            this,             &MainWindow::showDashboard);
    connect(m_newInvoicePage, &NewInvoicePage::invoiceSaved,
            m_dashboardPage,  &DashboardPage::refresh);
}

// ─── تشغيل خادم HTTP ─────────────────────────────────────────────────────────
void MainWindow::startHttpServer()
{
    m_httpServer = new HttpServer(this);

    connect(m_httpServer, &HttpServer::imageReceived,
            this,         &MainWindow::onImageReceivedFromPhone);
    connect(m_httpServer, &HttpServer::serverError,
            this,         &MainWindow::onServerError);

    if (m_httpServer->start(8080)) {
        const QString url = m_httpServer->localUrl();
        m_sidebar->setServerUrl(url);
        qDebug() << "[MainWindow] خادم HTTP يعمل على:" << url;
    }
}

// ─── التنقل بين الصفحات ──────────────────────────────────────────────────────
void MainWindow::showDashboard()
{
    m_stack->setCurrentIndex(0);
}

void MainWindow::showNewInvoice()
{
    m_stack->setCurrentIndex(1);
}

// ─── استقبال صورة من الهاتف ──────────────────────────────────────────────────
void MainWindow::onImageReceivedFromPhone(const QByteArray &imageData,
                                          const QString &fileName)
{
    qDebug() << "[MainWindow] صورة واردة من الهاتف:" << fileName
             << "الحجم:" << imageData.size() << "بايت";

    // الانتقال لصفحة الاستخراج
    showNewInvoice();

    // تحميل الصورة في صفحة الفاتورة الجديدة (يفتح المراجعة تلقائياً)
    m_newInvoicePage->loadImageFromData(imageData, fileName);

    // إشعار سطح المكتب (اختياري — يعمل إذا كان النظام يدعمه)
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        auto *tray = new QSystemTrayIcon(
            QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation),
            this
            );
        tray->show();
        tray->showMessage(
            tr("فاتورة جديدة"),
            tr("تم استلام الصورة من الهاتف: %1").arg(fileName),
            QSystemTrayIcon::Information, 3000
            );
    }

    // رفع النافذة إلى الأمام
    raise();
    activateWindow();
}

// ─── معالجة خطأ الخادم ───────────────────────────────────────────────────────
void MainWindow::onServerError(const QString &msg)
{
    qWarning() << "[MainWindow] خطأ الخادم:" << msg;
    QMessageBox::warning(this,
                         tr("خادم HTTP"),
                         tr("⚠️ تعذّر تشغيل خادم الاستقبال اللاسلكي:\n%1\n\n"
                            "يمكنك مواصلة العمل يدوياً بسحب الصور من الكمبيوتر.").arg(msg));
}