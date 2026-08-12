#pragma once
#include <QFrame>

class QLabel;

// Small rounded card used on the dashboard for "Dernière facture",
// "Montant total", "Total factures".
class StatCard : public QFrame
{
    Q_OBJECT
public:
    explicit StatCard(const QString &iconGlyph, const QString &title, QWidget *parent = nullptr);

    void setValue(const QString &value);

private:
    QLabel *m_valueLabel = nullptr;
};
