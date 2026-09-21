#include "platform/desktop_theme.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>
#include <QVariant>

using namespace Qt::StringLiterals;

namespace whatsie::platform {

namespace {

const auto kPortalService = u"org.freedesktop.portal.Desktop"_s;
const auto kPortalPath = u"/org/freedesktop/portal/desktop"_s;
const auto kSettingsIface = u"org.freedesktop.portal.Settings"_s;

} // namespace

std::optional<Qt::ColorScheme> readDesktopColorScheme()
{
    QDBusInterface portal(kPortalService, kPortalPath, kSettingsIface, QDBusConnection::sessionBus());
    if (!portal.isValid()) {
        return std::nullopt;
    }
    // portal Settings.Read returns a variant that GNOME nests inside another
    // variant, so unwrap QDBusVariant until a plain value is left — otherwise
    // every read looks empty and silently falls back to Qt (which does not track
    // the setting on GNOME at all).
    const auto read = [&portal](const QString& nameSpace) -> QVariant {
        QDBusReply<QVariant> reply = portal.call(u"Read"_s, nameSpace, u"color-scheme"_s);
        if (!reply.isValid()) {
            return {};
        }
        QVariant value = reply.value();
        while (value.canConvert<QDBusVariant>()) {
            value = value.value<QDBusVariant>().variant();
        }
        return value;
    };

    // GNOME's own key first, as a string: the standard appearance namespace is
    // stuck reporting "light" on some GNOME setups while this one tracks it.
    const QString gnome = read(u"org.gnome.desktop.interface"_s).toString();
    if (gnome == u"prefer-dark"_s) {
        return Qt::ColorScheme::Dark;
    }
    if (gnome == u"prefer-light"_s) {
        return Qt::ColorScheme::Light;
    }
    // The cross-desktop standard (KDE and a healthy GNOME): 1 = dark, 2 = light.
    const QVariant fdo = read(u"org.freedesktop.appearance"_s);
    if (fdo.isValid()) {
        switch (fdo.toUInt()) {
        case 1:
            return Qt::ColorScheme::Dark;
        case 2:
            return Qt::ColorScheme::Light;
        default:
            break;
        }
    }
    return std::nullopt;
}

DesktopThemeWatcher::DesktopThemeWatcher(QObject* parent)
    : QObject(parent)
{
    // The portal fires SettingChanged on GNOME where Qt's colorSchemeChanged does
    // not; the trailing variant argument is truncated to match this two-argument
    // slot.
    QDBusConnection::sessionBus().connect(kPortalService, kPortalPath, kSettingsIface,
                                          u"SettingChanged"_s, this,
                                          SLOT(onSettingChanged(QString, QString)));
}

void DesktopThemeWatcher::onSettingChanged(const QString& nameSpace, const QString& key)
{
    if (key != u"color-scheme"_s) {
        return;
    }
    if (nameSpace == u"org.freedesktop.appearance"_s || nameSpace == u"org.gnome.desktop.interface"_s) {
        Q_EMIT colorSchemeChanged();
    }
}

} // namespace whatsie::platform
