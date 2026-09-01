// name:     nav-settings
// purpose:  add a Settings entry to WhatsApp's left nav rail so the app is
//           reachable without a system tray (FEATURES A11). A Qt widget cannot
//           be drawn over the QWebEngineView (and a floating window cannot track
//           it on Wayland), so the button must live in the page. Approach: clone
//           a neighbouring rail entry and re-place it on a timer (proven robust
//           against WhatsApp's rail rebuilds). Click calls back through the
//           QWebChannel bridge.
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

    // Buttons in the far-left column, excluding any inside a modal/overlay (the
    // media viewer, dialogs) — those must never be treated as rail entries or we
    // would clone one and hijack, e.g., the viewer's close button.
    var OVERLAY = '[role="dialog"],[aria-modal="true"],[data-animate-modal-popup],'
        + '[data-animate-modal-backdrop],[data-animate-media-viewer]';
    function railButtons() {
        return Array.prototype.slice.call(document.querySelectorAll('button')).filter(function (b) {
            if (b.closest(OVERLAY)) { return false; }
            var r = b.getBoundingClientRect();
            return r.width > 0 && r.width <= 72 && r.left < 80;
        });
    }
    // The persistent nav rail is a vertical stack of icon (svg, no img) buttons
    // near the top-left. A media viewer / modal has no such stack, so this tells
    // "real rail is present" from "some transient overlay is showing".
    function mainRailPresent(rail) {
        var n = 0;
        for (var i = 0; i < rail.length; i++) {
            var b = rail[i];
            if (b.getBoundingClientRect().top < 320 && b.querySelector('svg') && !b.querySelector('img')) {
                n++;
            }
        }
        return n >= 3;
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

    // The two anchors we place relative to, recomputed every tick so the button
    // tracks WhatsApp's rail instead of sticking wherever it first landed:
    //   avatar   = the BOTTOM-MOST rail button with an <img> (the profile). This
    //              matters because WhatsApp's "Meta AI" entry is also an <img>;
    //              picking the last one avoids latching onto Meta AI before the
    //              profile avatar has loaded (the cause of the wandering icon).
    //   template = the icon (svg-only) button directly above that avatar, in the
    //              same group — cloned so our entry looks native and lands right
    //              above the profile.
    function anchors() {
        var rail = railButtons();
        rail.sort(function (a, b) { return a.getBoundingClientRect().top - b.getBoundingClientRect().top; });
        if (!mainRailPresent(rail)) { return null; } // don't inject into a viewer/overlay
        var mine = function (b) { return b.closest('[id^="whatsie-"]'); };
        var avatar = null;
        for (var i = rail.length - 1; i >= 0; i--) {
            if (!mine(rail[i]) && rail[i].querySelector('img')) { avatar = rail[i]; break; }
        }
        if (!avatar) { return null; }
        var avatarTop = avatar.getBoundingClientRect().top;
        var template = null;
        for (var j = rail.length - 1; j >= 0; j--) {
            var b = rail[j];
            if (b !== avatar && !mine(b) && b.querySelector('svg') && !b.querySelector('img') &&
                b.getBoundingClientRect().top < avatarTop) {
                template = b;
                break;
            }
        }
        if (!template) { return null; }
        var avatarWrapper = wrapperSharedWith(avatar, template);
        var templateWrapper = wrapperSharedWith(template, avatar);
        if (!avatarWrapper || !templateWrapper ||
            avatarWrapper.parentElement !== templateWrapper.parentElement) {
            return null;
        }
        return { avatarWrapper: avatarWrapper, templateWrapper: templateWrapper };
    }

    // Placed correctly only when our entry sits immediately above the current
    // avatar; if WhatsApp reflows the rail (or the real avatar appears after a
    // provisional placement), this turns false and install() moves it.
    function placed(a) {
        var entry = document.getElementById(ID);
        return !!(entry && entry.isConnected && a &&
                  entry.parentElement === a.avatarWrapper.parentElement &&
                  entry.nextElementSibling === a.avatarWrapper);
    }

    function install() {
        try {
            var a = anchors();
            if (!a) { return; }
            if (placed(a)) { return; }

            var existing = document.getElementById(ID);
            if (existing) { existing.remove(); }
            var avatarWrapper = a.avatarWrapper;
            var templateWrapper = a.templateWrapper;

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
                if (!anchors()) { return; } // never act while a viewer/overlay is up
                var api = window.__whatsie;
                if (api && api.bridge && typeof api.bridge.openSettings === 'function') {
                    api.bridge.openSettings();
                }
            }, true);

            avatarWrapper.parentElement.insertBefore(entry, avatarWrapper);
        } catch (e) {
            var api = window.__whatsie;
            api && api.report && api.report('nav-settings', e);
        }
    }

    // WhatsApp rebuilds the rail on navigation; a 1s timer re-places the button
    // (a MutationObserver over WA's constantly-mutating DOM would force layout
    // far too often).
    install();
    setInterval(install, 1000);
})();
