// name:     connection-watchdog
// purpose:  report when WhatsApp's WebSocket connection drops / recovers so C++
//           can reload after a suspend (FEATURES S13). Reload policy lives in
//           C++ (core::ConnectionWatchdogPolicy) — this only observes.
// depends:  window.WebSocket (standard API); WhatsApp uses it for its link
// verified: 2026-08-27 against WhatsApp Web 2.3000.x
// on-fail:  no-op (native WebSocket kept)
(function () {
    'use strict';
    var api = window.__whatsie;
    if (!api) {
        return;
    }
    try {
        var Native = window.WebSocket;
        if (!Native || Native.__whatsiePatched) {
            return;
        }
        var openCount = 0;
        var reported = null;
        function isWhatsApp(url) {
            try {
                return /(^|\.)whatsapp\.(net|com)$/.test(new URL(url, location.href).host);
            } catch (e) {
                return false;
            }
        }
        // The reload policy exists to recover an established session after a
        // suspend/network drop — not to run during login. On the landing / QR /
        // phone-number screens WhatsApp opens and closes short-lived sockets whose
        // churn would report the link "down" and trigger a reload that aborts the
        // pairing handshake (both QR and "link with phone number"). The chat pane
        // exists only once logged in, so gate "down" reports on it.
        function loggedIn() {
            return !!document.querySelector('#pane-side');
        }
        function report(up) {
            if (up === reported) {
                return;
            }
            if (!up && !loggedIn()) {
                return; // never report the link down before the user is logged in
            }
            reported = up;
            if (api.bridge) {
                api.bridge.connectionChanged(up);
            }
        }
        function Patched(url, protocols) {
            var ws = protocols === undefined ? new Native(url) : new Native(url, protocols);
            if (isWhatsApp(url)) {
                // A failing socket fires both 'error' and 'close'; count each
                // socket's open once and release it once, so one blip does not
                // over-decrement and falsely report the whole link down.
                var counted = false;
                var settled = false;
                ws.addEventListener('open', function () { counted = true; openCount++; report(true); });
                var drop = function () {
                    if (settled || !counted) { return; }
                    settled = true;
                    openCount = Math.max(0, openCount - 1);
                    if (openCount === 0) { report(false); }
                };
                ws.addEventListener('close', drop);
                ws.addEventListener('error', drop);
            }
            return ws;
        }
        Patched.prototype = Native.prototype;
        ['CONNECTING', 'OPEN', 'CLOSING', 'CLOSED'].forEach(function (k) { Patched[k] = Native[k]; });
        Patched.__whatsiePatched = true;
        window.WebSocket = Patched;
    } catch (e) {
        api.report('connection-watchdog', e);
    }
})();
