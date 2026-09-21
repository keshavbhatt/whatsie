#include "platform/desktop_theme.h"

// No desktop colour-scheme portal off Linux: callers fall back to QStyleHints.
namespace whatsie::platform {

std::optional<Qt::ColorScheme> readDesktopColorScheme()
{
    return std::nullopt;
}

DesktopThemeWatcher::DesktopThemeWatcher(QObject* parent)
    : QObject(parent)
{}

void DesktopThemeWatcher::onSettingChanged(const QString&, const QString&) {}

} // namespace whatsie::platform
