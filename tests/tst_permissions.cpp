// Drives a real getUserMedia() request through a local custom scheme and
// checks that the user's answer is persisted for the whole camera/microphone
// family and reused without a second prompt (the "messy call permissions"
// report of 2026-08-27).

#include "web/permission_controller.h"

#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QRandomGenerator>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlScheme>
#include <QWebEngineUrlSchemeHandler>

using namespace Qt::StringLiterals;
using whatsie::web::PermissionController;
using PermissionType = QWebEnginePermission::PermissionType;
using State = QWebEnginePermission::State;

namespace {
const QByteArray kScheme = QByteArrayLiteral("wtest");
// Origins carry a trailing slash in QWebEnginePermission::origin().
const QUrl kOrigin(u"wtest://app/"_s);

class MediaPageHandler : public QWebEngineUrlSchemeHandler
{
public:
    void requestStarted(QWebEngineUrlRequestJob* job) override
    {
        auto* body = new QBuffer(job);
        body->setData(QByteArrayLiteral("<!doctype html><title>start</title><script>"
                                        "navigator.mediaDevices.getUserMedia({audio:true,video:true})"
                                        ".then(()=>{document.title='granted';})"
                                        ".catch(e=>{document.title='error:'+e.name;});"
                                        "</script>"));
        job->reply(QByteArrayLiteral("text/html"), body);
    }
};

/// A profile with on-disk permissions in a temp dir.
struct DiskProfile
{
    QTemporaryDir dir;
    QWebEngineProfile profile{u"permtest-%1"_s.arg(QRandomGenerator::global()->generate())};
    MediaPageHandler handler;

    DiskProfile()
    {
        profile.setPersistentStoragePath(dir.filePath(u"storage"_s));
        profile.setCachePath(dir.filePath(u"cache"_s));
        profile.setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
        profile.installUrlSchemeHandler(kScheme, &handler);
    }
    [[nodiscard]] QString permissionsFile() const { return dir.filePath(u"storage/permissions.json"_s); }
};
} // namespace

class TestPermissions : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void familyOfTypes()
    {
        const auto media = PermissionController::relatedTypes(PermissionType::MediaVideoCapture);
        QCOMPARE(media.size(), 3);
        QVERIFY(media.contains(PermissionType::MediaAudioVideoCapture));
        QCOMPARE(PermissionController::relatedTypes(PermissionType::Geolocation),
                 QList<PermissionType>{PermissionType::Geolocation});
    }

    void storedGrantCoversFamilyAndPersists()
    {
        DiskProfile p;
        QCOMPARE(PermissionController::storedState(p.profile, kOrigin, PermissionType::MediaVideoCapture),
                 State::Ask);
        // Simulate the controller's store step for one family member.
        p.profile.queryPermission(kOrigin, PermissionType::MediaAudioVideoCapture).grant();
        QCOMPARE(PermissionController::storedState(p.profile, kOrigin, PermissionType::MediaVideoCapture),
                 State::Granted);
        // Chromium's pref store commits lazily (~10 s); the in-memory state is
        // immediate, which is what the controller relies on.
        QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(p.permissionsFile()), 20000);
        QFile f(p.permissionsFile());
        QVERIFY(f.open(QIODevice::ReadOnly));
        // Chromium stores the combined type as its two halves.
        const QString content = QString::fromUtf8(f.readAll());
        QVERIFY2(content.contains(u"MediaAudioCapture"_s) && content.contains(u"MediaVideoCapture"_s),
                 qPrintable(content));

        // A denial anywhere in the family wins.
        p.profile.queryPermission(kOrigin, PermissionType::MediaAudioCapture).deny();
        QCOMPARE(PermissionController::storedState(p.profile, kOrigin, PermissionType::MediaVideoCapture),
                 State::Denied);
    }

    void promptOnceThenAnswerFromStore()
    {
        DiskProfile p;
        QWebEnginePage page(&p.profile);
        PermissionController controller;
        controller.attach(page);
        QSignalSpy prompts(&controller, &PermissionController::promptRequested);

        page.load(QUrl(u"wtest://app/"_s));
        if (!prompts.wait(10000)) {
            QSKIP("getUserMedia produced no permission request on this host (no media stack?)");
        }
        QCOMPARE(prompts.count(), 1);
        const auto permission = prompts.first().first().value<QWebEnginePermission>();
        QCOMPARE(permission.permissionType(), PermissionType::MediaAudioVideoCapture);
        QCOMPARE(permission.origin(), kOrigin);

        controller.answer(permission, true);
        QCOMPARE(PermissionController::storedState(p.profile, kOrigin, PermissionType::MediaAudioCapture),
                 State::Granted);
        QCOMPARE(PermissionController::storedState(p.profile, kOrigin, PermissionType::MediaVideoCapture),
                 State::Granted);
        // Second request (a reload): no prompt — answered from the store.
        QSignalSpy loaded(&page, &QWebEnginePage::loadFinished);
        page.load(QUrl(u"wtest://app/"_s));
        QVERIFY(loaded.wait(10000));
        QTest::qWait(1500);
        QCOMPARE(prompts.count(), 1);
    }

    void denialIsStoredToo()
    {
        DiskProfile p;
        QWebEnginePage page(&p.profile);
        PermissionController controller;
        controller.attach(page);
        QSignalSpy prompts(&controller, &PermissionController::promptRequested);
        page.load(QUrl(u"wtest://app/"_s));
        if (!prompts.wait(10000)) {
            QSKIP("getUserMedia produced no permission request on this host");
        }
        controller.answer(prompts.first().first().value<QWebEnginePermission>(), false);
        QCOMPARE(
            PermissionController::storedState(p.profile, kOrigin, PermissionType::MediaAudioVideoCapture),
            State::Denied);
        QCOMPARE(PermissionController::storedState(p.profile, kOrigin, PermissionType::MediaAudioCapture),
                 State::Denied);
    }
};

int main(int argc, char* argv[])
{
    QWebEngineUrlScheme scheme(kScheme);
    scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
    scheme.setFlags(QWebEngineUrlScheme::SecureScheme | QWebEngineUrlScheme::LocalScheme |
                    QWebEngineUrlScheme::LocalAccessAllowed);
    QWebEngineUrlScheme::registerScheme(scheme);

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    QApplication::setApplicationName(u"whatsie-permtest"_s);
    QApplication::setOrganizationName(u"ktechpit"_s);
    TestPermissions test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_permissions.moc"
