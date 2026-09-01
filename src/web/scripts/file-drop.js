// name:     file-drop
// purpose:  attach files dragged onto the window (FEATURES M6). Used when
//           Chromium's native drop can't read the paths (Wayland / Flatpak).
// depends:  DataTransfer + ClipboardEvent (standard); #main footer, contenteditable
// verified: 2026-08-27 against WhatsApp Web 2.3000.x
// on-fail:  no-op; reports via bridge
//
// A synthetic HTML5 *drop* event does NOT reach WhatsApp Web's handler, but a
// synthetic *paste* on the focused composer does — it opens the media editor for
// images/videos and the document preview otherwise, exactly like a real drop
(function () {
    'use strict';
    var api = window.__whatsie || {};
    window.__whatsieDropFiles = function (files) {
        try {
            if (!files || !files.length) {
                return false;
            }
            var dt = new DataTransfer();
            files.forEach(function (f) {
                var bin = atob(f.b64);
                var bytes = new Uint8Array(bin.length);
                for (var i = 0; i < bin.length; i++) {
                    bytes[i] = bin.charCodeAt(i);
                }
                dt.items.add(new File([bytes], f.name, { type: f.type || 'application/octet-stream' }));
            });

            // Paste lands on the focused editable box. With a media preview open a
            // caption box appears OUTSIDE the footer (and is the right target); with
            // nothing open only the chat composer in the footer exists.
            function visible(el) { return el && el.offsetParent !== null; }
            function inFooter(el) { return !!(el && el.closest && el.closest('#main footer')); }
            var boxes = document.querySelectorAll('[contenteditable="true"]');
            var caption = null, composer = null;
            for (var b = 0; b < boxes.length; b++) {
                if (!visible(boxes[b])) {
                    continue;
                }
                if (inFooter(boxes[b])) {
                    if (!composer) { composer = boxes[b]; }
                } else if (!caption) {
                    caption = boxes[b];
                }
            }
            var target = caption || composer;
            if (!target) {
                api.log && api.log('file-drop: no chat open to attach to');
                return false;
            }
            target.focus();
            target.dispatchEvent(new ClipboardEvent('paste', { clipboardData: dt, bubbles: true, cancelable: true }));
            return true;
        } catch (e) {
            api.report && api.report('file-drop', e);
            return false;
        }
    };
})();
