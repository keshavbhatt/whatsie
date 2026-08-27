#pragma once

#include <QString>

namespace whatsie::platform {

/// Opens the system file manager with `filePath` selected when the platform
/// supports it (Linux: org.freedesktop.FileManager1), otherwise opens the
/// containing folder. Returns false when nothing could be launched.
bool revealInFileManager(const QString& filePath);

/// Opens a directory in the file manager.
bool openDirectory(const QString& directory);

} // namespace whatsie::platform
