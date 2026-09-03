#include "platform/autostart.h"

namespace whatsie::platform {

// Platforms without a launch-at-login backend: report unsupported.
bool autostartEnabled()
{
    return false;
}

bool setAutostartEnabled(bool /*enabled*/, bool /*startMinimized*/)
{
    return false;
}

} // namespace whatsie::platform
