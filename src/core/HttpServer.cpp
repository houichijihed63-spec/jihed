#include "HttpServer.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QFile>
#include <QDebug>
#include <QDateTime>

// ─────────────────────────────────────────────────────────────────────────────
// CORS headers injected into every response.
// Required by MIUI Browser / Android WebView when the origin is a local IP.
// ─────────────────────────────────────────────────────────────────────────────
static const QByteArray CORS_HEADERS =
    "Access-Control-Allow-Origin: *\r\n"
    "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
    "Access-Control-Allow-Headers: Content-Type\r\n";

// ─── Constructor ─────────────────────────────────────────────────────────────
HttpServer::HttpServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection,
            this,    &HttpServer::onNewConnection);
    loadUploadPage();
}

HttpServer::~HttpServer()
{
    stop();
}

// ─── Start / Stop ────────────────────────────────────────────────────────────
bool HttpServer::start(quint16 port)
{
    m_port = port;
    // Listen on ALL local interfaces (IPv4) so the phone can reach us
    if (!m_server->listen(QHostAddress::AnyIPv4, port)) {
        emit serverError(tr("Failed to start server on port %1: %2")
                           .arg(port).arg(m_server->errorString()));
        return false;
    }
    qDebug() << "[HttpServer] listening on:" << localUrl();
    return true;
}

void HttpServer::stop()
{
    if (m_server->isListening())
        m_server->close();
}

bool HttpServer::isRunning() const
{
    return m_server->isListening();
}

// ─── Local URL ───────────────────────────────────────────────────────────────
QString HttpServer::localUrl() const
{
    const auto ifaces = QNetworkInterface::allAddresses();
    for (const auto &addr : ifaces) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol
                && !addr.isLoopback()) {
            return QStringLiteral("http://%1:%2").arg(addr.toString()).arg(m_port);
        }
    }
    return QStringLiteral("http://127.0.0.1:%1").arg(m_port);
}

// ─── New connection ──────────────────────────────────────────────────────────
void HttpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead,
                this,   &HttpServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected,
                this,   &HttpServer::onDisconnected);
    }
}

// ─── Read data ───────────────────────────────────────────────────────────────
// FIX: Mobile phones send large images split across many TCP segments.
// The original code called readAll() only once in onReadyRead() and then
// immediately tried to parse – causing truncated bodies and silent failures.
//
// Solution: accumulate every chunk into a per-socket buffer.  We only call
// handleRequest() when we have received all bytes declared in Content-Length,
// OR when the socket signals disconnected (for chunked / unknown length).
void HttpServer::onReadyRead()
{
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    m_buffers[socket] += socket->readAll();
    const QByteArray &buf = m_buffers[socket];

    // ── Determine if we have a complete request ───────────────────────────
    // First locate the end of the HTTP headers
    const int headerEnd = buf.indexOf("\r\n\r\n");
    if (headerEnd < 0) return; // headers not fully arrived yet

    // Extract Content-Length from headers (case-insensitive search)
    const QByteArray headers = buf.left(headerEnd).toLower();
    qsizetype contentLength  = -1;
    const int clIdx = headers.indexOf("content-length:");
    if (clIdx >= 0) {
        const int clEnd = headers.indexOf('\n', clIdx);
        const QByteArray clValue =
            buf.mid(clIdx + 15, clEnd - clIdx - 15).trimmed();
        bool ok = false;
        contentLength = clValue.toLongLong(&ok);
        if (!ok) contentLength = -1;
    }

    // How many body bytes have we received?
    const qsizetype bodyReceived = buf.size() - (headerEnd + 4);

    if (contentLength >= 0 && bodyReceived < contentLength) {
        // Body still arriving – wait for more data
        return;
    }

    // We have a complete (or length-less) request – process it
    QByteArray complete = buf;
    m_buffers.remove(socket);
    handleRequest(socket, complete);
}

void HttpServer::onDisconnected()
{
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    // If there is leftover data (no Content-Length header, e.g. GET), parse it
    if (m_buffers.contains(socket)) {
        QByteArray leftover = m_buffers.take(socket);
        if (!leftover.isEmpty())
            handleRequest(socket, leftover);
    }
    socket->deleteLater();
}

