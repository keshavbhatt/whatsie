#pragma once

#include <QString>

namespace whatsie::core {
class Settings;
}

namespace whatsie::ui {

/// Markdown block for bug reports: versions, host, paths, recent log lines
/// (FEATURES P10, D1). Never includes message content or the session. The last
/// crash (if any) is appended only when includeCrash is set.
[[nodiscard]] QString buildDiagnostics(const core::Settings& settings, const QString& userAgent,
                                       int logLines = 60, bool includeCrash = true);

/// The full report a user submits: their own description followed by the
/// diagnostics block. This is what goes on the clipboard to paste into an issue.
[[nodiscard]] QString bugReportBody(const core::Settings& settings, const QString& userAgent,
                                    const QString& summary, bool includeCrash);

/// A GitHub "new issue" URL for the whatsie repo, pre-filled with the user's
/// summary and a short environment block only — logs never go in the URL (they
/// exceed GitHub's length limit), so the URL always stays valid.
[[nodiscard]] QString bugReportUrl(const QString& userAgent, const QString& summary);

} // namespace whatsie::ui
