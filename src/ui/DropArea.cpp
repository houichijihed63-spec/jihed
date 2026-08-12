#include "DropArea.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QPixmap>

DropArea::DropArea(QWidget *parent) : QFrame(parent)
{
    setObjectName("dropArea");
    setAcceptDrops(true);
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(220);

    m_iconLabel = new QLabel(QString::fromUtf8("\u2B06"), this); // upload arrow glyph
    m_iconLabel->setObjectName("dropAreaIcon");
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(56, 56);

    m_titleLabel = new QLabel(tr("Glissez-déposez une image ou cliquez pour choisir"), this);
    m_titleLabel->setObjectName("dropAreaTitle");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_hintLabel = new QLabel(tr("PNG, JPG, JPEG, WEBP — max 5 Mo"), this);
    m_hintLabel->setObjectName("dropAreaHint");
    m_hintLabel->setAlignment(Qt::AlignCenter);

    m_previewLabel = new QLabel(this);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setVisible(false);
    m_previewLabel->setMaximumHeight(140);

    auto *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_previewLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(8);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_hintLabel);
    layout->addStretch();
}

void DropArea::clearSelection()
{
    m_selectedPath.clear();
    m_previewLabel->setVisible(false);
    m_iconLabel->setVisible(true);
    m_titleLabel->setText(tr("Glissez-déposez une image ou cliquez pour choisir"));
    m_hintLabel->setText(tr("PNG, JPG, JPEG, WEBP — max 5 Mo"));
}

void DropArea::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void DropArea::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty())
        trySelect(urls.first().toLocalFile());
}

void DropArea::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    const QString path = QFileDialog::getOpenFileName(
        this, tr("اختر صورة الفاتورة"), QString(),
        tr("Images (*.png *.jpg *.jpeg *.webp)"));
    if (!path.isEmpty())
        trySelect(path);
}

void DropArea::trySelect(const QString &path)
{
    const QFileInfo fi(path);
    static const QStringList kAllowed = {"png", "jpg", "jpeg", "webp"};

    if (!fi.exists() || !kAllowed.contains(fi.suffix().toLower())) {
        emit invalidFile(tr("صيغة الملف غير مدعومة. الصيغ المقبولة: PNG, JPG, JPEG, WEBP"));
        return;
    }
    if (fi.size() > kMaxBytes) {
        emit invalidFile(tr("حجم الملف يتجاوز 5 ميغابايت."));
        return;
    }

    m_selectedPath = path;
    showPreview(path);
    emit imageSelected(path);
}

void DropArea::showPreview(const QString &path)
{
    const QPixmap pixmap(path);
    if (!pixmap.isNull()) {
        m_previewLabel->setPixmap(pixmap.scaledToHeight(120, Qt::SmoothTransformation));
        m_previewLabel->setVisible(true);
        m_iconLabel->setVisible(false);
    }
    m_titleLabel->setText(QFileInfo(path).fileName());
    m_hintLabel->setText(tr("انقر لاختيار صورة أخرى"));
}
