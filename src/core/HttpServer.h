#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>

// خادم HTTP خفيف داخل Qt — يستقبل صور الفواتير من الهاتف عبر WiFi المحلي
class HttpServer : public QObject
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);
    ~HttpServer();

    // بدء الاستماع على المنفذ المحدد (افتراضي 8080)
    bool start(quint16 port = 8080);
    void stop();

    bool isRunning() const;
    QString localUrl() const; // مثال: http://192.168.1.10:8080

signals:
    // يُطلق عند استلام صورة جديدة من الهاتف
    void imageReceived(const QByteArray &imageData, const QString &fileName);
    // يُطلق عند خطأ في الخادم
    void serverError(const QString &message);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    void handleRequest(QTcpSocket *socket, const QByteArray &data);
    void sendHtml(QTcpSocket *socket, const QByteArray &html, int statusCode = 200);
    void sendResponse(QTcpSocket *socket, int code, const QString &body);

    // استخراج محتوى multipart/form-data من الطلب
    QByteArray extractImageFromMultipart(const QByteArray &body,
                                         const QByteArray &boundary,
                                         QString &outFileName);

    QTcpServer *m_server = nullptr;
    quint16     m_port   = 8080;

    // HTML صفحة الهاتف مخزنة في الذاكرة
    QByteArray  m_uploadPageHtml;
    void        loadUploadPage();
};
