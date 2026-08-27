#pragma once

#include <QNetworkProxy>

namespace whatsie::core {
struct ProxyConfig;
}

namespace whatsie::web {

/// Maps a ProxyConfig to a QNetworkProxy (FEATURES P3). System mode is not
/// representable here — the caller uses QNetworkProxyFactory instead — so a
/// System config maps to DefaultProxy. Pure; unit-tested.
[[nodiscard]] QNetworkProxy toNetworkProxy(const core::ProxyConfig& config);

/// Applies the configuration process-wide: System → the desktop's proxy
/// settings, otherwise the explicit application proxy. QtWebEngine honours
/// QNetworkProxy::applicationProxy() for its network stack.
void applyProxy(const core::ProxyConfig& config);

} // namespace whatsie::web
