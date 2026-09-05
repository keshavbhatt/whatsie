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

    // We intercept the drag in the widget layer (so the drop works on Wayland /
    // Flatpak, where Chromium's native drop can't read the paths), which means
    // WhatsApp Web never sees the dragover and never shows its own drop overlay.
    // Draw our own so the target is as obvious as it is in a browser.
    var HINT_ID = '__whatsie_drop_hint';
    window.__whatsieDropHint = function (show) {
        try {
            var el = document.getElementById(HINT_ID);
            if (show) {
                if (!document.querySelector('#main')) {
                    return; // no chat open: nothing to attach to, so no drop zone
                }
                if (!el) {
                    el = document.createElement('div');
                    el.id = HINT_ID;
                    el.textContent = 'Drop files to send';
                    var s = el.style;
                    s.position = 'fixed';
                    s.inset = '0';
                    s.zIndex = '2147483646';
                    s.display = 'flex';
                    s.alignItems = 'center';
                    s.justifyContent = 'center';
                    s.font = '600 24px Segoe UI, Helvetica, Arial, sans-serif';
                    s.color = '#fff';
                    s.background = 'rgba(11,20,26,0.55)';
                    s.border = '3px dashed rgba(255,255,255,0.7)';
                    s.boxSizing = 'border-box';
                    s.pointerEvents = 'none'; // never intercept the drag/drop itself
                    document.body.appendChild(el);
                }
            } else if (el) {
                el.remove();
            }
        } catch (e) {
            api.report && api.report('drop-hint', e);
        }
    };

    // A transient toast for drops that could not be attached (e.g. a file the
    // sandbox can't read), so the failure is never silent.
    window.__whatsieDropNotice = function (msg) {
        try {
            if (!msg) {
                return;
            }
            var t = document.createElement('div');
            t.textContent = msg;
            var s = t.style;
            s.position = 'fixed';
            s.left = '50%';
            s.bottom = '84px';
            s.transform = 'translateX(-50%)';
            s.zIndex = '2147483647';
            s.maxWidth = '78%';
            s.padding = '12px 18px';
            s.borderRadius = '8px';
            s.background = 'rgba(30,30,30,0.96)';
            s.color = '#fff';
            s.font = '500 14px Segoe UI, Helvetica, Arial, sans-serif';
            s.lineHeight = '1.4';
            s.textAlign = 'center';
            s.boxShadow = '0 4px 16px rgba(0,0,0,0.4)';
            s.pointerEvents = 'none';
            document.body.appendChild(t);
            setTimeout(function () {
                t.style.transition = 'opacity .4s';
                t.style.opacity = '0';
            }, 4600);
            setTimeout(function () {
                t.remove();
            }, 5100);
        } catch (e) {
            api.report && api.report('drop-notice', e);
        }
    };

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
