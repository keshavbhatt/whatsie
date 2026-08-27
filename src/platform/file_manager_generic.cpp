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

} // namespace whatsie::platform
