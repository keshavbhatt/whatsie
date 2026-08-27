#!/usr/bin/env -S node --experimental-websocket --no-warnings
// Evaluate a JavaScript expression inside the WhatsApp Web page of a running
// whatsie started with QTWEBENGINE_REMOTE_DEBUGGING=<port>.
//
//   scripts/cdp-eval.mjs 'document.title'
//   scripts/cdp-eval.mjs --port 9222 'JSON.stringify(Object.keys(localStorage))'
//   scripts/cdp-eval.mjs --call Emulation.setEmulatedMedia '{"features":[{"name":"prefers-color-scheme","value":"light"}]}'
//
// Dev tool only: nothing here ships. Prints the JSON-serialised value.
const args = process.argv.slice(2);
let port = 9222;
const exprs = [];
const calls = [];
for (let i = 0; i < args.length; i++) {
    if (args[i] === '--port') { port = Number(args[++i]); }
    else if (args[i] === '--call') { calls.push([args[++i], JSON.parse(args[++i] || '{}')]); }
    else if (args[i] === '--sleep') { calls.push(['sleep', Number(args[++i])]); }
    else { exprs.push(args[i]); }
}
if (exprs.length === 0 && calls.length === 0) { console.error('usage: cdp-eval.mjs [--port N] <expr>'); process.exit(2); }

const targets = await (await fetch(`http://127.0.0.1:${port}/json`)).json();
const page = targets.find(t => t.type === 'page' && /(^|\.)whatsapp\.com$/.test(new URL(t.url).hostname));
if (!page) { console.error('no whatsapp page target; targets: ' + targets.map(t => t.url).join(', ')); process.exit(1); }

const ws = new WebSocket(page.webSocketDebuggerUrl);
await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
let nextId = 1;
const pending = new Map();
ws.onmessage = ev => {
    const m = JSON.parse(ev.data);
    if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); }
};
const call = (method, params = {}) => new Promise(res => {
    const id = nextId++;
    pending.set(id, res);
    ws.send(JSON.stringify({ id, method, params }));
});
await call('Runtime.enable');
for (const [method, params] of calls) {
    if (method === 'sleep') { await new Promise(r => setTimeout(r, params)); continue; }
    const r = await call(method, params);
    console.log(JSON.stringify(r.result ?? r.error));
}
for (const expression of exprs) {
    const r = await call('Runtime.evaluate', { expression, returnByValue: true, awaitPromise: true });
    if (r.result?.exceptionDetails) {
        console.error('exception:', r.result.exceptionDetails.text, r.result.exceptionDetails.exception?.description ?? '');
    } else {
        const v = r.result?.result?.value;
        console.log(typeof v === 'string' ? v : JSON.stringify(v));
    }
}
ws.close();
