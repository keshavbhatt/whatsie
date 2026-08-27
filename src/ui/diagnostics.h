#pragma once

#include <QString>

namespace whatsie::core {
class Settings;
}

namespace whatsie::ui {

/// Markdown block for bug reports: versions, host, paths, recent log lines
/// (FEATURES P10, D1). Never includes message content or the session.
[[nodiscard]] QString buildDiagnostics(const core::Settings& settings, const QString& userAgent,
                                       int logLines = 60);

} // namespace whatsie::ui
