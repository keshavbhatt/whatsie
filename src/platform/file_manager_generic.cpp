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
