#pragma once

#include <QString>
#include <QStringList>

// Guards around destructive storage operations (FEATURES P5). W#230: a
// cache-clear once deleted a user's home directory. Nothing is removed
// unless it passes isSafeToDelete().
namespace whatsie::core {

/// True only when `path` is a real directory strictly *inside* one of
/// `allowedRoots` (after resolving symlinks) and is not a user-facing
/// location (home, Downloads, ...).
[[nodiscard]] bool isSafeToDelete(const QString& path, const QStringList& allowedRoots);

/// Recursive size; symlinks are not followed.
[[nodiscard]] qint64 directorySize(const QString& path);

/// Removes the directory if isSafeToDelete() agrees. Returns false otherwise.
bool removeDirectorySafely(const QString& path, const QStringList& allowedRoots);

} // namespace whatsie::core
