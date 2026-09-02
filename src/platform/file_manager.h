#pragma once

#include <QString>

namespace whatsie::platform {

/// Opens the system file manager with `filePath` selected when the platform
/// supports it (Linux: org.freedesktop.FileManager1), otherwise opens the
/// containing folder. Returns false when nothing could be launched.
bool revealInFileManager(const QString& filePath);

/// Opens a directory in the file manager.
bool openDirectory(const QString& directory);

/// Opens a file in its default application. Uses the desktop portal's fd-based
/// OpenFile where available (the route that works under snap/flatpak
/// confinement, where QDesktopServices::openUrl silently no-ops a local file),
/// falling back to QDesktopServices. Returns false when nothing could open it.
bool openFile(const QString& filePath);

} // namespace whatsie::platform
