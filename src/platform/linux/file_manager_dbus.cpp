#include "platform/file_manager.h"
#include "platform/logging.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

#include <fcntl.h>
#include <unistd.h>

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

// xdg-desktop-portal OpenDirectory: opens a file manager at a directory passed
// as a real file descriptor. This is the call that works under snap/flatpak
// confinement (OpenURI silently no-ops a local file:// directory).
bool portalOpenDirectory(const QString& directory)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected() || bus.interface() == nullptr ||
        !bus.interface()->isServiceRegistered(u"org.freedesktop.portal.Desktop"_s)) {
        return false;
    }
    const int fd = ::open(QFile::encodeName(directory).constData(), O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (fd < 0) {
        return false;
    }
    QDBusUnixFileDescriptor descriptor(fd);
    ::close(fd); // QDBusUnixFileDescriptor keeps its own dup
    QDBusMessage call = QDBusMessage::createMethodCall(
        u"org.freedesktop.portal.Desktop"_s, u"/org/freedesktop/portal/desktop"_s,
        u"org.freedesktop.portal.OpenURI"_s, u"OpenDirectory"_s);
    call << QString() << QVariant::fromValue(descriptor) << QVariantMap{};
    const QDBusMessage reply = bus.call(call, QDBus::Block, 3000);
    return reply.type() == QDBusMessage::ReplyMessage;
}

} // namespace

bool openDirectory(const QString& directory)
{
    const QString uri = QUrl::fromLocalFile(directory).toString();

    // FileManager1.ShowFolders opens the folder itself (not selecting it in the
    // parent) and is D-Bus activated, so no child process inherits a dev-broken
    // library path. Preferred where it is reachable (most desktops).
    QDBusMessage show =
        QDBusMessage::createMethodCall(u"org.freedesktop.FileManager1"_s, u"/org/freedesktop/FileManager1"_s,
                                       u"org.freedesktop.FileManager1"_s, u"ShowFolders"_s);
    show << QStringList{uri} << QString();
    const QDBusReply<void> reply = QDBusConnection::sessionBus().call(show, QDBus::Block, 3000);
    if (reply.isValid()) {
        return true;
    }
    qCDebug(lcPlatform) << "FileManager1.ShowFolders unavailable:" << reply.error().message();

    // Under confinement FileManager1 may be blocked; the portal's fd-based
    // OpenDirectory is the sanctioned route (plain OpenURI no-ops a local dir).
    if (portalOpenDirectory(directory)) {
        return true;
    }
    if (portalOpenUri(uri)) {
        return true;
    }
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
