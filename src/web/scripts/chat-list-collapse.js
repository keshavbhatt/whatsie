// name:     chat-list-collapse
// purpose:  a nav-rail button that collapses WhatsApp's chat-list pane to a
//           narrow strip of profile pictures, handing that width to the open
//           conversation (Telegram-style), and expands it again. The collapsed
//           state is remembered in localStorage, so there is no C++ setting.
// depends:  the rail's <button> entries and the avatar (last rail entry with an
//           <img>); structural anchors #side / #pane-side and same-shape
//           measurement for the pane columns. No obfuscated classes, no React
//           internals — a rule that stops matching stops collapsing rather than
//           breaking the page.
// verified: 2026-09-04 against WhatsApp Web 2.3000.x
// on-fail:  no-op; the list simply stays as WhatsApp draws it
(function () {
    'use strict';
    var api = window.__whatsie || {};
    var ID = 'whatsie-collapse-nav';
    var STYLE_ID = 'whatsie-chat-list-collapse';
    var PANE_ATTR = 'data-whatsie-pane';
    var STORAGE_KEY = 'whatsie:chatListCollapsed';
    // Avatar (48px) plus the padding WhatsApp's own row draws either side, so the
    // picture lands centred without overriding class-named padding. Measured
    // against a live page; a restyle makes it off-centre, never broken.
    var STRIP_WIDTH = '97px';

    // Feather/Lucide "panel-left" glyphs, stroked with currentColor so they match
    // the rail. The chevron points inward (collapse) when open, outward (expand)
    // when collapsed.
    var ICON_COLLAPSE =
        '<rect width="18" height="18" x="3" y="3" rx="2"/><path d="M9 3v18"/><path d="m16 15-3-3 3-3"/>';
    var ICON_EXPAND =
        '<rect width="18" height="18" x="3" y="3" rx="2"/><path d="M9 3v18"/><path d="m14 9 3 3-3 3"/>';

    var collapsed = false;
    try { collapsed = localStorage.getItem(STORAGE_KEY) === '1'; } catch (e) { /* private mode */ }

    // --- the collapse itself -------------------------------------------------

    function collapseCss() {
        // The pane width lives on an unnamed wrapper above #side, and a parallel
        // column pushes the conversation across; both are tagged by measurement
        // in tagPanes(). Pin grow/shrink (never the `flex` shorthand — the pane
        // column is in a column-direction flex parent, where flex-basis would set
        // its height) and set only the width.
        return '[' + PANE_ATTR + ']{flex-grow:0!important;flex-shrink:0!important;' +
               'flex-basis:auto!important;width:' + STRIP_WIDTH + '!important;' +
               'min-width:' + STRIP_WIDTH + '!important;max-width:' + STRIP_WIDTH + '!important}' +
               '#side,#pane-side{overflow-x:hidden!important}' +
               '#pane-side [role="row"]{overflow:hidden!important}';
    }

    function untagPanes() {
        document.querySelectorAll('[' + PANE_ATTR + ']').forEach(function (e) {
            e.removeAttribute(PANE_ATTR);
        });
    }

    // Tag every column the pane occupies: the wrapper that holds #side, and the
    // parallel column that shares its left edge and full height. The same-left,
    // same-height test separates those from the full-width containers that also
    // start there, and keeps working once the columns are STRIP_WIDTH wide.
    function tagPanes() {
        var side = document.querySelector('#side');
        if (!side || !side.parentElement) { return; }
        var wrap = side.parentElement;
        var w = wrap.getBoundingClientRect();
        untagPanes();
        document.querySelectorAll('div').forEach(function (e) {
            var r = e.getBoundingClientRect();
            if (Math.abs(r.left - w.left) > 0.6) { return; }
            if (Math.abs(r.height - w.height) > 0.6) { return; }
            e.setAttribute(PANE_ATTR, '1');
        });
        wrap.setAttribute(PANE_ATTR, '1');
    }

    function apply() {
        try {
            var el = document.getElementById(STYLE_ID);
            if (collapsed) {
                tagPanes();
                if (!el) {
                    el = document.createElement('style');
                    el.id = STYLE_ID;
                    (document.head || document.documentElement).appendChild(el);
                }
                el.textContent = collapseCss();
            } else {
                if (el) { el.remove(); }
                untagPanes();
            }
        } catch (e) {
            api.report && api.report('chat-list-collapse', e);
        }
    }

    function toggle() {
        collapsed = !collapsed;
        try { localStorage.setItem(STORAGE_KEY, collapsed ? '1' : '0'); } catch (e) { /* private mode */ }
        apply();
        install();
    }

    // --- the rail button (mirrors nav-settings' proven approach) -------------

    var OVERLAY_OPEN = '[data-animate-media-viewer],[data-testid="media-viewer-modal"]';
    function overlayOpen() { return !!document.querySelector(OVERLAY_OPEN); }

    function railButtons() {
        return Array.prototype.slice.call(document.querySelectorAll('button')).filter(function (b) {
            var r = b.getBoundingClientRect();
            return r.width > 0 && r.width <= 72 && r.left < 80;
        });
    }
    function mainRailPresent(rail) {
        var n = 0;
        for (var i = 0; i < rail.length; i++) {
            var b = rail[i];
            if (b.getBoundingClientRect().top < 320 && b.querySelector('svg') && !b.querySelector('img')) { n++; }
        }
        return n >= 3;
    }
    function wrapperSharedWith(node, other) {
        while (node.parentElement) {
            if (node.parentElement.contains(other)) { return node; }
            node = node.parentElement;
        }
        return null;
    }

    // Anchor above the Settings button when nav-settings has placed one, else
    // directly above the avatar — so the two injected entries stack instead of
    // fighting over the same slot immediately above the profile.
    function anchors() {
        if (overlayOpen()) { return null; }
        var rail = railButtons();
        rail.sort(function (a, b) { return a.getBoundingClientRect().top - b.getBoundingClientRect().top; });
        if (!mainRailPresent(rail)) { return null; }
        var mine = function (b) { return b.closest('[id^="whatsie-"]'); };
        var avatar = null;
        for (var i = rail.length - 1; i >= 0; i--) {
            if (!mine(rail[i])) { avatar = rail[i]; break; }
        }
        if (!avatar || !avatar.querySelector('img')) { return null; }
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
        var box = templateWrapper.getBoundingClientRect();
        if (box.width > 72 || box.height > 120) { return null; }
        // Sit above the Settings entry if present (and a sibling here), else above
        // the avatar.
        var settings = document.getElementById('whatsie-settings-nav');
        var reference = (settings && settings.parentElement === avatarWrapper.parentElement)
            ? settings : avatarWrapper;
        return { templateWrapper: templateWrapper, parent: avatarWrapper.parentElement, reference: reference };
    }

    function placed(a) {
        var entry = document.getElementById(ID);
        if (!entry || !entry.isConnected || !a) { return false; }
        if (entry.querySelector('[data-testid]')) { return false; }
        return entry.parentElement === a.parent && entry.nextElementSibling === a.reference;
    }

    function install() {
        try {
            var a = anchors();
            if (!a) { return; }
            var entry = document.getElementById(ID);
            if (!placed(a)) {
                if (entry) { entry.remove(); }
                entry = a.templateWrapper.cloneNode(true);
                entry.id = ID;
                Array.prototype.slice.call(entry.querySelectorAll('[data-testid]'))
                    .forEach(function (n) { n.removeAttribute('data-testid'); });
                a.parent.insertBefore(entry, a.reference);
            }
            var button = entry.querySelector('button') || entry;
            button.removeAttribute('data-navbar-item');
            button.removeAttribute('aria-pressed');
            button.removeAttribute('aria-selected');
            button.removeAttribute('aria-current');
            var label = collapsed ? 'Expand chat list' : 'Collapse chat list';
            button.setAttribute('aria-label', label);
            button.setAttribute('title', label);
            var svg = button.querySelector('svg');
            if (svg) {
                svg.setAttribute('viewBox', '0 0 24 24');
                svg.setAttribute('fill', 'none');
                svg.setAttribute('stroke', 'currentColor');
                svg.setAttribute('stroke-width', '2');
                svg.setAttribute('stroke-linecap', 'round');
                svg.setAttribute('stroke-linejoin', 'round');
                svg.innerHTML = collapsed ? ICON_EXPAND : ICON_COLLAPSE;
            }
        } catch (e) {
            api.report && api.report('chat-list-collapse', e);
        }
    }

    // One delegated click for the life of the page (cloneNode drops per-node
    // listeners, so a handler on the button itself would die on a rail rebuild).
    document.addEventListener('click', function (ev) {
        var target = ev.target;
        if (!target || typeof target.closest !== 'function' || !target.closest('#' + ID)) { return; }
        ev.preventDefault();
        ev.stopPropagation();
        if (overlayOpen()) { return; }
        toggle();
    }, true);

    // WhatsApp rebuilds the rail and the pane on navigation; a 1s timer re-places
    // the button and, while collapsed, re-tags the pane columns (a MutationObserver
    // over WA's constantly-mutating DOM would force layout far too often).
    function tick() {
        install();
        if (collapsed) { apply(); }
    }
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', function () { apply(); install(); });
    } else {
        apply();
        install();
    }
    setInterval(tick, 1000);
})();
