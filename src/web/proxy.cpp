#include "web/proxy.h"

#include "core/settings/settings.h"
#include "web/logging.h"

#include <QNetworkProxyFactory>

namespace whatsie::web {

QNetworkProxy toNetworkProxy(const core::ProxyConfig& config)
{
    switch (config.mode) {
    case core::ProxyMode::None:
        return QNetworkProxy(QNetworkProxy::NoProxy);
    case core::ProxyMode::Manual: {
        const auto type =
            config.type == core::ProxyType::Socks5 ? QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy;
        return QNetworkProxy(type, config.host, static_cast<quint16>(config.port), config.user,
                             config.password);
    }
    case core::ProxyMode::System:
        break;
    }
    return QNetworkProxy(QNetworkProxy::DefaultProxy);
}

void applyProxy(const core::ProxyConfig& config)
{
    if (config.mode == core::ProxyMode::System) {
        QNetworkProxyFactory::setUseSystemConfiguration(true);
        qCInfo(lcWeb) << "proxy: following system configuration";
        return;
    }
    QNetworkProxyFactory::setUseSystemConfiguration(false);
    const QNetworkProxy proxy = toNetworkProxy(config);
    QNetworkProxy::setApplicationProxy(proxy);
    qCInfo(lcWeb) << "proxy:" << (config.mode == core::ProxyMode::None ? "direct (none)" : "manual")
                  << proxy.hostName() << proxy.port();
}

} // namespace whatsie::web
