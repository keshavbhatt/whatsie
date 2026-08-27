// name:     nav-settings
// purpose:  add a Settings entry to WhatsApp's left nav rail so the app is
//           reachable without a system tray (FEATURES A11). A Qt widget cannot
//           be drawn over the QWebEngineView (and a floating window cannot track
//           it on Wayland), so the button must live in the page. Approach — clone
//           a neighbouring rail entry and re-place it on a timer — is adapted
//           from whatly (webtweaks.cpp), which proved it against WhatsApp's rail
//           rebuilds. Click calls back through the QWebChannel bridge.
// depends:  window.__whatsie.bridge.openSettings; the rail's <button> entries and
//           the avatar (last rail button with an <img>). All obfuscated classes
//           are avoided — entries are found by shape, not class.
// verified: 2026-08-28 against WhatsApp Web 2.3000.x
// on-fail:  no-op; the tray and Ctrl+, still open Settings
(function () {
    'use strict';
    var ID = 'whatsie-settings-nav';
    var GEAR =
        '<path fill="currentColor" d="M12 8a4 4 0 1 0 0 8 4 4 0 0 0 0-8zm0 6a2 2 0 1 1 0-4 2 2 0 0 1 0 4z"/>' +
        '<path fill="currentColor" d="M19.4 13c.04-.32.06-.66.06-1s-.02-.68-.06-1l2.11-1.65a.5.5 0 0 0 .12-.64l-2-3.46a.5.5 0 0 0-.61-.22l-2.49 1a7.3 7.3 0 0 0-1.73-1l-.38-2.65A.5.5 0 0 0 14 1h-4a.5.5 0 0 0-.5.42l-.38 2.65a7.3 7.3 0 0 0-1.73 1l-2.49-1a.5.5 0 0 0-.61.22l-2 3.46a.5.5 0 0 0 .12.64L4.6 11c-.04.32-.06.66-.06 1s.02.68.06 1l-2.11 1.65a.5.5 0 0 0-.12.64l2 3.46c.14.24.42.32.61.22l2.49-1c.52.4 1.1.74 1.73 1l.38 2.65c.04.24.25.42.5.42h4c.25 0 .46-.18.5-.42l.38-2.65c.63-.26 1.21-.6 1.73-1l2.49 1c.24.1.47.02.61-.22l2-3.46a.5.5 0 0 0-.12-.64L19.4 13z"/>';

    function railButtons() {
        return Array.prototype.slice.call(document.querySelectorAll('button')).filter(function (b) {
            var r = b.getBoundingClientRect();
            return r.width > 0 && r.width <= 72 && r.left < 80;
        });
    }
    // Climb until `node` and `other` become siblings — the level rail entries
    // share, several wrappers above the <button> itself.
    function wrapperSharedWith(node, other) {
        while (node.parentElement) {
            if (node.parentElement.contains(other)) { return node; }
            node = node.parentElement;
        }
        return null;
    }

    var railParent = null;

    function placed() {
        var entry = document.getElementById(ID);
        return !!(entry && entry.isConnected && railParent && railParent.isConnected &&
                  entry.parentElement === railParent);
    }

    function install() {
        if (placed()) { return; }
        try {
            var existing = document.getElementById(ID);
            if (existing) { existing.remove(); }

            var rail = railButtons();
            var avatar = null;
            var template = null;
            for (var i = rail.length - 1; i >= 0; i--) {
                if (!avatar && rail[i].querySelector('img')) { avatar = rail[i]; continue; }
                if (avatar && !template && rail[i].querySelector('svg') &&
                    !rail[i].querySelector('img') && !rail[i].closest('[id^="whatsie-"]')) {
                    template = rail[i];
                }
            }
            if (!avatar || !template) { return; }

            var avatarWrapper = wrapperSharedWith(avatar, template);
            var templateWrapper = wrapperSharedWith(template, avatar);
            if (!avatarWrapper || !templateWrapper ||
                avatarWrapper.parentElement !== templateWrapper.parentElement) {
                return;
            }

            // Clone a whole neighbouring entry so it looks native without relying
            // on any obfuscated class; cloneNode drops its listeners.
            var entry = templateWrapper.cloneNode(true);
            entry.id = ID;
            var button = entry.querySelector('button') || entry;
            button.removeAttribute('data-navbar-item');
            button.removeAttribute('aria-pressed');
            button.removeAttribute('aria-selected');
            button.removeAttribute('aria-current');
            button.setAttribute('aria-label', 'Whatsie settings');
            button.setAttribute('title', 'Whatsie settings');
            var svg = button.querySelector('svg');
            if (svg) {
                svg.setAttribute('viewBox', '0 0 24 24');
                svg.innerHTML = GEAR;
            }
            entry.addEventListener('click', function (ev) {
                ev.preventDefault();
                ev.stopPropagation();
                var api = window.__whatsie;
                if (api && api.bridge && typeof api.bridge.openSettings === 'function') {
                    api.bridge.openSettings();
                }
            }, true);

            avatarWrapper.parentElement.insertBefore(entry, avatarWrapper);
            railParent = avatarWrapper.parentElement;
        } catch (e) {
            var api = window.__whatsie;
            api && api.report && api.report('nav-settings', e);
        }
    }

    // WhatsApp rebuilds the rail on navigation; a 1s timer re-places the button
    // (a MutationObserver over WA's constantly-mutating DOM would force layout
    // far too often — whatly's lesson).
    install();
    setInterval(install, 1000);
})();
