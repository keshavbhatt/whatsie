#ifndef WEBENGINENOTIFPROXY_H
#define WEBENGINENOTIFPROXY_H

#include <memory>

#include <QWebEngineNotification>

class WebEngineNotifProxy;

using WebEngineNotifPtr      = std::unique_ptr<QWebEngineNotification>;
using WebEngineNotifProxyPtr = std::shared_ptr<WebEngineNotifProxy>;

/**
 * - Wrap the unique_ptr notification to prevent accidental use-after-free.
 * - Capture the proxy in lambdas with shared_ptr to extend the notification’s lifetime.
 * - Use invoke() for all calls to ensure execution in the owning thread.
 * - Keep the unique_ptr const to make ownership explicit, but allow calls on the pointee.
 * - Expose only what is necessary to interact asynchronously; keep call sites clean.
 */
class WebEngineNotifProxy: public std::enable_shared_from_this<WebEngineNotifProxy> {
  private:
    explicit WebEngineNotifProxy(WebEngineNotifPtr n);

  public:
    // The proxy manages the notification's lifetime;
    // public const pointer allows read-only access to unique_ptr in queued lambdas.
    const WebEngineNotifPtr notif;

    WebEngineNotifProxy(const WebEngineNotifProxy&) = delete;
    WebEngineNotifProxy& operator=(const WebEngineNotifProxy&) = delete;


    void invoke(void (QWebEngineNotification::*fn)() const);

    static WebEngineNotifProxyPtr create(WebEngineNotifPtr n);
};

#endif /* WEBENGINENOTIFPROXY_H */
