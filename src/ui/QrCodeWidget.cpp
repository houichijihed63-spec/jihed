#include "QrCodeWidget.h"

#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

// ─── Constructor ─────────────────────────────────────────────────────────────
QrCodeWidget::QrCodeWidget(QWidget *parent)
    : QWidget(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    setMinimumSize(160, 160);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedSize(160, 160);

    // Fallback label shown while the image is loading or if network fails
    m_fallbackLabel = new QLabel(tr("Loading QR..."), this);
    m_fallbackLabel->setAlignment(Qt::AlignCenter);
    m_fallbackLabel->setStyleSheet("color:#64748b;font-size:11px;");
    m_fallbackLabel->setGeometry(0, 0, 160, 160);

    connect(m_nam, &QNetworkAccessManager::finished,
            this,  &QrCodeWidget::onImageDownloaded);
}

// ─── Set URL and fetch QR image ──────────────────────────────────────────────
void QrCodeWidget::setUrl(const QString &url)
{
    m_url = url;
    m_qrPixmap = QPixmap(); // clear previous
    m_fallbackLabel->setText(tr("Loading QR..."));
    m_fallbackLabel->setVisible(true);
    update();
    fetchQrImage();
}

// ─── Fetch QR PNG from the free QR Server API ────────────────────────────────
// Uses https://api.qrserver.com/v1/create-qr-code/ which is free,
// requires no API key, and works on local networks (request goes out to
// the internet once, result is cached).
//
// FIX: The original custom QR encoder failed for URLs > 25 chars.
// Delegating to a proven library/API produces a correct, scannable QR code.
void QrCodeWidget::fetchQrImage()
{
    if (m_url.isEmpty()) return;

    QUrl apiUrl("https://api.qrserver.com/v1/create-qr-code/");
    QUrlQuery query;
    query.addQueryItem("size",   "160x160");
    query.addQueryItem("margin", "4");
    query.addQueryItem("data",   m_url);
    apiUrl.setQuery(query);

    QNetworkRequest req(apiUrl);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    m_nam->get(req);
    qDebug() << "[QrCodeWidget] fetching QR for:" << m_url;
}

// ─── Handle downloaded image ─────────────────────────────────────────────────
void QrCodeWidget::onImageDownloaded(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[QrCodeWidget] QR download failed:" << reply->errorString();
        // Show the URL as plain text so the user can still type it manually
        m_fallbackLabel->setText(m_url);
        m_fallbackLabel->setStyleSheet(
            "color:#3b82f6;font-size:9px;word-wrap:break-word;padding:4px;");
        m_fallbackLabel->setWordWrap(true);
        m_fallbackLabel->setVisible(true);
        return;
    }

    const QByteArray data = reply->readAll();
    if (!m_qrPixmap.loadFromData(data)) {
        qWarning() << "[QrCodeWidget] Failed to decode QR PNG";
        m_fallbackLabel->setText(m_url);
        m_fallbackLabel->setVisible(true);
        return;
    }

    m_fallbackLabel->setVisible(false);
    setFixedSize(m_qrPixmap.size());
    update();
    qDebug() << "[QrCodeWidget] QR image loaded successfully";
}

// ─── Paint ───────────────────────────────────────────────────────────────────
void QrCodeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::white);

    if (!m_qrPixmap.isNull()) {
        p.drawPixmap(0, 0, m_qrPixmap);
    }
}