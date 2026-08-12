#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <memory>

class Sidebar;
class DashboardPage;
class NewInvoicePage;
class HttpServer;
class Database;

// النافذة الرئيسية — تربط الشريط الجانبي + الصفحتين + خادم HTTP
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showDashboard();
    void showNewInvoice();
    // استقبال صورة واردة من الهاتف عبر WiFi
    void onImageReceivedFromPhone(const QByteArray &imageData,
                                  const QString &fileName);
    void onServerError(const QString &msg);

private:
    void buildUi();
    void startHttpServer();

    // Database ليست QObject (لا تقبل parent) — تُدار هنا كـ unique_ptr عادي.
    std::unique_ptr<Database> m_database;

    Sidebar          *m_sidebar       = nullptr;
    QStackedWidget   *m_stack         = nullptr;
    DashboardPage    *m_dashboardPage = nullptr;
    NewInvoicePage   *m_newInvoicePage= nullptr;
    HttpServer       *m_httpServer    = nullptr;
};
