#pragma once

#include <QString>

namespace whatsie::platform {

/// Installs handlers for fatal signals that write a short backtrace to
/// `crashFilePath`, and reads any report a previous run left there into memory
/// (then removes the file) so it can be attached to a bug report.
void installCrashHandler(const QString& crashFilePath);

/// The previous run's crash report, or empty when the last exit was clean.
[[nodiscard]] QString lastCrashReport();

} // namespace whatsie::platform
