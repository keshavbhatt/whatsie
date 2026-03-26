#include "webenginenotifproxy.h"

WebEngineNotifProxy::WebEngineNotifProxy(WebEngineNotifPtr n)
  : notif(std::move(n)) {}


void WebEngineNotifProxy::invoke(void (QWebEngineNotification::*fn)() const) {
  QMetaObject::invokeMethod(notif.get(),
                            [self = shared_from_this(), fn]() {
                              (self->notif.get()->*fn)();
                            },
                            Qt::QueuedConnection);
}

WebEngineNotifProxyPtr WebEngineNotifProxy::create(WebEngineNotifPtr n) {
  // direct construction because ctor is private
  return WebEngineNotifProxyPtr(new WebEngineNotifProxy(std::move(n)));
}
