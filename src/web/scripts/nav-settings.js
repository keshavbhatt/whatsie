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
    // The Whatsie mark (speech bubble + W), viewBox 0 0 64 64, filled with
    // currentColor so it inherits the rail's icon colour. Sourced from
    // resources/icons/whatsie-symbolic.svg.
    var ICON_VIEWBOX = '0 0 64 64';
    var ICON =
        '<path fill="currentColor" d="M 32 5 C 17.098857 5 5 16.969621 5 31.710938 C 5 37.047389 6.5860348 42.020918 9.3144531 46.193359 L 5 59 L 18.179688 54.65625 C 22.223999 57.046126 26.949584 58.419922 32 58.419922 C 46.901143 58.419922 59 46.452254 59 31.710938 C 59 16.969621 46.901143 5 32 5 z M 32 10.308594 C 43.939672 10.308594 53.634766 19.899327 53.634766 31.710938 C 53.634766 43.522547 43.939672 53.109375 32 53.109375 C 27.481058 53.109375 23.28555 51.738327 19.8125 49.390625 L 11.679688 52.072266 L 14.365234 44.101562 C 11.847131 40.603908 10.365234 36.327615 10.365234 31.710938 C 10.365234 19.899327 20.060328 10.308594 32 10.308594 z"/>' +
        '<path fill="currentColor" d="m 16.831427,21.17152 q 1.756611,-0.320463 3.560698,-0.462891 0.486629,-0.03561 0.985127,-0.03561 1.353065,0 2.789213,0.272987 0.439153,2.36193 0.83083,4.723859 0.391676,2.350061 0.747746,4.759467 0.04748,0.23738 0.344201,0.308594 0.296725,0.05935 0.367939,-0.11869 0.367939,-0.973258 0.747747,-2.278847 0.391676,-1.317458 0.783353,-2.670524 0.391677,-1.364934 0.735878,-2.670523 0.35607,-1.317458 0.652794,-2.290716 1.673528,-0.142428 3.406402,-0.23738 0.379807,-0.02374 0.759615,-0.01187 1.341196,0 2.634917,0.249249 0.747746,2.36193 1.471755,4.664515 0.735878,2.302584 1.424279,4.71199 0.04748,0.11869 0.154297,0.261118 0.106821,0.130559 0.23738,0.11869 0.142428,-0.01187 0.272987,-0.225511 0.130559,-0.225511 0.213642,-0.759616 0.58158,-4.427134 1.068209,-8.972959 1.103817,-0.23738 2.314454,-0.23738 0.308594,-0.01187 0.629057,0.01187 1.554838,0.07121 3.204628,0.225511 -0.07121,1.697266 -0.284856,3.750602 -0.201773,2.041467 -0.498498,4.189755 -0.284855,2.148288 -0.617187,4.308445 -0.332332,2.148288 -0.664664,4.082934 -0.320463,1.922777 -0.617188,3.548829 -0.284855,1.614183 -0.510366,2.682392 -1.317459,0.09495 -2.504358,0.166166 -1.186899,0.07121 -2.338192,0.11869 -1.151292,0.04748 -2.314454,0.05935 -1.151292,0.01187 -2.409405,0.01187 -0.04748,-0.23738 -0.154297,-0.854567 -0.09495,-0.617188 -0.225511,-1.471756 -0.130559,-0.854567 -0.296725,-1.815956 -0.154297,-0.961388 -0.308594,-1.827825 -0.142428,-0.878305 -0.272987,-1.590445 -0.130559,-0.724008 -0.201773,-1.044471 -0.09495,-0.439153 -0.284855,-0.640926 -0.178035,-0.201773 -0.379808,-0.189904 -0.189904,0.01187 -0.367939,0.23738 -0.166166,0.213642 -0.284856,0.59345 -0.142428,0.58158 -0.403546,1.542969 -0.249248,0.961388 -0.557842,2.148288 -0.296725,1.17503 -0.617188,2.480619 -0.320463,1.293721 -0.605319,2.563703 -1.341196,0.09495 -2.46875,0.142428 -0.902044,0.03561 -1.756611,0.03561 -0.213642,0 -0.427284,0 -1.056341,-0.01187 -2.136419,-0.07121 -1.06821,-0.05935 -2.255109,-0.106821 -0.605319,-2.82482 -1.05634,-5.614034 -0.451022,-2.801082 -0.842699,-5.578427 -0.391677,-2.789214 -0.783354,-5.590296 -0.379807,-2.812952 -0.890174,-5.602165 z"/>';

    // Overlays that must freeze this script completely. The fullscreen media
    // viewer is the dangerous one: WhatsApp mounts it (data-testid
    // "drawer-fullscreen" / "media-viewer-modal") as a live, screen-covering,
    // position:fixed subtree — and the "Media" rail entry can host it. If we
    // re-clone the rail while that is up, cloneNode deep-copies the whole viewer
    // into our button, which then blankets the page and turns every click into
    // openSettings(). So while any of these is present we do not touch the DOM.
    var OVERLAY = '[role="dialog"],[aria-modal="true"],[data-animate-modal-popup],'
        + '[data-animate-modal-backdrop],[data-animate-media-viewer],'
        + '[data-testid="drawer-fullscreen"],[data-testid="media-viewer-modal"]';
    // The "the media viewer is actually up" test — and ONLY that. Its single job
    // is to stop us re-cloning the Media rail entry (the viewer's host) while the
    // viewer is mounted; nothing else needs to freeze the script or block a click
    // on our own button. It must therefore be a tight allowlist of the real viewer
    // markers. In particular it must NOT include [role="dialog"] / [aria-modal]:
    // WhatsApp's Meta AI page mounts a persistent, visible role="dialog" that never
    // goes away, which used to make this true forever and silently kill every
    // settings click after the user opened Meta AI. (drawer-fullscreen is likewise
    // always-present chrome and stays out too.)
    var OVERLAY_OPEN = '[data-animate-media-viewer],[data-testid="media-viewer-modal"]';
    function overlayOpen() {
        return !!document.querySelector(OVERLAY_OPEN);
    }
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
    //   avatar   = the profile, which is ALWAYS the bottom-most entry of the rail
    //              and carries an <img>. We therefore take the last rail entry and
    //              require it to be an image: WhatsApp's "Meta AI" entry is also an
    //              <img> but sits in the TOP group (never last), so this excludes
    //              it structurally — no locale-fragile label match. And before the
    //              avatar has loaded the bottom entry is a plain icon, so we wait
    //              rather than latch onto Meta AI and land in the wrong group (the
    //              cause of the icon appearing up top, above Meta AI).
    //   template = the icon (svg-only) button directly above that avatar, in the
    //              same group — cloned so our entry looks native and lands right
    //              above the profile.
    function anchors() {
        if (overlayOpen()) { return null; } // never mutate the DOM while a viewer/dialog is up
        var rail = railButtons();
        rail.sort(function (a, b) { return a.getBoundingClientRect().top - b.getBoundingClientRect().top; });
        if (!mainRailPresent(rail)) { return null; } // don't inject into a viewer/overlay
        var mine = function (b) { return b.closest('[id^="whatsie-"]'); };
        var avatar = null;
        for (var i = rail.length - 1; i >= 0; i--) {
            if (!mine(rail[i])) { avatar = rail[i]; break; } // bottom-most real entry
        }
        if (!avatar || !avatar.querySelector('img')) { return null; } // avatar not loaded yet
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
        // A rail entry is small. If the shared wrapper is large we have climbed
        // into some other container (e.g. a media-viewer panel) — never clone
        // that, or the clone becomes a full-area click target that hijacks every
        // click in the region (owner report: clicking anywhere in the gallery
        // opened Settings).
        var box = templateWrapper.getBoundingClientRect();
        if (box.width > 72 || box.height > 120) { return null; }
        return { avatarWrapper: avatarWrapper, templateWrapper: templateWrapper };
    }

    // Correctly placed only when our entry sits immediately above the CURRENT
    // avatar. This is deliberately position-based, not "is it still in the
    // remembered container": WhatsApp reveals the profile avatar late and rebuilds
    // the rail, so the anchor moves under us. Re-checking position each tick lets
    // install() lift a provisionally-placed button to the right spot once the
    // avatar loads — and also catches WhatsApp re-parenting our entry out of the
    // rail (then parentElement no longer matches and we re-place).
    function placed(a) {
        var entry = document.getElementById(ID);
        if (!entry || !entry.isConnected || !a) { return false; }
        // If our entry ever absorbed WhatsApp content (a stray data-testid means a
        // real WA subtree rode in on a clone), it is not correctly placed — force a
        // clean rebuild rather than leave a page-covering blob behind.
        if (entry.querySelector('[data-testid]')) { return false; }
        return entry.parentElement === a.avatarWrapper.parentElement &&
               entry.nextElementSibling === a.avatarWrapper;
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
            // Defence in depth: drop any WhatsApp-tagged descendant the clone may
            // carry, so our button can never smuggle in a live WA subtree (see the
            // OVERLAY note). Our own button is found by shape, not data-testid.
            Array.prototype.slice.call(entry.querySelectorAll('[data-testid]'))
                .forEach(function (n) { n.removeAttribute('data-testid'); });
            var button = entry.querySelector('button') || entry;
            button.removeAttribute('data-navbar-item');
            button.removeAttribute('aria-pressed');
            button.removeAttribute('aria-selected');
            button.removeAttribute('aria-current');
            button.setAttribute('aria-label', 'Whatsie settings');
            button.setAttribute('title', 'Whatsie settings');
            var svg = button.querySelector('svg');
            if (svg) {
                svg.setAttribute('viewBox', ICON_VIEWBOX);
                svg.innerHTML = ICON;
            }
            // NB: the click is handled by ONE delegated listener on document (set
            // up once, below) — deliberately NOT attached to this node. WhatsApp
            // rebuilds the rail by cloning it, and cloneNode drops listeners, so a
            // per-node handler silently dies after a few navigations (the button
            // then looked right but stopped opening Settings). Delegation survives.
            avatarWrapper.parentElement.insertBefore(entry, avatarWrapper);
        } catch (e) {
            var api = window.__whatsie;
            api && api.report && api.report('nav-settings', e);
        }
    }

    // One delegated click handler for the life of the page: fires whenever a
    // click lands inside our entry, however many times WhatsApp has re-cloned it.
    // Capture phase + stopPropagation so the cloned nav button's own behaviour
    // (it was copied from a real rail entry) never also runs.
    document.addEventListener('click', function (ev) {
        var target = ev.target;
        if (!target || typeof target.closest !== 'function' || !target.closest('#' + ID)) {
            return;
        }
        ev.preventDefault();
        ev.stopPropagation();
        if (overlayOpen()) { return; } // never act while a viewer/overlay is up
        var api = window.__whatsie;
        if (api && api.bridge && typeof api.bridge.openSettings === 'function') {
            api.bridge.openSettings();
        }
    }, true);

    // WhatsApp rebuilds the rail on navigation; a 1s timer re-places the button
    // (a MutationObserver over WA's constantly-mutating DOM would force layout
    // far too often).
    install();
    setInterval(install, 1000);
})();
