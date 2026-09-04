// name:     linked-device-name
// purpose:  label this session as "Whatsie for <OS>" in the phone's Linked
//           Devices list instead of the generic "Google Chrome (Linux)". The
//           brand string comes from window.__whatsie.config.linkedDeviceName
//           and is empty when the setting is off.
// depends:  WhatsApp Web's WAWebBrowserInfo module (WAWebMiscBrowserUtils.info
//           on older builds); wraps the browser-info object it returns
// verified: 2026-09-04 against WhatsApp Web 2.3000.x (cf. whatsapp-web.js #3325)
// on-fail:  no-op; WhatsApp's stock browser label is used. Only the module
//           registry is read — a WAWeb module is never require()d, so this can
//           never force-load a half-ready module and block login.
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

    // A module's evaluated exports, but only if WhatsApp has already run its
    // factory. Reading these off the registry record never triggers evaluation —
    // unlike require(), which force-resolves the whole dependency subtree.
    function loadedExports(rec) {
        if (!rec) { return null; }
        return rec.exports || (rec.publicModule && rec.publicModule.exports) || null;
    }

    function hookOnce() {
        if (hooked) { return true; }
        try {
            if (typeof window.require !== 'function') { return false; }
            var map = window.require('__debug').modulesMap;
            if (!map) { return false; }
            // Current builds: WAWebBrowserInfo's default export is the info function.
            var rec = map['WAWebBrowserInfo'];
            if (rec && typeof rec.defaultExport === 'function') {
                var wrapped = wrap(rec.defaultExport);
                rec.defaultExport = wrapped;
                if (rec.exports && typeof rec.exports.default === 'function') {
                    rec.exports.default = wrapped;
                }
                hooked = true;
                return true;
            }
            // Older builds: a plain export on WAWebMiscBrowserUtils. Read it from
            // the registry ONLY if WhatsApp has already loaded it — never require()
            // a WAWeb module here. Forcing WAWebMiscBrowserUtils to resolve before
            // its dependency subtree (UserPrefs, storage, lazy chunks) is ready
            // corrupts WhatsApp's module system and blocks login — both the
            // phone-number "alt pairing" flow and the QR flow.
            var mod = loadedExports(map['WAWebMiscBrowserUtils']);
            if (mod && typeof mod.info === 'function') {
                mod.info = wrap(mod.info);
                hooked = true;
                return true;
            }
        } catch (e) { /* module system not ready yet — retry */ }
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
