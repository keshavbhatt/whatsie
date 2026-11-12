// Script bundle + web policies on a real (off-the-record) profile.

#include "core/settings/settings.h"
#include "core/theme/theme_service.h"
#include "web/clipboard_fix.h"
#include "web/file_drop.h"
#include "web/permission_controller.h"
#include "web/script_bundle.h"
#include "web/web_page.h"

#include <QDir>
#include <QJSEngine>
#include <QJsonObject>
#include <QMimeData>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>
#include <QWebEngineView>

using namespace Qt::StringLiterals;
using namespace whatsie::web;

namespace {

// Runs `js` in the page and waits for its result.
QVariant runJs(QWebEnginePage* page, const QString& js)
{
    QVariant out;
    bool done = false;
    page->runJavaScript(js, [&](const QVariant& v) {
        out = v;
        done = true;
    });
    if (!QTest::qWaitFor([&] { return done; }, 5000)) {
        return u"<timeout>"_s;
    }
    return out;
}

// A cut-down WhatsApp nav rail: four icon entries up top, then an icon entry and
// the profile avatar at the bottom. The bottom icon entry also hosts a
// full-screen, position:fixed viewer (#viewer, with a "next" button), the way
// WhatsApp mounts its status viewer (issue #409). The viewer does not grow its
// entry's box, which is what fooled the old size check. The entry also carries a
// wide tooltip, which must not count as a viewer.
constexpr auto kRailFixture = R"(<!doctype html><html><head><style>
body { margin: 0; }
nav { position: fixed; left: 0; top: 0; width: 64px; height: 100vh;
      display: flex; flex-direction: column; }
.bottom { margin-top: auto; }
.e { width: 48px; height: 48px; margin: 4px 8px; }
.e button { width: 40px; height: 40px; padding: 0; }
.e svg { width: 24px; height: 24px; }
#viewer { position: fixed; inset: 0; background: #222; z-index: 10; }
.tip { position: absolute; left: 60px; width: 150px; height: 28px; }
#next { position: absolute; right: 20px; top: 300px; width: 40px; height: 40px; }
</style></head><body>
<nav>
  <div class="top">
    <div class="e"><div><button><svg viewBox="0 0 1 1"></svg></button></div></div>
    <div class="e"><div><button><svg viewBox="0 0 1 1"></svg></button></div></div>
    <div class="e"><div><button><svg viewBox="0 0 1 1"></svg></button></div></div>
    <div class="e"><div><button><svg viewBox="0 0 1 1"></svg></button></div></div>
  </div>
  <div class="bottom">
    <div class="e" id="tw"><div><button><svg viewBox="0 0 1 1"></svg></button></div>
      <span class="tip">Media and starred</span>
      <div id="host"><div id="viewer"><button id="next">next</button></div></div></div>
    <div class="e" id="aw"><div><button><img width="32" height="32"
      src="data:image/gif;base64,R0lGODlhAQABAAAAACw="></button></div></div>
  </div>
</nav>
<script>
window.__opened = 0;
window.__whatsie = { bridge: { openSettings: function () { window.__opened++; } } };
</script>
</body></html>)";

} // namespace

