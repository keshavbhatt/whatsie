// name:     bootstrap
// purpose:  create window.__whatsie (config + error reporting) and connect the
//           QWebChannel bridge; every other script relies on this object.
// depends:  qt.webChannelTransport (QWebChannel), nothing from WhatsApp Web
// verified: 2026-08-27 against WhatsApp Web 2.3000.x
// on-fail:  no-op; reports are queued until the bridge is ready
//
// The C++ side prepends `window.__whatsieConfig = {...};` and qwebchannel.js
// to this file, so `QWebChannel` and the config exist when it runs.
(function () {
    'use strict';
    if (window.__whatsie) {
        return;
    }
    var pending = [];
    var api = {
        config: window.__whatsieConfig || {},
        bridge: null,
        report: function (name, error) {
            var message = error && error.message ? error.message : String(error);
            if (api.bridge) {
                api.bridge.scriptFailed(name, message);
            } else {
                pending.push([name, message]);
            }
        },
        log: function (message) {
            if (api.bridge) {
                api.bridge.log(String(message));
            }
        }
    };
    window.__whatsie = api;
    try {
        if (typeof QWebChannel === 'function' && window.qt && window.qt.webChannelTransport) {
            new QWebChannel(window.qt.webChannelTransport, function (channel) {
                api.bridge = channel.objects.bridge || null;
                if (api.bridge) {
                    pending.forEach(function (p) { api.bridge.scriptFailed(p[0], p[1]); });
                    pending = [];
                }
            });
        }
    } catch (e) {
        pending.push(['bootstrap', e && e.message ? e.message : String(e)]);
    }
})();
