#include "OcrClient.h"

#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QUrl>

static const char *kOcrEndpoint = "https://api.ocr.space/parse/image";

OcrClient::OcrClient(QObject *parent) : QObject(parent) {}

void OcrClient::setApiKey(const QString &key) { m_apiKey = key; }
QString OcrClient::apiKey() const { return m_apiKey; }

QString OcrClient::effectiveApiKey() const
{
    if (!m_apiKey.isEmpty())
        return m_apiKey;
    QSettings settings;
    return settings.value("ocr/apiKey").toString();
}

void OcrClient::extractFromImage(const QString &imagePath)
{
    OcrResult failure;
    failure.success = false;

    QFileInfo fi(imagePath);
    if (!fi.exists()) {
        failure.errorMessage = QObject::tr("الصورة غير موجودة: %1").arg(imagePath);
        emit finished(failure);
        return;
    }

    const QString key = effectiveApiKey();
    if (key.isEmpty()) {
        failure.errorMessage = QObject::tr("لم يتم ضبط مفتاح OCR.space API.");
        emit finished(failure);
        return;
    }

    QFile *file = new QFile(imagePath);
    if (!file->open(QIODevice::ReadOnly)) {
        failure.errorMessage = QObject::tr("تعذّر فتح ملف الصورة.");
        emit finished(failure);
        delete file;
        return;
    }

    emit progress(QObject::tr("جارٍ إرسال الصورة إلى محرك OCR..."));

    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    auto textPart = [](const QString &name, const QString &value) {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant(QString("form-data; name=\"%1\"").arg(name)));
        part.setBody(value.toUtf8());
        return part;
    };

    multiPart->append(textPart("apikey", key));
    multiPart->append(textPart("language", "fre"));
    multiPart->append(textPart("isOverlayRequired", "true"));
    multiPart->append(textPart("OCREngine", "2"));
    multiPart->append(textPart("scale", "true"));
    multiPart->append(textPart("detectOrientation", "true"));

    QHttpPart imagePart;
    const QString mime = fi.suffix().toLower() == "png" ? "image/png" : "image/jpeg";
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(mime));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                         QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fi.fileName())));
    file->setParent(multiPart);
    imagePart.setBodyDevice(file);
    multiPart->append(imagePart);

    QNetworkRequest request{QUrl(kOcrEndpoint)};
    QNetworkReply *reply = m_manager.post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const bool httpOk = (reply->error() == QNetworkReply::NoError);
        const QByteArray body = reply->readAll();
        OcrResult result = parseResponse(body, httpOk);
        if (!httpOk && result.errorMessage.isEmpty())
            result.errorMessage = reply->errorString();
        reply->deleteLater();
        emit finished(result);
    });
}

OcrResult OcrClient::parseResponse(const QByteArray &json, bool httpOk) const
{
    OcrResult result;
    if (!httpOk) {
        result.success = false;
        return result;
    }

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        result.success = false;
        result.errorMessage = QObject::tr("استجابة OCR غير صالحة.");
        return result;
    }

    const QJsonObject root = doc.object();
    if (root.value("IsErroredOnProcessing").toBool(false)) {
        result.success = false;
        const QJsonValue errVal = root.value("ErrorMessage");
        if (errVal.isArray() && !errVal.toArray().isEmpty())
            result.errorMessage = errVal.toArray().first().toString();
        else
            result.errorMessage = errVal.toString(QObject::tr("خطأ غير معروف من OCR.space"));
        return result;
    }

    const QJsonArray parsedResults = root.value("ParsedResults").toArray();
    if (parsedResults.isEmpty()) {
        result.success = false;
        result.errorMessage = QObject::tr("لم يتم العثور على نص في الصورة.");
        return result;
    }

    const QJsonObject firstResult = parsedResults.first().toObject();
    result.rawText = firstResult.value("ParsedText").toString();

    const QJsonObject overlay = firstResult.value("TextOverlay").toObject();
    const QJsonArray lines = overlay.value("Lines").toArray();
    for (const QJsonValue &lineVal : lines) {
        const QJsonArray words = lineVal.toObject().value("Words").toArray();
        for (const QJsonValue &wordVal : words) {
            const QJsonObject w = wordVal.toObject();
            OcrWord word;
            word.text   = w.value("WordText").toString();
            word.left   = w.value("Left").toDouble();
            word.top    = w.value("Top").toDouble();
            word.width  = w.value("Width").toDouble();
            word.height = w.value("Height").toDouble();
            if (!word.text.trimmed().isEmpty())
                result.words << word;
        }
    }

    result.success = true;
    return result;
}
