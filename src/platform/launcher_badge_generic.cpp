#include "platform/launcher_badge.h"

namespace whatsie::platform {

// No launcher-badge protocol on this platform (a Windows taskbar overlay could
// slot in here later — FEATURES T8).
void setLauncherCount(const QString& /*desktopId*/, int /*count*/) {}

} // namespace whatsie::platform
