// name:     sw-recovery
// purpose:  recover from a corrupt service-worker registration once per session
//           by unregistering it and reloading (FEATURES S16, W#43). Never clears
//           storage, so the WhatsApp session is preserved.
// depends:  navigator.serviceWorker (standard API)
// verified: 2026-08-27 against WhatsApp Web 2.3000.x
// on-fail:  no-op
(function () {
    'use strict';
    var api = window.__whatsie;
    try {
        if (!('serviceWorker' in navigator) || navigator.serviceWorker.__whatsiePatched) {
            return;
        }
        var FLAG = '__whatsie_sw_recovered';
        var register = navigator.serviceWorker.register.bind(navigator.serviceWorker);
        navigator.serviceWorker.register = function () {
            return register.apply(null, arguments).catch(function (err) {
                try {
                    if (!sessionStorage.getItem(FLAG)) {
                        sessionStorage.setItem(FLAG, '1');
                        api && api.log && api.log('service worker registration failed, recovering: ' + err);
                        navigator.serviceWorker.getRegistrations()
                            .then(function (rs) { return Promise.all(rs.map(function (r) { return r.unregister(); })); })
                            .then(function () { location.reload(); });
                    }
                } catch (e) { /* ignore */ }
                throw err;
            });
        };
        navigator.serviceWorker.__whatsiePatched = true;
    } catch (e) {
        api && api.report('sw-recovery', e);
    }
})();
