#include "StatCard.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

// ─── مُنشئ ───────────────────────────────────────────────────────────────────
// بطاقة إحصائية صغيرة: أيقونة (يمين) + عنوان وقيمة (يسار)، بأسلوب مطابق
// لمحددات style.qss: #statCard / #statCardIcon / #statCardTitle / #statCardValue
StatCard::StatCard(const QString &iconGlyph, const QString &title, QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("statCard"));
    setFrameShape(QFrame::NoFrame);
    setMinimumHeight(88);

    // ─── الأيقونة ────────────────────────────────────────────────
    auto *iconLabel = new QLabel(iconGlyph, this);
    iconLabel->setObjectName(QStringLiteral("statCardIcon"));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(44, 44);

    // ─── العنوان + القيمة ────────────────────────────────────────
    auto *titleLabel = new QLabel(title, this);
    titleLabel->setObjectName(QStringLiteral("statCardTitle"));

    m_valueLabel = new QLabel(QStringLiteral("—"), this);
    m_valueLabel->setObjectName(QStringLiteral("statCardValue"));

    auto *textLayout = new QVBoxLayout;
    textLayout->setSpacing(2);
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(m_valueLabel);

    // ─── التخطيط العام ───────────────────────────────────────────
    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(16, 14, 16, 14);
    rootLayout->setSpacing(12);
    rootLayout->addWidget(iconLabel, 0, Qt::AlignTop);
    rootLayout->addLayout(textLayout, 1);
}

// ─── تحديث القيمة المعروضة ───────────────────────────────────────────────────
void StatCard::setValue(const QString &value)
{
    m_valueLabel->setText(value);
}