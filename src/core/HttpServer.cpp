#include "HttpServer.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QDir>

// ─── مُنشئ ───────────────────────────────────────────────────────────────────
HttpServer::HttpServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection,
            this,     &HttpServer::onNewConnection);
    loadUploadPage();
}

HttpServer::~HttpServer()
{
    stop();
}

// ─── تشغيل/إيقاف ─────────────────────────────────────────────────────────────
bool HttpServer::start(quint16 port)
{
    m_port = port;
    if (!m_server->listen(QHostAddress::AnyIPv4, port)) {
        emit serverError(tr("فشل بدء الخادم على المنفذ %1: %2")
                             .arg(port).arg(m_server->errorString()));
        return false;
    }
    qDebug() << "[HttpServer] يستمع على:" << localUrl();
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

// ─── عنوان URL المحلي ─────────────────────────────────────────────────────────
QString HttpServer::localUrl() const
{
    // البحث عن أول IPv4 غير loopback
    const auto ifaces = QNetworkInterface::allAddresses();
    for (const auto &addr : ifaces) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol
            && !addr.isLoopback()) {
            return QStringLiteral("http://%1:%2").arg(addr.toString()).arg(m_port);
        }
    }
    return QStringLiteral("http://127.0.0.1:%1").arg(m_port);
}

// ─── معالجة الاتصالات ─────────────────────────────────────────────────────────
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

void HttpServer::onReadyRead()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    const QByteArray data = socket->readAll();
    handleRequest(socket, data);
}

void HttpServer::onDisconnected()
{
    if (auto *socket = qobject_cast<QTcpSocket*>(sender()))
        socket->deleteLater();
}

// ─── تحليل الطلب HTTP ─────────────────────────────────────────────────────────
void HttpServer::handleRequest(QTcpSocket *socket, const QByteArray &data)
{
    // سطر الطلب الأول: "GET / HTTP/1.1"
    const int firstLine = data.indexOf('\n');
    if (firstLine < 0) { sendResponse(socket, 400, "Bad Request"); return; }

    const QByteArray requestLine = data.left(firstLine).trimmed();
    const QList<QByteArray> parts = requestLine.split(' ');
    if (parts.size() < 2) { sendResponse(socket, 400, "Bad Request"); return; }

    const QByteArray method = parts[0];
    const QByteArray path   = parts[1];

    // ─── GET / → صفحة الرفع ───────────────────────────────────────
    if (method == "GET" && (path == "/" || path == "/upload")) {
        sendHtml(socket, m_uploadPageHtml);
        return;
    }

    // ─── POST /upload → استلام الصورة ─────────────────────────────
    if (method == "POST" && path == "/upload") {
        // استخراج حدود multipart
        const int headerEnd = data.indexOf("\r\n\r\n");
        if (headerEnd < 0) { sendResponse(socket, 400, "Bad Request"); return; }

        const QByteArray headers = data.left(headerEnd);
        const QByteArray body    = data.mid(headerEnd + 4);

        // إيجاد boundary
        QByteArray boundary;
        for (const auto &hdr : headers.split('\n')) {
            const QByteArray h = hdr.trimmed().toLower();
            if (h.startsWith("content-type:") && h.contains("boundary=")) {
                const int bi = h.indexOf("boundary=") + 9;
                boundary = "--" + h.mid(bi).trimmed();
                break;
            }
        }

        if (boundary.isEmpty()) {
            sendResponse(socket, 400, "Missing boundary");
            return;
        }

        QString fileName;
        const QByteArray imageData =
            extractImageFromMultipart(body, boundary, fileName);

        if (imageData.isEmpty()) {
            sendResponse(socket, 400, "لم يتم العثور على صورة في الطلب");
            return;
        }

        // إرسال استجابة نجاح
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
  <div class="icon">✅</div>
  <h2>تم استلام الفاتورة!</h2>
  <p>جاري المعالجة على الكمبيوتر...</p>
  <a href="/">رفع فاتورة أخرى</a>
</div></body></html>)";

        sendHtml(socket, successHtml);

        // إطلاق الإشارة للتطبيق
        emit imageReceived(imageData, fileName);
        return;
    }

    // ─── 404 ───────────────────────────────────────────────────────
    sendResponse(socket, 404, "Not Found");
}

