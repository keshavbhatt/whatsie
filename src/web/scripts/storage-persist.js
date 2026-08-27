// name:     storage-persist
// purpose:  make navigator.storage.persist()/persisted() resolve true so
//           WhatsApp Web does not take its "storage may be evicted" path
//           (spurious console errors, degraded caching). Our profile is
//           persistent anyway.
// depends:  navigator.storage (standard API)
// verified: 2026-08-27 against WhatsApp Web 2.3000.x
// on-fail:  no-op
(function () {
    'use strict';
    try {
        if (!navigator.storage) {
            return;
        }
        var granted = function () { return Promise.resolve(true); };
        Object.defineProperty(navigator.storage, 'persist', { value: granted, configurable: true });
        Object.defineProperty(navigator.storage, 'persisted', { value: granted, configurable: true });
    } catch (e) {
        window.__whatsie && window.__whatsie.report('storage-persist', e);
    }
})();
