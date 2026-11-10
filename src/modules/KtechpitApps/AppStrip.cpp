// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
#include "AppStrip.h"

#include "AppCard.h"
#include "AppCatalog.h"
#include "MoreAppsDialog.h"

#include <QBoxLayout>
#include <QLabel>

using namespace Qt::StringLiterals;

namespace ktechpit {

AppStrip::AppStrip(const QString& place, int count, Orientation orientation, QWidget* parent)
    : QWidget(parent)
    , m_place(place)
    , m_count(count)
{
    setObjectName(u"ktechpitAppStrip"_s);

    m_title = new QLabel(tr("More apps from Ktechpit"), this);
    m_title->setObjectName(u"ktechpitAppStripTitle"_s); // for the host's style sheet
    QFont bold = m_title->font();
    bold.setBold(true);
    m_title->setFont(bold);

    auto* seeAll = new QLabel(u"<a href=\"all\" style=\"text-decoration:none\">%1</a>"_s.arg(tr("See all")), this);
    seeAll->setObjectName(u"ktechpitAppStripSeeAll"_s);
    seeAll->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    connect(seeAll, &QLabel::linkActivated, this, [this] {
        if (const AppCatalog::ShowAll& showAll = AppCatalog::instance()->showAll()) {
            showAll(m_place, window());
            return;
        }
        MoreAppsDialog dialog(m_place, window());
        dialog.exec();
    });

    auto* head = new QHBoxLayout;
    head->setContentsMargins(0, 0, 0, 0);
    head->addWidget(m_title);
    head->addStretch();
    head->addWidget(seeAll);

    m_cards = new QWidget(this);
    m_cardsLayout = orientation == Orientation::Horizontal
                        ? static_cast<QBoxLayout*>(new QHBoxLayout(m_cards))
                        : static_cast<QBoxLayout*>(new QVBoxLayout(m_cards));
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(10);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addLayout(head);
    layout->addWidget(m_cards);

    AppCatalog* catalog = AppCatalog::instance();
    connect(catalog, &AppCatalog::changed, this, [this] { rebuild(false); });
    connect(catalog, &AppCatalog::enabledChanged, this, [this] { rebuild(true); });
    rebuild(true);
}

void AppStrip::setTitle(const QString& title)
{
    m_title->setText(title);
    m_title->setVisible(!title.isEmpty());
}

void AppStrip::rebuild(bool repick)
{
    while (QLayoutItem* item = m_cardsLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    AppCatalog* catalog = AppCatalog::instance();
    QList<AppInfo> apps;
    if (catalog->isEnabled() && !repick && !m_shown.isEmpty()) {
        const QList<AppInfo> all = catalog->apps();
        for (const QString& id : std::as_const(m_shown)) {
            for (const AppInfo& app : all) {
                if (app.id == id) {
                    apps << app;
                }
            }
        }
    }
    if (catalog->isEnabled() && apps.isEmpty()) {
        apps = catalog->randomApps(m_count);
    }
    m_shown.clear();
    for (const AppInfo& app : std::as_const(apps)) {
        m_shown << app.id;
    }
    for (const AppInfo& app : apps) {
        // Equal stretch: side by side, every card gets the same width, whatever
        // the length of its text.
        m_cardsLayout->addWidget(new AppCard(app, m_place, m_cards), 1);
    }
    setVisible(!apps.isEmpty());
}

} // namespace ktechpit
