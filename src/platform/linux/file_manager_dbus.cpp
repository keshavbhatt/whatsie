#include "platform/file_manager.h"
#include "platform/logging.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

using namespace Qt::StringLiterals;

namespace whatsie::platform {

bool openDirectory(const QString& directory)
{
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
