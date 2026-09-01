// name:     theme-control
// purpose:  make WhatsApp follow the app's theme setting (FEATURES A1). Drives
//           WhatsApp's own theme state — its WAWeb modules, the React store, and
//           the DOM/localStorage it persists to — rather than QStyleHints, which
//           the portal/KDE platform theme overrides before it reaches Blink
//           (verified 2026-08-27 via CDP). Sequence proven against live
//           WhatsApp Web.
// depends:  window.__whatsie.config.colorScheme; WhatsApp's require() modules,
//           .app-wrapper-web React fiber, and `dark` body class (all optional —
//           each step is guarded and degrades to a no-op).
// verified: 2026-08-27 against WhatsApp Web 2.3000.x
// on-fail:  no-op; falls back to WhatsApp's own OS-following behaviour
// live-api: window.__whatsieSetTheme('system'|'light'|'dark'); called from
//           WebView::applyThemeLive() on theme change and every page load.
(function () {
    'use strict';
    var api = window.__whatsie || {};

    function osTheme() {
        try {
            return window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
        } catch (e) {
            return 'light';
        }
    }

    function applyTheme(mode) {
        var system = mode === 'system';
        var theme = system ? osTheme() : (mode === 'dark' ? 'dark' : 'light');
        var isDark = theme === 'dark';

        // 1. WhatsApp's own preference + theme modules.
        try {
            if (typeof require === 'function') {
                try {
                    var up = require('WAWebUserPrefsGeneral');
                    if (up) {
                        if (typeof up.setSystemThemeMode === 'function') { up.setSystemThemeMode(system); }
                        if (typeof up.setTheme === 'function') { up.setTheme(theme); }
                    }
                } catch (e) { /* module absent */ }
                try {
                    var tc = require('WAWebThemeContext');
                    if (tc && typeof tc.applyThemeToUI === 'function') { tc.applyThemeToUI(theme); }
                } catch (e) { /* module absent */ }
                try {
                    var st = require('WAWebSystemTheme');
                    if (st) { st.theme = theme; }
                } catch (e) { /* module absent */ }
            }
        } catch (e) { /* require unavailable */ }

        // 2. React store: walk the fiber ancestors for the component holding
        //    { theme, systemThemeMode } and setState; else forceUpdate upward.
        try {
            var wrapper = document.querySelector('.app-wrapper-web');
            var key = wrapper && Object.keys(wrapper).find(function (k) {
                return k.indexOf('__reactFiber') === 0 || k.indexOf('__reactInternalInstance') === 0;
            });
            if (key) {
                var fiber = wrapper[key];
                var found = false;
                while (fiber) {
                    var sn = fiber.stateNode;
                    if (sn && sn.state && sn.state.theme !== undefined &&
                        sn.state.systemThemeMode !== undefined && typeof sn.setState === 'function') {
                        sn.setState({ theme: theme, systemThemeMode: system });
                        found = true;
                        break;
                    }
                    fiber = fiber.return;
                }
                if (!found) {
                    fiber = wrapper[key];
                    var count = 0;
                    while (fiber && count < 10) {
                        if (fiber.stateNode && typeof fiber.stateNode.forceUpdate === 'function') {
                            try { fiber.stateNode.forceUpdate(); } catch (e) { /* ignore */ }
                            count++;
                        }
                        fiber = fiber.return;
                    }
                }
            }
        } catch (e) { /* React internals changed */ }

        // 3. DOM attributes + localStorage — persists across reloads, covers any
        //    CSS-only observers and WhatsApp's own startup read.
        try {
            var root = document.documentElement;
            root.setAttribute('data-theme', theme);
            root.setAttribute('data-color-mode', theme);
            root.style.colorScheme = theme;
            if (document.body) { document.body.classList.toggle('dark', isDark); }
            localStorage.setItem('theme', theme);
            if (system) {
                localStorage.setItem('system-theme-mode', 'true');
            } else {
                localStorage.removeItem('system-theme-mode');
            }
            try {
                window.dispatchEvent(new StorageEvent('storage', {
                    key: 'theme', newValue: theme, storageArea: localStorage, url: location.href
                }));
            } catch (e) { /* StorageEvent ctor unsupported */ }
        } catch (e) {
            api.report && api.report('theme-control', e);
        }
    }

    window.__whatsieSetTheme = function (mode) { applyTheme(mode); };

    // First paint: apply the configured theme, retrying briefly until WhatsApp's
    // modules are up (C++ also re-applies on every loadFinished).
    var initial = (api.config && api.config.colorScheme) || 'system';
    var tries = 0;
    function seed() {
        applyTheme(initial);
        tries++;
        if (tries < 20 && typeof require !== 'function') {
            setTimeout(seed, 250);
        }
    }
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', seed);
    } else {
        seed();
    }
})();
