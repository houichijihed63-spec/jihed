#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QVector>
#include <QString>

// One word read by the OCR engine, with its bounding box in image pixels.
// Coordinates are used later by TableExtractor to rebuild table columns.
struct OcrWord {
    QString text;
    double  left   = 0.0;
    double  top    = 0.0;
    double  width  = 0.0;
    double  height = 0.0;

    double right()    const { return left + width; }
    double bottom()   const { return top + height; }
    double centerX()  const { return left + width / 2.0; }
    double centerY()  const { return top + height / 2.0; }
};

struct OcrResult {
    bool             success = false;
    QString          errorMessage;
    QString          rawText;
    QVector<OcrWord> words;
};

// Thin wrapper around the OCR.space "parse/image" endpoint.
// Sends the image as multipart/form-data and asks for word-level
// coordinates (isOverlayRequired=true) so the table layout can be rebuilt.
class OcrClient : public QObject
{
    Q_OBJECT
public:
    explicit OcrClient(QObject *parent = nullptr);

    void setApiKey(const QString &key);
    QString apiKey() const;

    // Reads settings-stored key if none was set explicitly.
    QString effectiveApiKey() const;

    void extractFromImage(const QString &imagePath);

signals:
    void finished(OcrResult result);
    void progress(const QString &message);

private:
    QNetworkAccessManager m_manager;
    QString m_apiKey;

    OcrResult parseResponse(const QByteArray &json, bool httpOk) const;
};