// ─── استخراج الصورة من multipart ─────────────────────────────────────────────
QByteArray HttpServer::extractImageFromMultipart(const QByteArray &body,
                                                  const QByteArray &boundary,
                                                  QString &outFileName)
{
    outFileName = QStringLiteral("invoice_%1.jpg")
                      .arg(QDateTime::currentSecsSinceEpoch());

    // تقسيم الجسم عند حدود multipart
    int partStart = body.indexOf(boundary);
    while (partStart >= 0) {
        const int partEnd = body.indexOf(boundary, partStart + boundary.size());
        const QByteArray part = (partEnd > 0)
                                    ? body.mid(partStart + boundary.size(),
                                               partEnd - partStart - boundary.size())
                                    : body.mid(partStart + boundary.size());

        // انتهى عند "--"
        if (part.trimmed().startsWith("--")) break;

        const int headerEnd = part.indexOf("\r\n\r\n");
        if (headerEnd < 0) { partStart = body.indexOf(boundary, partStart+1); continue; }

        const QByteArray partHeaders = part.left(headerEnd).toLower();
        const QByteArray partBody    = part.mid(headerEnd + 4).trimmed();

        // تحقق أن هذا الجزء يحتوي على صورة
        if (partHeaders.contains("content-type: image") ||
            partHeaders.contains("name=\"file\"") ||
            partHeaders.contains("name=\"image\"")) {

            // استخراج اسم الملف إن وُجد
            const int fnIdx = partHeaders.indexOf("filename=\"");
            if (fnIdx >= 0) {
                const int fnEnd = partHeaders.indexOf('"', fnIdx + 10);
                if (fnEnd > fnIdx + 10) {
                    outFileName = QString::fromUtf8(
                        partHeaders.mid(fnIdx + 10, fnEnd - fnIdx - 10));
                    if (outFileName.isEmpty())
                        outFileName = QStringLiteral("invoice_%1.jpg")
                                          .arg(QDateTime::currentSecsSinceEpoch());
                }
            }
            return partBody;
        }
        partStart = body.indexOf(boundary, partStart + 1);
    }
    return {};
}

