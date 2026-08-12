#pragma once
#include <QWidget>
#include "../core/Database.h"

class QPushButton;
class QLabel;
class QLineEdit;
class QDateEdit;
class QTableWidget;
class StatCard;

// "Tableau de bord" page: stat cards, search/filter panel, invoice table.
class DashboardPage : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardPage(Database *db, QWidget *parent = nullptr);

    void refresh();

signals:
    void newInvoiceRequested();
    void editInvoiceRequested(int invoiceId);
    void deleteInvoiceRequested(int invoiceId);

private:
    void buildUi();
    void applyFilters();

    Database *m_db = nullptr;

    QPushButton *m_newInvoiceBtn = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;

    StatCard *m_lastInvoiceCard = nullptr;
    StatCard *m_totalAmountCard = nullptr;
    StatCard *m_totalCountCard = nullptr;

    QLineEdit *m_amountMaxEdit = nullptr;
    QLineEdit *m_amountMinEdit = nullptr;
    QDateEdit *m_dateFromEdit = nullptr;
    QLineEdit *m_searchEdit = nullptr;

    QTableWidget *m_table = nullptr;
};
