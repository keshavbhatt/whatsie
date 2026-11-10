// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
#include "AppLinks.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QUrlQuery>

using namespace Qt::StringLiterals;

namespace ktechpit::AppLinks {

QUrl pageUrl(const AppInfo& app, const QString& source, const QString& place)
{
    QUrl url = app.page;
    if (!url.isValid()) {
        url = QUrl(u"https://ktechpit.com/USS/public/product.php"_s);
        QUrlQuery q;
        q.addQueryItem(u"slug"_s, app.id);
        url.setQuery(q);
    }
    if (!source.isEmpty()) {
        QUrlQuery q(url);
        q.removeAllQueryItems(u"ref"_s);
        q.addQueryItem(u"ref"_s, place.isEmpty() ? source : source + u"-"_s + place);
        url.setQuery(q);
    }
    return url;
}

QUrl getUrl(const AppInfo& app, Package package, const QString& source, const QString& place)
{
    switch (package) {
    case Package::Snap:
        if (!app.snap.isEmpty()) {
            return QUrl(u"snap://"_s + app.snap);
        }
        break;
    case Package::Flatpak:
    case Package::Native:
        // A distro package user most likely has a software centre that speaks AppStream.
        if (!app.flatpak.isEmpty()) {
            return QUrl(u"appstream://"_s + app.flatpak);
        }
        if (package == Package::Native && !app.snap.isEmpty()) {
            return QUrl(u"snap://"_s + app.snap);
        }
        break;
    case Package::AppImage:
    case Package::Windows:
        break;
    }
    return pageUrl(app, source, place);
}

void openGet(const AppInfo& app, const QString& place)
{
    const CatalogConfig& c = AppCatalog::instance()->config();
    const QUrl url = getUrl(app, c.package, c.source, place);
    if (!QDesktopServices::openUrl(url) && url.scheme() != u"https"_s) {
        openPage(app, place);
    }
}

void openPage(const AppInfo& app, const QString& place)
{
    const CatalogConfig& c = AppCatalog::instance()->config();
    QDesktopServices::openUrl(pageUrl(app, c.source, place));
}

bool isInstalled(const AppInfo& app)
{
#if defined(Q_OS_LINUX)
    if (!app.snap.isEmpty() && QFileInfo::exists(u"/snap/"_s + app.snap + u"/current"_s)) {
        return true;
    }
    if (!app.flatpak.isEmpty()) {
        const QStringList roots{
            u"/var/lib/flatpak/app/"_s,
            // In a snap, $HOME is the snap's own folder; the real one is in SNAP_REAL_HOME.
            qEnvironmentVariable("SNAP_REAL_HOME", QDir::homePath()) + u"/.local/share/flatpak/app/"_s,
        };
        for (const QString& root : roots) {
            if (QFileInfo::exists(root + app.flatpak + u"/current"_s)) {
                return true;
            }
        }
    }
#else
    Q_UNUSED(app)
#endif
    return false;
}

} // namespace ktechpit::AppLinks
