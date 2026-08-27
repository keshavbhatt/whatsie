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
#include <QTemporaryDir>
#include <QTest>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>

using namespace Qt::StringLiterals;
using namespace whatsie::web;

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
        QCOMPARE(outcome.files.size(), 1); // big skipped (cap), dir + missing ignored
        QCOMPARE(outcome.skipped, QStringList{u"big.bin"_s});
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
