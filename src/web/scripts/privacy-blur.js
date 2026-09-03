// name:     privacy-blur
// purpose:  blur message text and media until hovered, for over-the-shoulder
//           privacy (FEATURES A8). Level comes from window.__whatsie.config and
//           can be changed live via window.__whatsieSetBlur(level).
// depends:  own <style>; structural anchors #pane-side, #main and ARIA roles
//           (role=listitem/row) — WhatsApp's own class names are compiler-
//           generated and change every deploy, so we never key off them
// verified: 2026-09-03 against WhatsApp Web 2.3000.x
// on-fail:  no-op
(function () {
    'use strict';
    var api = window.__whatsie || {};
    var STYLE_ID = 'whatsie-privacy-blur';
    function radius(level) {
        return level <= 0 ? 0 : (level === 1 ? 3 : (level === 2 ? 6 : 10));
    }
    function apply(level) {
        try {
            var el = document.getElementById(STYLE_ID);
            if (level <= 0) {
                if (el) { el.remove(); }
                return;
            }
            if (!el) {
                el = document.createElement('style');
                el.id = STYLE_ID;
                (document.head || document.documentElement).appendChild(el);
            }
            var b = radius(level) + 'px';
            el.textContent =
                '#pane-side [role="listitem"] span[dir],' +
                '#main [role="row"],' +
                '#main img,#main video{filter:blur(' + b + ');transition:filter .12s ease}' +
                '#pane-side [role="listitem"]:hover span[dir],' +
                '#main [role="row"]:hover,' +
                '#main img:hover,#main video:hover{filter:none}';
        } catch (e) {
            api.report && api.report('privacy-blur', e);
        }
    }
    window.__whatsieSetBlur = apply;
    var level = (api.config && api.config.blurLevel) || 0;
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', function () { apply(level); });
    } else {
        apply(level);
    }
})();