// ─── Parse and dispatch HTTP request ─────────────────────────────────────────
void HttpServer::handleRequest(QTcpSocket *socket, const QByteArray &data)
{
    const int firstLine = data.indexOf('\n');
    if (firstLine < 0) { sendResponse(socket, 400, "Bad Request"); return; }

    const QByteArray requestLine = data.left(firstLine).trimmed();
    const QList<QByteArray> parts = requestLine.split(' ');
    if (parts.size() < 2) { sendResponse(socket, 400, "Bad Request"); return; }

    const QByteArray method = parts[0].toUpper();
    const QByteArray path   = parts[1];

    // ── OPTIONS pre-flight (required by modern Android browsers) ──────────
    if (method == "OPTIONS") {
        sendCorsOk(socket);
        return;
    }

    // ── GET / or /upload → serve the upload page ──────────────────────────
    if (method == "GET" && (path == "/" || path == "/upload")) {
        sendHtml(socket, m_uploadPageHtml);
        return;
    }

    // ── POST /upload → receive the image ─────────────────────────────────
    if (method == "POST" && path == "/upload") {
        const int headerEnd = data.indexOf("\r\n\r\n");
        if (headerEnd < 0) { sendResponse(socket, 400, "Bad Request"); return; }

        const QByteArray rawHeaders = data.left(headerEnd);
        const QByteArray body       = data.mid(headerEnd + 4);

        // FIX: Extract boundary case-insensitively from header *name* but
        // preserve the exact value (boundary string is case-sensitive per RFC).
        QByteArray boundary;
        for (const auto &hdr : rawHeaders.split('\n')) {
            const QByteArray hdrTrimmed = hdr.trimmed();
            if (hdrTrimmed.toLower().startsWith("content-type:")
                    && hdrTrimmed.toLower().contains("boundary=")) {
                const QByteArray lower = hdrTrimmed.toLower();
                const int bi = lower.indexOf("boundary=") + 9;
                // Use original (non-lowercased) header for the actual value
                boundary = "--" + hdrTrimmed.mid(bi).trimmed();
                break;
            }
        }

        if (boundary.isEmpty()) {
            sendResponse(socket, 400, "Missing boundary");
            return;
        }

        QString    fileName;
        const QByteArray imageData =
            extractImageFromMultipart(body, boundary, fileName);

        if (imageData.isEmpty()) {
            sendResponse(socket, 400, "No image found in request");
            return;
        }

        // Success page
        const QByteArray successHtml = R"(
<!DOCTYPE html><html lang="ar" dir="rtl">
<head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body{font-family:sans-serif;background:#1a2236;color:#fff;
     display:flex;align-items:center;justify-content:center;
     height:100vh;margin:0;text-align:center}
.box{background:#22304a;padding:2rem;border-radius:1rem;
     box-shadow:0 4px 32px rgba(0,0,0,.4)}
.icon{font-size:4rem;margin-bottom:1rem}
h2{color:#4ade80;margin:0 0 .5rem}
p{color:#94a3b8;margin:0 0 1.5rem}
a{background:#2563eb;color:#fff;padding:.7rem 1.5rem;
  border-radius:.5rem;text-decoration:none;font-weight:bold}
</style></head><body>
<div class="box">
  <div class="icon">&#x2705;</div>
  <h2>&#x062a;&#x0645; &#x0627;&#x0633;&#x062a;&#x0644;&#x0627;&#x0645; &#x0627;&#x0644;&#x0641;&#x0627;&#x062a;&#x0648;&#x0631;&#x0629;!</h2>
  <p>&#x062c;&#x0627;&#x0631;&#x064a; &#x0627;&#x0644;&#x0645;&#x0639;&#x0627;&#x0644;&#x062c;&#x0629; &#x0639;&#x0644;&#x0649; &#x0627;&#x0644;&#x0643;&#x0645;&#x0628;&#x064a;&#x0648;&#x062a;&#x0631;...</p>
  <a href="/">&#x0631;&#x0641;&#x0639; &#x0641;&#x0627;&#x062a;&#x0648;&#x0631;&#x0629; &#x0623;&#x062e;&#x0631;&#x0649;</a>
</div></body></html>)";

        sendHtml(socket, successHtml);
        emit imageReceived(imageData, fileName);
        return;
    }

    sendResponse(socket, 404, "Not Found");
}

// ─── Extract image from multipart/form-data ──────────────────────────────────
QByteArray HttpServer::extractImageFromMultipart(const QByteArray &body,
                                                const QByteArray &boundary,
                                                QString          &outFileName)
{
    outFileName = QStringLiteral("invoice_%1.jpg")
                      .arg(QDateTime::currentSecsSinceEpoch());

    int partStart = body.indexOf(boundary);
    while (partStart >= 0) {
        const int partEnd = body.indexOf(boundary, partStart + boundary.size());
        const QByteArray part =
            (partEnd > 0)
                ? body.mid(partStart + boundary.size(),
                           partEnd - partStart - boundary.size())
                : body.mid(partStart + boundary.size());

        if (part.trimmed().startsWith("--")) break;

        const int headerEnd = part.indexOf("\r\n\r\n");
        if (headerEnd < 0) {
            partStart = body.indexOf(boundary, partStart + 1);
            continue;
        }

        // Compare headers case-insensitively
        const QByteArray partHeadersLower = part.left(headerEnd).toLower();
        const QByteArray partHeadersOrig  = part.left(headerEnd);
        const QByteArray partBody         = part.mid(headerEnd + 4);

        // Remove trailing CRLF that the multipart boundary adds
        QByteArray imageData = partBody;
        if (imageData.endsWith("\r\n"))
            imageData.chop(2);

        if (partHeadersLower.contains("content-type: image")
                || partHeadersLower.contains("name=\"file\"")
                || partHeadersLower.contains("name=\"image\"")) {

            // Try to extract the original filename
            const int fnIdx = partHeadersLower.indexOf("filename=\"");
            if (fnIdx >= 0) {
                const int fnEnd = partHeadersOrig.indexOf('"', fnIdx + 10);
                if (fnEnd > fnIdx + 10) {
                    const QString extracted =
                        QString::fromUtf8(partHeadersOrig.mid(fnIdx + 10,
                                                              fnEnd - fnIdx - 10));
                    if (!extracted.isEmpty())
                        outFileName = extracted;
                }
            }
            return imageData;
        }
        partStart = body.indexOf(boundary, partStart + 1);
    }
    return {};
}

// ─── Response helpers ────────────────────────────────────────────────────────
void HttpServer::sendHtml(QTcpSocket *socket, const QByteArray &html, int statusCode)
{
    const QByteArray status = (statusCode == 200) ? "200 OK" : "500 Error";
    const QByteArray response =
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + QByteArray::number(html.size()) + "\r\n"
        + CORS_HEADERS +
        "Connection: close\r\n"
        "\r\n" + html;
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

void HttpServer::sendResponse(QTcpSocket *socket, int code, const QString &body)
{
    const QByteArray bodyBytes = body.toUtf8();
    const QByteArray status    =
        QByteArray::number(code) + " " +
        (code == 200 ? "OK" :
         code == 400 ? "Bad Request" :
         code == 404 ? "Not Found" : "Error");
    const QByteArray response =
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: " + QByteArray::number(bodyBytes.size()) + "\r\n"
        + CORS_HEADERS +
        "Connection: close\r\n"
        "\r\n" + bodyBytes;
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

// FIX: Handle OPTIONS pre-flight so MIUI/Chrome on Android doesn't block XHR
void HttpServer::sendCorsOk(QTcpSocket *socket)
{
    const QByteArray response =
        "HTTP/1.1 204 No Content\r\n"
        + CORS_HEADERS +
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n";
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

// ─── Load the upload HTML page ───────────────────────────────────────────────
void HttpServer::loadUploadPage()
{
    // Try the embedded resource first (requires resources.qrc to include the file)
    QFile f(QStringLiteral(":/web/upload"));
    if (f.open(QIODevice::ReadOnly)) {
        m_uploadPageHtml = f.readAll();
        qDebug() << "[HttpServer] upload page loaded from Qt resource";
        return;
    }
    // Fallback: load from the filesystem path next to the executable
    QFile f2(QStringLiteral("resources/web/upload"));
    if (f2.open(QIODevice::ReadOnly)) {
        m_uploadPageHtml = f2.readAll();
        qDebug() << "[HttpServer] upload page loaded from filesystem";
        return;
    }
    qWarning() << "[HttpServer] upload page not found in resources – using built-in fallback";
    // Minimal built-in fallback (the full page is in resources/web/upload)
    m_uploadPageHtml = R"(<!DOCTYPE html>
<html lang="ar" dir="rtl">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>الفولاذ — رفع فاتورة</title>
</head>
<body style="font-family:sans-serif;background:#0f172a;color:#e2e8f0;padding:2rem;text-align:center">
<h2>رفع فاتورة</h2>
<form method="POST" action="/upload" enctype="multipart/form-data">
  <input type="file" name="file" accept="image/*" capture="environment"><br><br>
  <button type="submit" style="padding:.8rem 2rem;background:#2563eb;color:#fff;border:none;border-radius:.5rem;font-size:1rem">إرسال</button>
</form>
</body>
</html>)";
}