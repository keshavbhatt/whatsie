// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
#include "MoreAppsView.h"

#include "AppCard.h"
#include "AppCatalog.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace ktechpit {

MoreAppsView::MoreAppsView(const QString& place, int columns, QWidget* parent)
    : QWidget(parent)
    , m_place(place)
    , m_columns(qMax(1, columns))
{
    setObjectName(u"ktechpitMoreAppsView"_s);

    m_intro = new QLabel(tr("Other apps by the same developer. Get opens your software centre."), this);
    m_intro->setObjectName(u"ktechpitMoreAppsIntro"_s);
    m_intro->setWordWrap(true);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(tr("Search apps"));
    m_search->setAccessibleName(tr("Search apps"));
    m_search->setClearButtonEnabled(true);
    connect(m_search, &QLineEdit::textChanged, this, &MoreAppsView::rebuild);

    // Refresh: fetch the list now instead of waiting for the weekly update.
    m_refresh = new QPushButton(tr("Refresh"), this);
    m_refresh->setObjectName(u"ktechpitMoreAppsRefresh"_s);
    m_refresh->setToolTip(tr("Get the latest list of apps from ktechpit.com"));
    AppCatalog* catalog = AppCatalog::instance();
    connect(m_refresh, &QPushButton::clicked, this, [this, catalog] {
        m_refresh->setEnabled(false);
        setStatus(tr("Getting the latest list…"));
        catalog->refresh();
    });
    connect(catalog, &AppCatalog::refreshFinished, this, [this](bool ok) {
        m_refresh->setEnabled(true);
        const int apps = static_cast<int>(AppCatalog::instance()->allApps().size());
        setStatus(!ok        ? tr("Could not reach ktechpit.com; showing the saved list.")
                  : apps == 1 ? tr("Up to date: 1 app.")
                              : tr("Up to date: %1 apps.").arg(apps));
    });

    auto* searchRow = new QHBoxLayout;
    searchRow->setContentsMargins(0, 0, 0, 0);
    searchRow->setSpacing(8);
    searchRow->addWidget(m_search, 1);
    searchRow->addWidget(m_refresh);

    m_grid = new QWidget;
    m_gridLayout = new QGridLayout(m_grid);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(8);
    for (int c = 0; c < m_columns; ++c) {
        m_gridLayout->setColumnStretch(c, 1); // every column as wide as the others
    }

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // the columns share the width
    scroll->setWidget(m_grid);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    layout->addWidget(m_intro);
    layout->addLayout(searchRow);
    layout->addWidget(scroll, 1);

    connect(catalog, &AppCatalog::changed, this, &MoreAppsView::rebuild);
    rebuild();
    if (catalog->isRefreshing()) {
        m_refresh->setEnabled(false);
        setStatus(tr("Getting the latest list…"));
    } else {
        const int apps = static_cast<int>(catalog->allApps().size());
        setStatus(apps == 1 ? tr("1 app.") : tr("%1 apps.").arg(apps));
    }
}

void MoreAppsView::setIntro(const QString& text)
{
    m_intro->setText(text);
    m_intro->setVisible(!text.isEmpty());
}

void MoreAppsView::setStatus(const QString& status)
{
    if (status == m_status) {
        return;
    }
    m_status = status;
    Q_EMIT statusChanged(status);
}

void MoreAppsView::rebuild()
{
    while (QLayoutItem* item = m_gridLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    const QString needle = m_search->text().trimmed();
    int n = 0;
    const QList<AppInfo> apps = AppCatalog::instance()->allApps();
    for (const AppInfo& app : apps) {
        const bool match = needle.isEmpty() || app.name.contains(needle, Qt::CaseInsensitive)
                           || app.summary.contains(needle, Qt::CaseInsensitive);
        if (!match) {
            continue;
        }
        m_gridLayout->addWidget(new AppCard(app, m_place, m_grid), n / m_columns, n % m_columns);
        ++n;
    }
    if (n == 0) {
        auto* none = new QLabel(tr("No app matches."), m_grid);
        none->setAlignment(Qt::AlignCenter);
        m_gridLayout->addWidget(none, 0, 0, 1, m_columns);
    }
    m_gridLayout->setRowStretch(m_gridLayout->rowCount(), 1);
}

} // namespace ktechpit
