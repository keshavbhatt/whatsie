// More apps from Ktechpit in About and on the lock screen. Runs offscreen; with
// WHATSIE_GRAB_DIR set it also saves screenshots of both themes there for review.

#include "core/settings/settings.h"
#include "core/theme/theme_service.h"
#include "ui/about_dialog.h"
#include "ui/lock_screen.h"
#include "ui/whatsapp_style.h"

#include "AppCatalog.h"
#include "MoreAppsDialog.h"

#include <QApplication>
#include <QDir>
#include <QFrame>
#include <QLineEdit>
#include <QPushButton>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>

using namespace Qt::StringLiterals;
using whatsie::core::ThemeService;

namespace {

void applyTheme(bool dark)
{
    QApplication::setPalette(dark ? ThemeService::darkPalette() : ThemeService::lightPalette());
    qApp->setStyleSheet(whatsie::ui::whatsappStyleSheet(dark));
}

void grab(QWidget& widget, const QString& name)
{
    const QString dir = qEnvironmentVariable("WHATSIE_GRAB_DIR");
    if (!dir.isEmpty()) {
        QDir().mkpath(dir);
        widget.grab().save(QDir(dir).filePath(name + u".png"_s));
    }
}

} // namespace

class TestMoreApps : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        QVERIFY(m_dir.isValid());
        m_settings = std::make_unique<whatsie::core::Settings>(m_dir.filePath(u"more-apps.ini"_s));
    }

    void aboutShowsTheStripInsteadOfTheButton()
    {
        applyTheme(true);
        whatsie::ui::AboutDialog dialog(*m_settings, u"test agent"_s);
        dialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialog));

        for (const QPushButton* button : dialog.findChildren<QPushButton*>()) {
            QVERIFY2(button->text() != u"More Applications"_s, "the old button is gone");
        }
        auto* box = dialog.findChild<QFrame*>(u"aboutMoreApps"_s);
        QVERIFY(box != nullptr);
        QVERIFY(box->isVisible());
        QCOMPARE(box->findChildren<QFrame*>(u"ktechpitAppCard"_s).size(), 2);
    }

    void lockLinkFollowsTheSetting()
    {
        whatsie::ui::LockScreen lock;
        lock.resize(900, 600);
        auto* link = lock.findChild<QPushButton*>(u"lockMoreAppsLink"_s);
        QVERIFY(link != nullptr);
        QVERIFY(!link->isVisibleTo(&lock)); // hidden until told
        QVERIFY(lock.findChildren<QFrame*>(u"ktechpitAppCard"_s).isEmpty()); // a link, no cards

        lock.setOtherAppsVisible(true);
        QVERIFY(link->isVisibleTo(&lock));
        lock.setOtherAppsVisible(false);
        QVERIFY(!link->isVisibleTo(&lock));
    }

    void lockLinkHidesWhenTheCatalogIsOff()
    {
        whatsie::ui::LockScreen lock;
        lock.setOtherAppsVisible(true);
        auto* link = lock.findChild<QPushButton*>(u"lockMoreAppsLink"_s);
        QVERIFY(link->isVisibleTo(&lock));
        ktechpit::AppCatalog::instance()->setEnabled(false);
        QVERIFY(!link->isVisibleTo(&lock));
        ktechpit::AppCatalog::instance()->setEnabled(true);
        QVERIFY(link->isVisibleTo(&lock));
    }

    void lockLinkSitsUnderTheCard()
    {
        whatsie::ui::LockScreen lock;
        lock.resize(1600, 1050);
        lock.setOtherAppsVisible(true);
        lock.show();
        QVERIFY(QTest::qWaitForWindowExposed(&lock));
        QTest::qWait(500); // the card's rise
        const QWidget* card = lock.findChild<QFrame*>(u"lockCard"_s);
        const QWidget* link = lock.findChild<QPushButton*>(u"lockMoreAppsLink"_s);
        QCOMPARE(link->geometry().top() - card->geometry().bottom() - 1, 18);
        QCOMPARE(link->geometry().center().x(), card->geometry().center().x());
        grab(lock, u"lock-tall"_s);
    }

    void lockLinkHandsFocusBackToThePasscode()
    {
        whatsie::ui::LockScreen lock;
        lock.resize(900, 600);
        lock.setOtherAppsVisible(true);
        lock.show();
        lock.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&lock));
        lock.reset();
        auto* input = lock.findChild<QLineEdit*>();
        QTRY_VERIFY(input->hasFocus());

        auto* link = lock.findChild<QPushButton*>(u"lockMoreAppsLink"_s);
        link->setFocus(Qt::TabFocusReason); // tabbing in is left alone
        QTest::qWait(50);
        QVERIFY(link->hasFocus());
        lock.findChild<QFrame*>(u"lockCard"_s)->findChild<QPushButton*>()->setFocus(Qt::TabFocusReason); // Unlock, so the next focus is new
        link->setFocus(Qt::ActiveWindowFocusReason); // back from the list: to the passcode
        QTRY_VERIFY(input->hasFocus());
    }

    void screenshots_data()
    {
        QTest::addColumn<bool>("dark");
        QTest::newRow("dark") << true;
        QTest::newRow("light") << false;
    }

    void screenshots()
    {
        if (qEnvironmentVariableIsEmpty("WHATSIE_GRAB_DIR")) {
            QSKIP("set WHATSIE_GRAB_DIR to save screenshots");
        }
        QFETCH(bool, dark);
        applyTheme(dark);
        const QString theme = dark ? u"dark"_s : u"light"_s;

        whatsie::ui::AboutDialog about(*m_settings, u"test agent"_s);
        about.show();
        QVERIFY(QTest::qWaitForWindowExposed(&about));
        QTest::qWait(500); // the fade-in
        grab(about, u"about-"_s + theme);

        whatsie::ui::LockScreen lock;
        lock.resize(1000, 640);
        lock.setOtherAppsVisible(true);
        lock.show();
        QVERIFY(QTest::qWaitForWindowExposed(&lock));
        lock.reset();
        QTest::qWait(500);
        grab(lock, u"lock-"_s + theme);

        ktechpit::MoreAppsDialog list(u"lock"_s);
        list.show();
        QVERIFY(QTest::qWaitForWindowExposed(&list));
        QTest::qWait(100);
        grab(list, u"more-apps-"_s + theme);
    }

private:
    QTemporaryDir m_dir;
    std::unique_ptr<whatsie::core::Settings> m_settings;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(u"whatsie-more-apps"_s);
    QApplication::setOrganizationName(u"ktechpit"_s);
    QApplication::setStyle(QStyleFactory::create(u"Fusion"_s));
    ktechpit::CatalogConfig config;
    config.hostAppId = u"whatsie"_s;
    config.source = u"whatsie"_s;
    config.oneLineText = true; // as in main.cpp
    config.refreshDelayMs = 24 * 60 * 60 * 1000; // never fetch during the test
    ktechpit::AppCatalog::init(config);
    TestMoreApps test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_more_apps.moc"
