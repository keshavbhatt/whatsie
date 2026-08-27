#pragma once

#include <QLatin1StringView>

// Every persisted key lives here exactly once. Format: "section/camelCase".
// Adding a key = adding a typed accessor pair on core::Settings + a test.
namespace whatsie::core::keys {

// window/
inline constexpr QLatin1StringView kWindowGeometry{"window/geometry"};
inline constexpr QLatin1StringView kWindowState{"window/state"};
inline constexpr QLatin1StringView kCloseAction{"window/closeAction"};
inline constexpr QLatin1StringView kStartMinimized{"window/startMinimized"};

// tray/
inline constexpr QLatin1StringView kTrayLeftClickToggles{"tray/leftClickToggles"};
inline constexpr QLatin1StringView kTraySymbolicIcon{"tray/symbolicIcon"};
inline constexpr QLatin1StringView kTrayHidden{"tray/hidden"};
inline constexpr QLatin1StringView kTrayDimWhenDisconnected{"tray/dimWhenDisconnected"};

// view/
inline constexpr QLatin1StringView kZoomFactor{"view/zoomFactor"};
inline constexpr QLatin1StringView kZoomFactorMaximized{"view/zoomFactorMaximized"};
inline constexpr QLatin1StringView kSmoothScrolling{"view/smoothScrolling"};
inline constexpr QLatin1StringView kInterfaceScale{"view/interfaceScale"};
inline constexpr QLatin1StringView kMuted{"view/muted"};

// general/
inline constexpr QLatin1StringView kAutostart{"general/autostart"};

// privacy/
inline constexpr QLatin1StringView kMessageBlurLevel{"privacy/messageBlurLevel"};

// appearance/
inline constexpr QLatin1StringView kTheme{"appearance/theme"};

// notifications/
inline constexpr QLatin1StringView kNotificationsEnabled{"notifications/enabled"};
inline constexpr QLatin1StringView kNotificationSound{"notifications/sound"};
inline constexpr QLatin1StringView kNotificationTimeoutSec{"notifications/timeoutSec"};

// downloads/
inline constexpr QLatin1StringView kDownloadDirectory{"downloads/directory"};
inline constexpr QLatin1StringView kAskWhereToSave{"downloads/askWhereToSave"};
inline constexpr QLatin1StringView kShowDownloadsOnStart{"downloads/showWindowOnStart"};

// files/
inline constexpr QLatin1StringView kLastOpenDirectory{"files/lastOpenDirectory"};

// advanced/
inline constexpr QLatin1StringView kHardwareAcceleration{"advanced/hardwareAcceleration"};

} // namespace whatsie::core::keys
