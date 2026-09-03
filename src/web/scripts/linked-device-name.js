// name:     linked-device-name
// purpose:  label this session as "Whatsie for <OS>" in the phone's Linked
//           Devices list instead of the generic "Google Chrome (Linux)". The
//           brand string comes from window.__whatsie.config.linkedDeviceName
//           and is empty when the setting is off.
// depends:  WhatsApp Web's WAWebBrowserInfo module (WAWebMiscBrowserUtils.info
//           on older builds); wraps the browser-info object it returns
// verified: 2026-09-03 against WhatsApp Web 2.3000.x (cf. whatsapp-web.js #3325)
// on-fail:  no-op; WhatsApp's stock browser label is used
//
// WhatsApp derives the linked-device label client-side and stores it on the
// phone at link time. The phone renders it as "Browser (OS)" but validates the
// browser NAME against known browsers (dropping unknown ones) while showing
// arbitrary OS text — so we put the brand in `os` and an unrecognised `name`,
// which collapses the label to just "Whatsie for <OS>".
(function () {
    'use strict';
    var api = window.__whatsie || {};
    var hooked = false;

    function wrap(orig) {
        return function () {
            var info = orig.apply(this, arguments);
            try {
                var name = window.__whatsieLinkedDeviceName;
                if (name && info) {
                    info.os = name;
                    info.name = 'Whatsie'; // unrecognised → the phone omits the browser prefix
                }
            } catch (e) { /* keep the stock label */ }
            return info;
        };
    }

    function hookOnce() {
        if (hooked) { return true; }
        try {
            if (typeof window.require !== 'function') { return false; }
            // Current builds: a bare-function module patched via the registry.
            var rec = window.require('__debug').modulesMap['WAWebBrowserInfo'];
            if (rec && typeof rec.defaultExport === 'function') {
                var wrapped = wrap(rec.defaultExport);
                rec.defaultExport = wrapped;
                if (rec.exports && typeof rec.exports.default === 'function') {
                    rec.exports.default = wrapped;
                }
                hooked = true;
                return true;
            }
            // Older builds: a plain export on WAWebMiscBrowserUtils.
            var mod = window.require('WAWebMiscBrowserUtils');
            if (mod && typeof mod.info === 'function') {
                mod.info = wrap(mod.info);
                hooked = true;
                return true;
            }
        } catch (e) { /* module not registered yet — retry */ }
        return false;
    }

    // The wrapper reads the live value; the module system loads asynchronously,
    // so retry until it resolves (~30s while the app boots).
    function apply(name) {
        window.__whatsieLinkedDeviceName = name || '';
        if (!name || hooked) { return; }
        var tries = 0;
        (function retry() {
            if (hookOnce()) { return; }
            if (++tries < 120) { setTimeout(retry, 250); }
        })();
    }

    try {
        apply((api.config && api.config.linkedDeviceName) || '');
    } catch (e) {
        if (api.report) { api.report('linked-device-name', e); }
    }
})();