class TestWebScripts : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void everyScriptResourceParses()
    {
        QJSEngine engine;
        for (const QString& resource : ScriptBundle::bootstrapResources()) {
            const QString source = ScriptBundle::readResource(resource);
            QVERIFY2(!source.isEmpty(), qPrintable(resource));
            const QJSValue result = engine.evaluate(u"(function(){ %1 })"_s.arg(source), resource);
            QVERIFY2(!result.isError(), qPrintable(resource + u": "_s + result.toString()));
        }
    }

    // Force-requiring a WhatsApp module resolves its whole dependency subtree
    // before WhatsApp has loaded it, which corrupts the module system and blocks
    // login. Scripts may inspect the registry (require('__debug')) but must never
    // require('WAWeb…'): see scripts/README.md (the "stable tier" rule).
    void noForcedWhatsAppModuleRequires()
    {
        static const QRegularExpression forbidden(u"\\brequire\\s*\\(\\s*['\"]WAWeb"_s);
        for (const QString& resource : ScriptBundle::bootstrapResources()) {
            const QString source = ScriptBundle::readResource(resource);
            QVERIFY2(!source.contains(forbidden), qPrintable(resource));
        }
    }

    // WhatsApp JSON-parses its 'theme' and 'system-theme-mode' localStorage keys
    // on startup; a raw `dark` (or a missing mode key) makes it fall back to
    // following the OS, so the chosen theme was lost on every restart.
    void themeControlPersistsJsonForWhatsApp()
    {
        QJSEngine engine;
        const QJSValue stubs = engine.evaluate(uR"(
            var window = this;
            var store = {};
            var localStorage = {
                setItem: function (k, v) { store[k] = String(v); },
                removeItem: function (k) { delete store[k]; }
            };
            var document = {
                readyState: 'complete',
                body: null,
                documentElement: { style: {}, setAttribute: function () {} },
                querySelector: function () { return null; }
            };
            var location = { href: 'https://web.whatsapp.com/' };
            function setTimeout() {}
        )"_s);
        QVERIFY2(!stubs.isError(), qPrintable(stubs.toString()));

        const QString resource = u":/scripts/theme-control.js"_s;
        const QJSValue loaded = engine.evaluate(ScriptBundle::readResource(resource), resource);
        QVERIFY2(!loaded.isError(), qPrintable(loaded.toString()));

        for (const QString& mode : {u"dark"_s, u"light"_s}) {
            const QJSValue applied = engine.evaluate(u"__whatsieSetTheme('%1')"_s.arg(mode));
            QVERIFY2(!applied.isError(), qPrintable(applied.toString()));
            QCOMPARE(engine.evaluate(u"JSON.parse(store['theme'])"_s).toString(), mode);
            QCOMPARE(engine.evaluate(u"JSON.parse(store['system-theme-mode'])"_s).toBool(), false);
        }
    }

    void bootstrapInstallsOnceAndReplaces()
    {
        QWebEngineProfile profile; // off-the-record
        ScriptBundle bundle(profile);
        bundle.installBootstrap({{u"theme"_s, u"dark"_s}});
        QVERIFY(bundle.isInstalled(u"bootstrap"_s));
        QCOMPARE(profile.scripts()->find(ScriptBundle::scriptName(u"bootstrap"_s)).size(), 1);
        const QWebEngineScript first =
            profile.scripts()->find(ScriptBundle::scriptName(u"bootstrap"_s)).first();
        QCOMPARE(first.injectionPoint(), QWebEngineScript::DocumentCreation);
        QCOMPARE(first.worldId(), static_cast<quint32>(QWebEngineScript::MainWorld));
        QVERIFY(first.sourceCode().startsWith(u"window.__whatsieConfig = {\"theme\":\"dark\"};"_s));
        QVERIFY(first.sourceCode().contains(u"QWebChannel"_s));
        QVERIFY(first.sourceCode().contains(u"storage-persist"_s));

        bundle.installBootstrap({{u"theme"_s, u"light"_s}});
        QCOMPARE(profile.scripts()->find(ScriptBundle::scriptName(u"bootstrap"_s)).size(), 1);
        QVERIFY(profile.scripts()
                    ->find(ScriptBundle::scriptName(u"bootstrap"_s))
                    .first()
                    .sourceCode()
                    .contains(u"\"light\""_s));
        bundle.remove(u"bootstrap"_s);
        QVERIFY(!bundle.isInstalled(u"bootstrap"_s));
    }

    // Issue #409: with the status viewer open, our rail entry was rebuilt as a
    // deep copy of the entry hosting the viewer. The dead copy covered the page,
    // so tapping "next" or closing the status opened Whatsie's Settings.
    void navSettingsNeverCopiesAHostedViewer()
    {
        QWebEngineProfile profile; // off-the-record
        QWebEngineView view;
        auto* page = new QWebEnginePage(&profile, &view);
        view.setPage(page);
        view.resize(1000, 700);
        view.show();
        QSignalSpy loaded(page, &QWebEnginePage::loadFinished);
        page->setHtml(QString::fromUtf8(kRailFixture), QUrl(u"https://web.whatsapp.com/"_s));
        QVERIFY(loaded.wait(15000));
        QVERIFY(loaded.first().first().toBool());
        runJs(page, ScriptBundle::readResource(u":/scripts/nav-settings.js"_s));

        // Viewer up: let the 1 s timer run a few times.
        QTest::qWait(2500);
        const QString oversized = u"(function () {"
                                  u"  var n = document.getElementById('whatsie-settings-nav');"
                                  u"  if (!n) { return false; }"
                                  u"  return Array.prototype.some.call(n.querySelectorAll('*'), function (e) {"
                                  u"    var r = e.getBoundingClientRect();"
                                  u"    return r.width > 72 || r.height > 120; });"
                                  u"})()"_s;
        QCOMPARE(runJs(page, oversized).toBool(), false);
        // "next" reaches the viewer, not our button.
        QCOMPARE(runJs(page, u"(function () { var e = document.elementFromPoint(950, 320);"
                             u" e.click(); return e.closest('#whatsie-settings-nav') ? 'ours' : e.id; })()"_s)
                     .toString(),
                 u"next"_s);
        QCOMPARE(runJs(page, u"window.__opened"_s).toInt(), 0);

        // Viewer closed: our entry lands right above the avatar, without the host.
        runJs(page, u"document.getElementById('viewer').remove()"_s);
        const QString placed = u"(function () {"
                               u"  var n = document.getElementById('whatsie-settings-nav');"
                               u"  return !!n && n.nextElementSibling === document.getElementById('aw')"
                               u"    && !n.querySelector('#host')"
                               u"    && n.querySelector('svg').getAttribute('viewBox') === '0 0 64 64';"
                               u"})()"_s;
        QTRY_VERIFY_WITH_TIMEOUT(runJs(page, placed).toBool(), 5000);
        runJs(page, u"document.querySelector('#whatsie-settings-nav button').click()"_s);
        QCOMPARE(runJs(page, u"window.__opened"_s).toInt(), 1);

        // A stray page-sized copy already inside our entry (users on 6.2.0) is
        // dropped on click, never turned into Settings, and the entry comes back.
        QCOMPARE(runJs(page, u"(function () { var d = document.createElement('div');"
                             u" d.id = 'stray'; d.style.cssText = 'position:fixed;inset:0;z-index:20';"
                             u" document.getElementById('whatsie-settings-nav').appendChild(d);"
                             u" document.elementFromPoint(500, 300).click();"
                             u" return [window.__opened, !!document.getElementById('stray')].join(); })()"_s)
                     .toString(),
                 u"1,false"_s);
        QTRY_VERIFY_WITH_TIMEOUT(runJs(page, placed).toBool(), 5000);
    }

    void permissionPolicy()
    {
        using P = QWebEnginePermission::PermissionType;
        using D = PermissionController::Decision;
        QCOMPARE(PermissionController::decide(P::Notifications), D::Grant);
        QCOMPARE(PermissionController::decide(P::DesktopVideoCapture), D::Grant);
        QCOMPARE(PermissionController::decide(P::ClipboardReadWrite), D::Grant);
        QCOMPARE(PermissionController::decide(P::MediaAudioVideoCapture), D::Ask);
        QCOMPARE(PermissionController::decide(P::Geolocation), D::Ask);
        QCOMPARE(PermissionController::decide(P::LocalFontsAccess), D::Deny);
        QCOMPARE(PermissionController::decide(P::Unsupported), D::Deny);
        QVERIFY(!PermissionController::describe(P::MediaVideoCapture).isEmpty());
    }

    void fileFilters()
    {
        QCOMPARE(nameFilterFor({}), u"All files (*)"_s);
        const QString images = nameFilterFor({u"image/png"_s, u".webp"_s});
        QVERIFY(images.startsWith(u"Supported files ("_s));
        QVERIFY(images.contains(u"*.png"_s));
        QVERIFY(images.contains(u"*.webp"_s));
        QVERIFY(images.endsWith(u";;All files (*)"_s));
        QVERIFY(nameFilterFor({u"image/*"_s}).contains(u"*.jpg"_s));
    }

    void dropPayloadReadsCapsAndSkips()
    {
        QTemporaryDir dir;
        const QString ok = dir.filePath(u"photo.png"_s);
        {
            QFile f(ok);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(QByteArrayLiteral("\x89PNG\r\n\x1a\n hello"));
        }
        const QString big = dir.filePath(u"big.bin"_s);
        {
            QFile f(big);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(QByteArray(2048, 'x'));
        }
        QVERIFY(QDir(dir.path()).mkpath(u"adir"_s));

        const auto outcome = whatsie::web::buildDropPayload(
            {ok, big, dir.filePath(u"adir"_s), dir.filePath(u"missing"_s)}, 1024);
        QCOMPARE(outcome.files.size(), 1); // dir ignored
        QCOMPARE(outcome.tooLarge, QStringList{u"big.bin"_s});   // over the cap
        QCOMPARE(outcome.unreadable, QStringList{u"missing"_s}); // not visible / gone
        const QJsonObject f = outcome.files.first().toObject();
        QCOMPARE(f.value(u"name"_s).toString(), u"photo.png"_s);
        QCOMPARE(f.value(u"type"_s).toString(), u"image/png"_s);
        QVERIFY(!f.value(u"b64"_s).toString().isEmpty());
        QCOMPARE(QByteArray::fromBase64(f.value(u"b64"_s).toString().toLatin1()),
                 QByteArrayLiteral("\x89PNG\r\n\x1a\n hello"));
    }

    void clipboardNeedsPng()
    {
        QMimeData withImage;
        QImage img(2, 2, QImage::Format_ARGB32);
        img.fill(Qt::red);
        withImage.setImageData(img);
        QVERIFY(needsPngRepresentation(withImage));
        withImage.setData(u"image/png"_s, QByteArrayLiteral("x"));
        QVERIFY(!needsPngRepresentation(withImage));
        QMimeData text;
        text.setText(u"hi"_s);
        QVERIFY(!needsPngRepresentation(text));
    }
};

QTEST_MAIN(TestWebScripts)
#include "tst_web_scripts.moc"
