#include "platform/file_manager.h"
#include "platform/logging.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

using namespace Qt::StringLiterals;

namespace whatsie::platform {

namespace {

// xdg-desktop-portal OpenURI: opens a URI through the portal service over
// D-Bus. No child process is spawned, so it works under confinement and in
// dev builds where a spawned xdg-open would inherit a broken library path.
bool portalOpenUri(const QString& uri)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected() || bus.interface() == nullptr ||
        !bus.interface()->isServiceRegistered(u"org.freedesktop.portal.Desktop"_s)) {
        return false;
    }
    QDBusMessage call = QDBusMessage::createMethodCall(u"org.freedesktop.portal.Desktop"_s,
                                                       u"/org/freedesktop/portal/desktop"_s,
                                                       u"org.freedesktop.portal.OpenURI"_s, u"OpenURI"_s);
    call << QString() << uri << QVariantMap{};
    const QDBusMessage reply = bus.call(call, QDBus::Block, 3000);
    return reply.type() == QDBusMessage::ReplyMessage;
}

} // namespace

bool openDirectory(const QString& directory)
{
    const QString uri = QUrl::fromLocalFile(directory).toString();
    if (portalOpenUri(uri)) {
        return true;
    }
    qCDebug(lcPlatform) << "portal OpenURI unavailable; falling back to QDesktopServices";
    return QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
}

bool revealInFileManager(const QString& filePath)
{
    // https://www.freedesktop.org/wiki/Specifications/file-manager-interface/
    QDBusMessage call =
        QDBusMessage::createMethodCall(u"org.freedesktop.FileManager1"_s, u"/org/freedesktop/FileManager1"_s,
                                       u"org.freedesktop.FileManager1"_s, u"ShowItems"_s);
    call << QStringList{QUrl::fromLocalFile(filePath).toString()} << QString();
    const QDBusReply<void> reply = QDBusConnection::sessionBus().call(call, QDBus::Block, 3000);
    if (reply.isValid()) {
        return true;
    }
    qCDebug(lcPlatform) << "FileManager1.ShowItems unavailable:" << reply.error().message()
                        << "- opening folder";
    return openDirectory(QFileInfo(filePath).absolutePath());
}

} // namespace whatsie::platform
