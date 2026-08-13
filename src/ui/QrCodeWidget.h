#pragma once
#include <QWidget>
#include <QString>

/**
 * QrCodeWidget — renders a QR Code for the local server URL.
 *
 * FIX: The original custom QR encoder was a simplified, non-spec-compliant
 * implementation that produced unreadable codes for URLs longer than ~25
 * characters (e.g. http://192.168.100.59:8080).
 *
 * This version replaces the broken encoder with an approach that uses the
 * free, no-key-required QR Server API (api.qrserver.com) to generate a
 * proper PNG, then displays it via QNetworkAccessManager.
 * Falls back to a plain text URL label if the network is unavailable.
 */
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;

class QrCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QrCodeWidget(QWidget *parent = nullptr);

    void    setUrl(const QString &url);
    QString url() const { return m_url; }

    // Cell size kept for API compatibility; ignored (PNG size is fixed at 160px)
    void setCellSize(int /*size*/) {}

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onImageDownloaded(QNetworkReply *reply);

private:
    void fetchQrImage();

    QString                m_url;
    QPixmap                m_qrPixmap;
    QNetworkAccessManager *m_nam = nullptr;
    QLabel                *m_fallbackLabel = nullptr;
};