#pragma once

#include <QString>

#include <functional>

namespace whatsie::core {

/// "photo.jpg" → "photo (1).jpg" … until `exists(fullPath)` is false. Never
/// overwrites (W#110). `exists` is injectable for tests.
[[nodiscard]] QString uniqueFileName(const QString& directory, const QString& fileName,
                                     const std::function<bool(const QString&)>& exists);
[[nodiscard]] QString uniqueFileName(const QString& directory, const QString& fileName);

/// Strips path separators and control characters from a name suggested by the
/// page; falls back to "download".
[[nodiscard]] QString sanitizeFileName(const QString& suggested);

/// "1.2 MB", "340 KB", "12 B". Binary units, one decimal above KB.
[[nodiscard]] QString humanSize(qint64 bytes);

} // namespace whatsie::core
