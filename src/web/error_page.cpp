#include "web/error_page.h"

using namespace Qt::StringLiterals;

namespace whatsie::web {

QString errorPageHtml(bool dark, const QString& title, const QString& detail)
{
    // Palette matches the app's WhatsApp-style light/dark themes.
    const QString bg = dark ? u"#0b141a"_s : u"#f0f2f5"_s;
    const QString fg = dark ? u"#e9edef"_s : u"#111b21"_s;
    const QString muted = dark ? u"#8696a0"_s : u"#667781"_s;
    const QString accent = u"#25d366"_s;
    const QString accentHover = u"#1da851"_s;
    const QString ring = dark ? u"rgba(37,211,102,0.16)"_s : u"rgba(37,211,102,0.20)"_s;

    QString html = uR"HTML(<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Whatsie</title>
<style>
  html,body{height:100%;margin:0}
  body{background:{{BG}};color:{{FG}};
    font-family:'Segoe UI',Ubuntu,'Helvetica Neue',Helvetica,Arial,sans-serif;
    display:flex;align-items:center;justify-content:center;text-align:center;
    -webkit-user-select:none;user-select:none}
  .wrap{max-width:420px;padding:32px 24px}
  .mark{width:96px;height:96px;border-radius:50%;margin:0 auto 26px;
    background:{{ACCENT}};box-shadow:0 0 0 12px {{RING}};
    display:flex;align-items:center;justify-content:center}
  .mark svg{width:52px;height:52px;fill:none;stroke:#fff;stroke-width:6;
    stroke-linecap:round;stroke-linejoin:round}
  h1{font-size:22px;font-weight:600;margin:0 0 10px}
  p{font-size:15px;line-height:1.5;color:{{MUTED}};margin:0 0 26px}
  button{appearance:none;border:0;cursor:pointer;
    background:{{ACCENT}};color:#fff;font-size:15px;font-weight:600;
    padding:12px 30px;border-radius:24px;transition:background .15s ease}
  button:hover{background:{{ACCENTHOVER}}}
  button:active{transform:translateY(1px)}
</style></head>
<body>
  <div class="wrap">
    <div class="mark">
      <svg viewBox="0 0 64 64" aria-hidden="true">
        <path d="M6 6 L58 58"/>
        <path d="M14 30a26 26 0 0 1 36 0"/>
        <path d="M22 40a14 14 0 0 1 20 0"/>
        <circle cx="32" cy="50" r="2.5" fill="#fff" stroke="none"/>
      </svg>
    </div>
    <h1>{{TITLE}}</h1>
    <p>{{DETAIL}}</p>
    <button onclick="try{window.__whatsie.bridge.retry()}catch(e){location.href='https://web.whatsapp.com/'}">Try again</button>
  </div>
</body></html>)HTML"_s;

    html.replace(u"{{BG}}"_s, bg);
    html.replace(u"{{FG}}"_s, fg);
    html.replace(u"{{MUTED}}"_s, muted);
    html.replace(u"{{ACCENT}}"_s, accent);
    html.replace(u"{{ACCENTHOVER}}"_s, accentHover);
    html.replace(u"{{RING}}"_s, ring);
    html.replace(u"{{TITLE}}"_s, title.toHtmlEscaped());
    html.replace(u"{{DETAIL}}"_s, detail.toHtmlEscaped());
    return html;
}

} // namespace whatsie::web
