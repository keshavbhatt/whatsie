#pragma once

#include <QLatin1StringView>

// Every persisted key lives here exactly once. Format: "section/camelCase".
// Adding a key = adding a typed accessor pair on core::Settings + a test.
namespace whatsie::core::keys {

// window/
inline constexpr QLatin1StringView kWindowGeometry{"window/geometry"};
inline constexpr QLatin1StringView kWindowState{"window/state"};
inline constexpr QLatin1StringView kSettingsDialogGeometry{"window/settingsDialogGeometry"};
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

// spellcheck/
inline constexpr QLatin1StringView kSpellCheckEnabled{"spellcheck/enabled"};
inline constexpr QLatin1StringView kSpellCheckLanguages{"spellcheck/languages"};

// lock/ (FEATURES P1) — the passcode salt/hash are stored; the passcode is not
inline constexpr QLatin1StringView kLockSalt{"lock/salt"};
inline constexpr QLatin1StringView kLockHash{"lock/hash"};
inline constexpr QLatin1StringView kLockIterations{"lock/iterations"};
inline constexpr QLatin1StringView kLockOnStart{"lock/onStart"};
inline constexpr QLatin1StringView kLockOnHide{"lock/onHide"};
inline constexpr QLatin1StringView kLockIdleMinutes{"lock/idleMinutes"};

// advanced/
inline constexpr QLatin1StringView kHardwareAcceleration{"advanced/hardwareAcceleration"};
inline constexpr QLatin1StringView kWebrtcPublicOnly{"advanced/webrtcPublicInterfacesOnly"};
// GPU auto-fallback state (ADR-032): set after repeated unstable GPU trials.
inline constexpr QLatin1StringView kGpuAutoDisabled{"advanced/gpuAutoDisabled"};
inline constexpr QLatin1StringView kGpuProbeStrikes{"advanced/gpuProbeStrikes"};
// Set when the GPU was auto-disabled, so the next launch can tell the user why.
inline constexpr QLatin1StringView kGpuFallbackNotice{"advanced/gpuFallbackNotice"};
// V8 JavaScript heap cap in MB; 0 = automatic. Bounds WhatsApp Web's memory.
inline constexpr QLatin1StringView kJsMemoryLimitMb{"advanced/jsMemoryLimitMb"};

// proxy/ (password is deliberately NOT a key — never persisted)
inline constexpr QLatin1StringView kProxyMode{"proxy/mode"};
inline constexpr QLatin1StringView kProxyType{"proxy/type"};
inline constexpr QLatin1StringView kProxyHost{"proxy/host"};
inline constexpr QLatin1StringView kProxyPort{"proxy/port"};
inline constexpr QLatin1StringView kProxyUser{"proxy/user"};

} // namespace whatsie::core::keys
