#include "DashboardPage.h"
#include "StatCard.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLocale>
#include <QDoubleValidator>
#include <QTimer>

DashboardPage::DashboardPage(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    buildUi();
    refresh();
}

void DashboardPage::buildUi()
{
    // ---- Top bar: "Nouvelle facture +" (left) / title + subtitle (right) ----
    m_newInvoiceBtn = new QPushButton(QString::fromUtf8("+  ") + tr("Nouvelle facture"), this);
    m_newInvoiceBtn->setObjectName("primaryButton");
    m_newInvoiceBtn->setCursor(Qt::PointingHandCursor);
    connect(m_newInvoiceBtn, &QPushButton::clicked, this, [this]() { emit newInvoiceRequested(); });

    m_titleLabel = new QLabel(tr("Tableau de bord"), this);
    m_titleLabel->setObjectName("pageTitle");

    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setObjectName("pageSubtitle");

    auto *titleLayout = new QVBoxLayout;
    titleLayout->setSpacing(2);
    titleLayout->addWidget(m_titleLabel, 0, Qt::AlignRight);
    titleLayout->addWidget(m_subtitleLabel, 0, Qt::AlignRight);

    auto *topBar = new QHBoxLayout;
    topBar->addWidget(m_newInvoiceBtn, 0, Qt::AlignLeft | Qt::AlignTop);
    topBar->addStretch();
    topBar->addLayout(titleLayout);

    // ---- Stat cards ----
    m_totalCountCard  = new StatCard(QString::fromUtf8("\U0001F4CB"), tr("Total factures"), this);
    m_totalAmountCard = new StatCard(QString::fromUtf8("\U0001F4B0"), tr("Montant total"), this);
    m_lastInvoiceCard = new StatCard(QString::fromUtf8("\U0001F4C5"), tr("Dernière facture"), this);

    auto *cardsLayout = new QHBoxLayout;
    cardsLayout->setSpacing(16);
    cardsLayout->addWidget(m_totalCountCard);
    cardsLayout->addWidget(m_totalAmountCard);
    cardsLayout->addWidget(m_lastInvoiceCard);

    // ---- Filter panel ----
    auto *filterFrame = new QFrame(this);
    filterFrame->setObjectName("filterFrame");

    auto *filterHeader = new QLabel(tr("Recherche & Filtres  \U0001F39B"), this);
    filterHeader->setObjectName("filterHeader");
    auto *filterHint = new QLabel(
        tr("Recherchez par téléphone, nom, CIN, numéro de facture ou filtrez par date/montant."), this);
    filterHint->setObjectName("filterHint");

    m_amountMaxEdit = new QLineEdit(this);
    m_amountMaxEdit->setPlaceholderText(QString::fromUtf8("\u221E"));
    m_amountMaxEdit->setValidator(new QDoubleValidator(0, 1e12, 3, m_amountMaxEdit));

    m_amountMinEdit = new QLineEdit(this);
    m_amountMinEdit->setPlaceholderText("0");
    m_amountMinEdit->setValidator(new QDoubleValidator(0, 1e12, 3, m_amountMinEdit));

    m_dateFromEdit = new QDateEdit(this);
    m_dateFromEdit->setDisplayFormat("dd/MM/yyyy");
    m_dateFromEdit->setSpecialValueText(" ");
    m_dateFromEdit->setDate(m_dateFromEdit->minimumDate());
    m_dateFromEdit->setCalendarPopup(true);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Téléphone, nom, CIN, N° facture..."));

    auto labeled = [this](const QString &label, QWidget *field) {
        auto *box = new QVBoxLayout;
        auto *l = new QLabel(label, this);
        l->setObjectName("fieldLabel");
        box->addWidget(l, 0, Qt::AlignRight);
        box->addWidget(field);
        return box;
    };

    auto *filterGrid = new QHBoxLayout;
    filterGrid->setSpacing(16);
    filterGrid->addLayout(labeled(tr("Rechercher"), m_searchEdit));
    filterGrid->addLayout(labeled(tr("Date début"), m_dateFromEdit));
    filterGrid->addLayout(labeled(tr("Montant min"), m_amountMinEdit));
    filterGrid->addLayout(labeled(tr("Montant max"), m_amountMaxEdit));

    auto *filterLayout = new QVBoxLayout(filterFrame);
    filterLayout->addWidget(filterHeader, 0, Qt::AlignRight);
    filterLayout->addWidget(filterHint, 0, Qt::AlignRight);
    filterLayout->addSpacing(8);
    filterLayout->addLayout(filterGrid);

    // ---- Table ----
    m_table = new QTableWidget(0, 7, this);
    m_table->setObjectName("invoiceTable");
    m_table->setHorizontalHeaderLabels(
        {tr("N° Facture"), tr("Date"), tr("Client"), tr("Tél"), tr("CIN"), tr("Montant"), tr("Actions")});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);

    auto *tableContainer = new QVBoxLayout;
    tableContainer->addWidget(filterFrame);
    tableContainer->addWidget(m_table);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(20);
    mainLayout->addLayout(topBar);
    mainLayout->addLayout(cardsLayout);
    mainLayout->addLayout(tableContainer);

    // Debounced live filtering.
    auto *debounce = new QTimer(this);
    debounce->setSingleShot(true);
    debounce->setInterval(300);
    connect(debounce, &QTimer::timeout, this, &DashboardPage::applyFilters);

    connect(m_searchEdit, &QLineEdit::textChanged, this, [debounce]() { debounce->start(); });
    connect(m_amountMinEdit, &QLineEdit::textChanged, this, [debounce]() { debounce->start(); });
    connect(m_amountMaxEdit, &QLineEdit::textChanged, this, [debounce]() { debounce->start(); });
    connect(m_dateFromEdit, &QDateEdit::dateChanged, this, [this]() { applyFilters(); });
}

