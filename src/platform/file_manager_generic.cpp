#include "platform/file_manager.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

namespace whatsie::platform {

bool openDirectory(const QString& directory)
{
    return QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
}

bool revealInFileManager(const QString& filePath)
{
    return openDirectory(QFileInfo(filePath).absolutePath());
}

bool openFile(const QString& filePath)
{
    return QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

bool openUrl(const QString& url)
{
    return QDesktopServices::openUrl(QUrl(url));
}

} // namespace whatsie::platform

namespace whatsie::platform {
// Autostart backends per OS land with their platform milestones; no-op fallback.
bool autostartEnabled() { return false; }
bool setAutostartEnabled(bool, bool) { return false; }
} // namespace whatsie::platform