// ─── إرسال استجابات HTTP ──────────────────────────────────────────────────────
void HttpServer::sendHtml(QTcpSocket *socket, const QByteArray &html, int statusCode)
{
    const QByteArray status = (statusCode == 200) ? "200 OK" : "500 Error";
    const QByteArray response =
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + QByteArray::number(html.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + html;
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

void HttpServer::sendResponse(QTcpSocket *socket, int code, const QString &body)
{
    const QByteArray bodyBytes = body.toUtf8();
    const QByteArray status    = QByteArray::number(code) + " " +
                                 (code == 200 ? "OK" :
                                  code == 400 ? "Bad Request" :
                                  code == 404 ? "Not Found" : "Error");
    const QByteArray response =
        "HTTP/1.1 " + status + "\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: " + QByteArray::number(bodyBytes.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + bodyBytes;
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

// ─── تحميل صفحة الرفع HTML ───────────────────────────────────────────────────
void HttpServer::loadUploadPage()
{
    // القراءة من الموارد المضمّنة
    QFile f(QStringLiteral(":/web/upload.html"));
    if (f.open(QIODevice::ReadOnly)) {
        m_uploadPageHtml = f.readAll();
        return;
    }
    // احتياطي: صفحة مضمّنة مباشرة في الكود
    m_uploadPageHtml = R"(<!DOCTYPE html>
<html lang="ar" dir="rtl">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<title>الفولاذ — رفع فاتورة</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:'Segoe UI',Arial,sans-serif;background:#0f172a;
     color:#e2e8f0;min-height:100vh;padding:1rem}
.header{text-align:center;padding:1.5rem 0 1rem}
.logo{width:80px;height:80px;border-radius:50%;
      border:3px solid #ef4444;margin:0 auto .75rem;display:block}
h1{font-size:1.4rem;color:#f1f5f9;margin-bottom:.25rem}
.subtitle{color:#64748b;font-size:.9rem}
.card{background:#1e293b;border-radius:1rem;padding:1.5rem;
      margin:.75rem 0;border:1px solid #334155}
.drop-area{border:2px dashed #3b82f6;border-radius:.75rem;
           padding:2rem 1rem;text-align:center;cursor:pointer;
           transition:all .3s;background:#0f172a}
.drop-area.drag-over{border-color:#60a5fa;background:#1e3a5f}
.drop-area input{display:none}
.drop-icon{font-size:3rem;margin-bottom:.75rem}
.drop-text{color:#94a3b8;font-size:.95rem;line-height:1.6}
.drop-text strong{color:#60a5fa}
.preview{display:none;margin-top:1rem}
.preview img{width:100%;border-radius:.5rem;max-height:250px;
             object-fit:contain;background:#0f172a}
.preview .file-info{margin-top:.5rem;font-size:.85rem;color:#64748b;
                    text-align:center}
.btn{display:block;width:100%;padding:1rem;margin-top:1rem;
     background:linear-gradient(135deg,#2563eb,#1d4ed8);
     color:#fff;border:none;border-radius:.75rem;font-size:1.05rem;
     font-weight:bold;cursor:pointer;transition:opacity .2s;
     font-family:inherit}
.btn:disabled{opacity:.5;cursor:not-allowed}
.btn:not(:disabled):hover{opacity:.9}
.progress{display:none;margin-top:1rem}
.progress-bar{height:8px;background:#1e3a5f;border-radius:4px;overflow:hidden}
.progress-fill{height:100%;width:0;background:linear-gradient(90deg,#3b82f6,#60a5fa);
               transition:width .3s;border-radius:4px}
.progress-text{text-align:center;margin-top:.5rem;font-size:.85rem;color:#64748b}
.status{display:none;padding:.75rem 1rem;border-radius:.5rem;
        text-align:center;margin-top:.75rem;font-weight:500}
.status.success{background:#14532d;color:#4ade80;border:1px solid #16a34a}
.status.error{background:#450a0a;color:#f87171;border:1px solid #dc2626}
.tips{margin-top:.5rem}
.tips h3{color:#94a3b8;font-size:.9rem;margin-bottom:.5rem}
.tips ul{list-style:none;padding:0}
.tips li{padding:.35rem 0;font-size:.85rem;color:#64748b;
         display:flex;align-items:center;gap:.5rem}
.tips li::before{content:'💡';font-size:.8rem}
</style>
</head>
<body>
<div class="header">
  <img class="logo"
       src="https://miaoda-conversation-file.s3cdn.medo.dev/user-dn6ofsr2tf5s/app-dnqz9anf7ny9/20260812/logo.png"
       alt="الفولاذ">
  <h1>رفع فاتورة</h1>
  <p class="subtitle">شركة الفولاذ — منزل بورقيبة</p>
</div>

<div class="card">
  <div class="drop-area" id="dropArea">
    <input type="file" id="fileInput" accept="image/*" capture="environment">
    <div class="drop-icon">📷</div>
    <div class="drop-text">
      <strong>اضغط هنا</strong> لالتقاط صورة بالكاميرا<br>
      أو اختر صورة من المعرض
    </div>
  </div>
  <div class="preview" id="preview">
    <img id="previewImg" src="" alt="معاينة الفاتورة">
    <div class="file-info" id="fileInfo"></div>
  </div>
  <div class="progress" id="progress">
    <div class="progress-bar"><div class="progress-fill" id="progressFill"></div></div>
    <div class="progress-text" id="progressText">جاري الرفع...</div>
  </div>
  <div class="status" id="statusMsg"></div>
  <button class="btn" id="uploadBtn" disabled>📤 رفع الفاتورة للكمبيوتر</button>
</div>

<div class="card tips">
  <h3>نصائح للحصول على نتيجة OCR أفضل:</h3>
  <ul>
    <li>التقط الصورة في إضاءة جيدة</li>
    <li>تأكد أن الفاتورة مستقيمة وواضحة</li>
    <li>لا تشمل خلفية كبيرة حول الفاتورة</li>
    <li>الدقة العالية = نتائج أدق</li>
  </ul>
</div>

<script>
const dropArea    = document.getElementById('dropArea');
const fileInput   = document.getElementById('fileInput');
const previewEl   = document.getElementById('preview');
const previewImg  = document.getElementById('previewImg');
const fileInfoEl  = document.getElementById('fileInfo');
const progressEl  = document.getElementById('progress');
const progressFill= document.getElementById('progressFill');
const progressText= document.getElementById('progressText');
const statusMsg   = document.getElementById('statusMsg');
const uploadBtn   = document.getElementById('uploadBtn');
let selectedFile  = null;

// ─── اختيار الملف ───────────────────────────────────────────────
dropArea.addEventListener('click', () => fileInput.click());
dropArea.addEventListener('dragover',  e => { e.preventDefault(); dropArea.classList.add('drag-over'); });
dropArea.addEventListener('dragleave', () => dropArea.classList.remove('drag-over'));
dropArea.addEventListener('drop', e => {
  e.preventDefault(); dropArea.classList.remove('drag-over');
  if (e.dataTransfer.files.length) handleFile(e.dataTransfer.files[0]);
});
fileInput.addEventListener('change', () => {
  if (fileInput.files.length) handleFile(fileInput.files[0]);
});

function handleFile(file) {
  if (!file.type.startsWith('image/')) {
    showStatus('يرجى اختيار ملف صورة فقط', 'error'); return;
  }
  selectedFile = file;
  const reader = new FileReader();
  reader.onload = e => {
    previewImg.src = e.target.result;
    previewEl.style.display = 'block';
    const kb = (file.size / 1024).toFixed(0);
    fileInfoEl.textContent = file.name + ' (' + kb + ' كيلوبايت)';
    uploadBtn.disabled = false;
    statusMsg.style.display = 'none';
  };
  reader.readAsDataURL(file);
}

// ─── الرفع ──────────────────────────────────────────────────────
uploadBtn.addEventListener('click', () => {
  if (!selectedFile) return;
  const formData = new FormData();
  formData.append('file', selectedFile, selectedFile.name);

  uploadBtn.disabled = true;
  progressEl.style.display = 'block';
  statusMsg.style.display   = 'none';

  const xhr = new XMLHttpRequest();
  xhr.upload.addEventListener('progress', e => {
    if (e.lengthComputable) {
      const pct = Math.round(e.loaded / e.total * 100);
      progressFill.style.width = pct + '%';
      progressText.textContent  = 'جاري الرفع... ' + pct + '%';
    }
  });
  xhr.addEventListener('load', () => {
    progressEl.style.display = 'none';
    if (xhr.status === 200) {
      showStatus('✅ تم الإرسال! جاري المعالجة على الكمبيوتر...', 'success');
      // إعادة ضبط الواجهة بعد 4 ثوانٍ
      setTimeout(() => {
        selectedFile = null;
        previewEl.style.display = 'none';
        fileInput.value = '';
        uploadBtn.disabled = true;
        statusMsg.style.display = 'none';
      }, 4000);
    } else {
      showStatus('حدث خطأ أثناء الرفع. حاول مجدداً.', 'error');
      uploadBtn.disabled = false;
    }
  });
  xhr.addEventListener('error', () => {
    progressEl.style.display = 'none';
    showStatus('تعذر الاتصال بالكمبيوتر. تأكد من الاتصال بنفس الشبكة.', 'error');
    uploadBtn.disabled = false;
  });
  xhr.open('POST', '/upload');
  xhr.send(formData);
});

function showStatus(msg, type) {
  statusMsg.textContent   = msg;
  statusMsg.className     = 'status ' + type;
  statusMsg.style.display = 'block';
}
</script>
</body>
</html>)";
}