void DashboardPage::applyFilters()
{
    InvoiceFilter filter;
    filter.searchText = m_searchEdit->text().trimmed();
    if (!m_amountMinEdit->text().trimmed().isEmpty())
        filter.amountMin = m_amountMinEdit->text().toDouble();
    if (!m_amountMaxEdit->text().trimmed().isEmpty())
        filter.amountMax = m_amountMaxEdit->text().toDouble();
    if (m_dateFromEdit->date() != m_dateFromEdit->minimumDate())
        filter.dateFrom = m_dateFromEdit->date();

    const QVector<Invoice> invoices = m_db->listInvoices(filter);

    QLocale locale(QLocale::French);
    m_table->setRowCount(invoices.size());
    for (int row = 0; row < invoices.size(); ++row) {
        const Invoice &inv = invoices[row];
        auto setCell = [this, row](int col, const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(row, col, item);
        };
        setCell(0, inv.invoiceNumber);
        setCell(1, inv.invoiceDate.isValid() ? inv.invoiceDate.toString("dd/MM/yyyy") : "-");
        setCell(2, inv.clientName.isEmpty() ? "-" : inv.clientName);
        setCell(3, inv.clientPhone.isEmpty() ? "-" : inv.clientPhone);
        setCell(4, inv.clientCin.isEmpty() ? "-" : inv.clientCin);
        setCell(5, QString("%1 %2").arg(inv.currency, locale.toString(inv.totalAmount, 'f', 2)));
        // Actions: edit + delete buttons
        auto *actions = new QWidget(this);
        auto *hl = new QHBoxLayout(actions);
        hl->setContentsMargins(4, 0, 4, 0);
        hl->setSpacing(6);
        QPushButton *editBtn = new QPushButton(QString::fromUtf8("✏"), actions);
        editBtn->setToolTip(tr("Modifier"));
        editBtn->setFixedSize(28, 28);
        editBtn->setObjectName("tableEditButton");
        QPushButton *deleteBtn = new QPushButton(QString::fromUtf8("🗑"), actions);
        deleteBtn->setToolTip(tr("Supprimer"));
        deleteBtn->setFixedSize(28, 28);
        deleteBtn->setObjectName("tableDeleteButton");
        hl->addWidget(editBtn);
        hl->addWidget(deleteBtn);
        hl->addStretch();
        m_table->setCellWidget(row, 6, actions);

        // capture invoice id for signals
        const int invId = inv.id;
        connect(editBtn, &QPushButton::clicked, this, [this, invId]() { emit editInvoiceRequested(invId); });
        connect(deleteBtn, &QPushButton::clicked, this, [this, invId]() { emit deleteInvoiceRequested(invId); });
    }
}

void DashboardPage::refresh()
{
    const DashboardStats s = m_db->stats();
    QLocale locale(QLocale::French);

    m_totalCountCard->setValue(QString::number(s.invoiceCount));
    m_totalAmountCard->setValue(locale.toString(s.totalAmount, 'f', 2) + " TND");
    m_lastInvoiceCard->setValue(s.lastInvoiceDate.isValid()
                                     ? s.lastInvoiceDate.toString("dd/MM/yyyy")
                                     : "-");

    m_subtitleLabel->setText(tr("facture(s) enregistrée(s) : %1").arg(s.invoiceCount));

    applyFilters();
}
