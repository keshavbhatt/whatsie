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

/// A GitHub "new issue" URL for the whatsie repo, pre-filled with the user's
/// title, description and a short environment block only. Logs never go in the
/// URL (they exceed GitHub's length limit and get the request rejected) — they
/// are copied to the clipboard separately for the reporter to paste in, so the
/// prefilled body must NOT repeat them. Title is length-limited.
[[nodiscard]] QString bugReportUrl(const QString& userAgent, const QString& title,
                                   const QString& description);

} // namespace whatsie::ui
