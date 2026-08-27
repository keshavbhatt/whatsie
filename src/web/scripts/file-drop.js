// name:     file-drop
// purpose:  attach files dragged onto the window by rebuilding real File objects
//           from bytes read in C++ and dispatching a synthetic drop on WhatsApp's
//           composer (FEATURES M6). Used when Chromium's native drop can't read
//           the paths (Wayland / Flatpak, Y#32).
// depends:  DataTransfer + DragEvent (standard); drop target #main footer or #main
// verified: 2026-08-27 against WhatsApp Web 2.3000.x
// on-fail:  no-op; reports via bridge
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
                var arr = new Uint8Array(bin.length);
                for (var i = 0; i < bin.length; i++) {
                    arr[i] = bin.charCodeAt(i);
                }
                dt.items.add(new File([arr], f.name, { type: f.type || 'application/octet-stream' }));
            });
            var target = document.querySelector('#main footer') || document.querySelector('#main') || document.body;
            var opts = { bubbles: true, cancelable: true, dataTransfer: dt };
            target.dispatchEvent(new DragEvent('dragenter', opts));
            target.dispatchEvent(new DragEvent('dragover', opts));
            target.dispatchEvent(new DragEvent('drop', opts));
            return true;
        } catch (e) {
            api.report && api.report('file-drop', e);
            return false;
        }
    };
})();
