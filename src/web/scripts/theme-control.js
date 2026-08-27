// name:     theme-control
// purpose:  make WhatsApp follow the app's theme setting (FEATURES A1). System
//           mode passes the OS scheme straight through; an explicit Light/Dark
//           forces it at the page level, because QStyleHints::setColorScheme is
//           overridden by the portal/KDE platform theme and never reaches Blink
//           (verified 2026-08-27 via CDP: explicit override left the page dark).
// depends:  window.__whatsie.config.colorScheme; WhatsApp's `dark` body class,
//           which its CSS is keyed on (removing it themes the page instantly).
// verified: 2026-08-27 against WhatsApp Web 2.3000.x
// on-fail:  no-op; falls back to the OS prefers-color-scheme
(function () {
    'use strict';
    var api = window.__whatsie || {};
    var realMatchMedia = window.matchMedia ? window.matchMedia.bind(window) : null;
    if (!realMatchMedia) { return; }

    var forced = null; // null (=system) | 'dark' | 'light'
    var wrappers = [];

    function isSchemeQuery(q) {
        return typeof q === 'string' && q.indexOf('prefers-color-scheme') >= 0;
    }
    function osDark() {
        try { return realMatchMedia('(prefers-color-scheme: dark)').matches; }
        catch (e) { return false; }
    }
    function schemeIsDark() {
        return forced ? forced === 'dark' : osDark();
    }

    // Replace matchMedia so WhatsApp's own theme logic reads our forced value,
    // while non-scheme queries pass straight through.
    window.matchMedia = function (query) {
        var real = realMatchMedia(query);
        if (!isSchemeQuery(query)) { return real; }
        var wantsDark = query.indexOf('dark') >= 0;
        var listeners = [];
        var onchange = null;
        function matches() { return wantsDark ? schemeIsDark() : !schemeIsDark(); }
        function fire() {
            var ev = { matches: matches(), media: query };
            listeners.slice().forEach(function (cb) {
                try { typeof cb === 'function' ? cb(ev) : (cb.handleEvent && cb.handleEvent(ev)); }
                catch (e) { /* ignore listener errors */ }
            });
            if (onchange) { try { onchange(ev); } catch (e) { /* ignore */ } }
        }
        try {
            // Forward real OS changes only while not forced.
            real.addEventListener('change', function () { if (!forced) { fire(); } });
        } catch (e) { /* older engines: ignore */ }
        wrappers.push(fire);
        return {
            media: query,
            get matches() { return matches(); },
            addEventListener: function (t, cb) { if (t === 'change' && cb) { listeners.push(cb); } },
            removeEventListener: function (t, cb) {
                listeners = listeners.filter(function (f) { return f !== cb; });
            },
            addListener: function (cb) { if (cb) { listeners.push(cb); } },
            removeListener: function (cb) {
                listeners = listeners.filter(function (f) { return f !== cb; });
            },
            dispatchEvent: function () { return true; },
            get onchange() { return onchange; },
            set onchange(cb) { onchange = cb; }
        };
    };

    function applyToDom() {
        try {
            if (document.body) {
                document.body.classList.toggle('dark', schemeIsDark());
            }
            if (forced) {
                localStorage.setItem('theme', JSON.stringify(forced));
            }
        } catch (e) {
            api.report && api.report('theme-control', e);
        }
    }
    function apply() {
        applyToDom();
        wrappers.slice().forEach(function (fire) { fire(); });
    }

    // Live updates from C++ (settings change). 'system' clears the override.
    window.__whatsieSetTheme = function (mode) {
        forced = (mode === 'dark' || mode === 'light') ? mode : null;
        apply();
    };

    var initial = api.config && api.config.colorScheme;
    forced = (initial === 'dark' || initial === 'light') ? initial : null;

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', applyToDom);
    } else {
        applyToDom();
    }
})();
