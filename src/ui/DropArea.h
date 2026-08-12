#pragma once
#include <QFrame>

class QLabel;

// Drag-and-drop / click-to-browse zone for picking the invoice image.
class DropArea : public QFrame
{
    Q_OBJECT
public:
    explicit DropArea(QWidget *parent = nullptr);

    QString selectedPath() const { return m_selectedPath; }
    void clearSelection();

signals:
    void imageSelected(const QString &path);
    void invalidFile(const QString &reason);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void trySelect(const QString &path);
    void showPreview(const QString &path);

    QLabel *m_iconLabel  = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_hintLabel  = nullptr;
    QLabel *m_previewLabel = nullptr;
    QString m_selectedPath;

    static constexpr qint64 kMaxBytes = 5 * 1024 * 1024; // 5 Mo
};
