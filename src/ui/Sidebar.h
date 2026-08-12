#pragma once
#include <QWidget>
#include <QString>

class QLabel;
class QPushButton;
class QrCodeWidget;

// الشريط الجانبي: شعار الشركة + أزرار التنقل + QR Code للهاتف
class Sidebar : public QWidget
{
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    // تحديث رابط QR Code (يُستدعى بعد بدء خادم HTTP)
    void setServerUrl(const QString &url);
    // تحديث معلومات الشركة
    void setCompanyInfo(const QString &name, const QString &city);

signals:
    void dashboardRequested();
    void newInvoiceRequested();

private:
    void buildUi();
    void applyActiveStyle(QPushButton *active);

    QLabel        *m_logoLabel    = nullptr;
    QLabel        *m_companyLabel = nullptr;
    QLabel        *m_cityLabel    = nullptr;
    QPushButton   *m_btnDashboard = nullptr;
    QPushButton   *m_btnNew       = nullptr;
    QrCodeWidget  *m_qrCode       = nullptr;
    QLabel        *m_qrLabel      = nullptr;
    QLabel        *m_urlLabel     = nullptr;
    QPushButton   *m_activeBtn    = nullptr;
};
