#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>
#include <QMap>

/**
 * Lightweight HTTP server embedded inside Qt.
 * Receives invoice images sent from a phone over the local WiFi network.
 *
 * FIXES applied vs. original:
 *  1. Per-socket request buffer (m_buffers) accumulates all TCP chunks before
 *     parsing – fixes broken uploads from mobile phones whose images arrive
 *     split across multiple TCP segments.
 *  2. Content-Length–aware read loop: waits until the full body has arrived.
 *  3. CORS headers added to every response – required by MIUI / Android
 *     WebView when the page is served from a local IP address.
 *  4. Multipart boundary extraction is now case-insensitive for the header
 *     name but preserves the exact boundary value (case-sensitive).
 *  5. OPTIONS pre-flight handled so modern browsers don't block the upload.
 */
class HttpServer : public QObject
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);
    ~HttpServer();

    bool start(quint16 port = 8080);
    void stop();

    bool    isRunning() const;
    QString localUrl()  const;

signals:
    void imageReceived(const QByteArray &imageData, const QString &fileName);
    void serverError(const QString &message);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    // ── request parsing ─────────────────────────────────────────────────────
    void     handleRequest(QTcpSocket *socket, const QByteArray &data);

    // ── multipart helper ─────────────────────────────────────────────────────
    QByteArray extractImageFromMultipart(const QByteArray &body,
                                         const QByteArray &boundary,
                                         QString          &outFileName);

    // ── response helpers ─────────────────────────────────────────────────────
    void sendHtml    (QTcpSocket *socket, const QByteArray &html,
                      int statusCode = 200);
    void sendResponse(QTcpSocket *socket, int code, const QString &body);
    void sendCorsOk  (QTcpSocket *socket);   // OPTIONS pre-flight

    // ── helpers ──────────────────────────────────────────────────────────────
    void loadUploadPage();

    // ── state ────────────────────────────────────────────────────────────────
    QTcpServer *m_server = nullptr;
    quint16     m_port   = 8080;

    // Per-socket accumulation buffer (fixes multi-packet mobile uploads)
    QMap<QTcpSocket *, QByteArray> m_buffers;

    QByteArray  m_uploadPageHtml;
};
