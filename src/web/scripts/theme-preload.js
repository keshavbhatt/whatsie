// name:     theme-preload
// purpose:  write the effective colour scheme into WhatsApp Web's own
//           preference (localStorage "theme") BEFORE the app boots, so the
//           page starts in the same theme as the Qt chrome. No React/DOM
//           poking (FEATURES A1; A3 dropped: a theme change reloads the page).
// depends:  localStorage key "theme" holding a JSON string "light"|"dark"
//           (WhatsApp Web preference storage format)
// verified: 2026-08-27 against WhatsApp Web 2.3000.x
// on-fail:  no-op; WhatsApp falls back to its own setting
(function () {
    'use strict';
    try {
        var cfg = (window.__whatsie && window.__whatsie.config) || {};
        var theme = cfg.theme === 'dark' ? 'dark' : (cfg.theme === 'light' ? 'light' : null);
        if (!theme) {
            return;
        }
        var wanted = JSON.stringify(theme);
        if (localStorage.getItem('theme') !== wanted) {
            localStorage.setItem('theme', wanted);
        }
        // "system-theme-mode" makes WhatsApp ignore "theme"; we always decide.
        if (localStorage.getItem('system-theme-mode') !== null) {
            localStorage.removeItem('system-theme-mode');
        }
    } catch (e) {
        window.__whatsie && window.__whatsie.report('theme-preload', e);
    }
})();
