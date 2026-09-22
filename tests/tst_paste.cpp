#include "core/settings/settings.h"
#include "core/theme/theme_service.h"
#include "web/web_view.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QProcess>
#include <QImage>
#include <QMimeData>
#include <memory>
#include <cstdio>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>
#include <QVBoxLayout>
#ifdef WHATSIE_HAVE_XTEST
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif

using namespace Qt::StringLiterals;

class TestPaste : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void clipboardPayload_data()
    {
        QTest::addColumn<bool>("image");
        QTest::newRow("rich-text") << false;
        QTest::newRow("image") << true;
    }

    void clipboardPayload()
    {
        QFETCH(bool, image);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        whatsie::core::Settings settings(dir.filePath(u"settings.ini"_s));
        whatsie::core::ThemeService theme(settings);
        whatsie::web::WebView view(settings, theme);
        QSignalSpy loaded(&view, &QWebEngineView::loadFinished);
        view.setHtml(u"<div contenteditable='true'></div><script>window.result=null;"
                     "document.addEventListener('paste',e=>{e.preventDefault();"
                     "window.result={text:e.clipboardData.getData('text/plain'),"
                     "html:e.clipboardData.getData('text/html'),"
                     "image:Array.from(e.clipboardData.items).some(i=>i.type==='image/png')};});"
                     "document.querySelector('div').focus();</script>"_s);
        view.show();
        QVERIFY(loaded.wait());
        QVERIFY(loaded.first().first().toBool());
        view.setFocus();
        QVERIFY(view.focusProxy());
        auto* mime = new QMimeData;
        if (image) {
            QImage bitmap(4, 4, QImage::Format_RGB32);
            bitmap.fill(Qt::red);
            mime->setImageData(bitmap);
        } else {
            mime->setText(u"bold"_s);
            mime->setHtml(u"<b>bold</b>"_s);
        }
        QApplication::clipboard()->setMimeData(mime);
        QTest::keyClick(view.focusProxy(), Qt::Key_V, Qt::ControlModifier);
        auto result = std::make_shared<QVariantMap>();
        auto read = [&] {
            view.page()->runJavaScript(u"window.result"_s, [result](const QVariant& value) {
                *result = value.toMap();
            });
            return !result->isEmpty();
        };
        QTRY_VERIFY(read());
        if (image) {
            QVERIFY(result->value(u"image"_s).toBool());
        } else {
            QCOMPARE(result->value(u"text"_s).toString(), u"bold"_s);
            QVERIFY(result->value(u"html"_s).toString().contains(u"<b>bold</b>"_s));
        }
        // Non-shortcut text input must still reach the editor.
        QTest::keyClicks(view.focusProxy(), u"hello"_s);
        auto text = std::make_shared<QString>();
        auto readText = [&] {
            view.page()->runJavaScript(u"document.querySelector('div').textContent"_s,
                                      [text](const QVariant& value) { *text = value.toString(); });
            return *text;
        };
        QTRY_COMPARE(readText(), u"hello"_s);
    }

    void singlePaste_data()
    {
        QTest::addColumn<int>("repetitions");
        QTest::addColumn<bool>("shiftInsert");
        QTest::newRow("single") << 1 << false;
        QTest::newRow("intentional-repeat") << 2 << false;
        QTest::newRow("shift-insert") << 1 << true;
    }

    void singlePaste()
    {
        QFETCH(int, repetitions);
        QFETCH(bool, shiftInsert);
        QProcess clipboardOwner;
        const bool native = qEnvironmentVariableIsSet("WHATSIE_NATIVE_PASTE");
        if (native) {
#ifndef WHATSIE_HAVE_XTEST
            QSKIP("XTest is unavailable");
#endif
            if (QGuiApplication::platformName() != u"xcb"_s)
                QSKIP("Native paste regression requires X11");
            clipboardOwner.start(QCoreApplication::applicationFilePath(), {u"--clipboard-owner"_s});
            QVERIFY(clipboardOwner.waitForStarted());
            QVERIFY(clipboardOwner.waitForReadyRead());
            QCOMPARE(clipboardOwner.readAllStandardOutput().trimmed(), QByteArray("ready"));
        }
        // QProcess closes the isolated clipboard owner on every exit path.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        whatsie::core::Settings settings(dir.filePath(u"settings.ini"_s));
        whatsie::core::ThemeService theme(settings);
        QWidget window;
        QVBoxLayout layout(&window);
        whatsie::web::WebView view(settings, theme, &window);
        layout.addWidget(&view);
        window.resize(600, 300);
        QSignalSpy loaded(&view, &QWebEngineView::loadFinished);
        view.setHtml(u"<div id='editor' contenteditable='true'></div>"
                     "<script>window.pastes=0;document.addEventListener('paste',"
                     "e=>{window.pastes++;e.preventDefault();const text=e.clipboardData.getData('text/plain');"
                     "setTimeout(()=>document.execCommand('insertText',false,text),0);});"
                     "document.querySelector('#editor').focus();</script>"_s);
        window.show();
        QVERIFY(loaded.wait());
        QVERIFY(loaded.first().first().toBool());
        view.setFocus();
        QVERIFY(view.focusProxy());
        if (!native)
            QApplication::clipboard()->setText(u"Test Alpha 731"_s);
        for (int iteration = 0; iteration < repetitions; ++iteration) {
            const auto key = shiftInsert ? Qt::Key_Insert : Qt::Key_V;
            const auto modifier = shiftInsert ? Qt::ShiftModifier : Qt::ControlModifier;
            // Exercise the ShortcutOverride phase before queued native key events.
            QKeyEvent shortcut(QEvent::ShortcutOverride, key, modifier);
            QApplication::sendEvent(view.focusProxy(), &shortcut);
            if (native) {
#ifdef WHATSIE_HAVE_XTEST
                Display* display = XOpenDisplay(nullptr);
                QVERIFY(display);
                XSetInputFocus(display, window.winId(), RevertToParent, CurrentTime);
                XSync(display, false);
                QTest::qWait(100);
                const auto control = XKeysymToKeycode(display, shiftInsert ? XK_Shift_L : XK_Control_L);
                const auto v = XKeysymToKeycode(display, shiftInsert ? XK_Insert : XK_v);
                XTestFakeKeyEvent(display, control, true, 0);
                XFlush(display);
                XTestFakeKeyEvent(display, v, true, 0);
                XFlush(display);
                XTestFakeKeyEvent(display, v, false, 0);
                XFlush(display);
                XTestFakeKeyEvent(display, control, false, 0);
                XFlush(display);
                XSync(display, false);
                XCloseDisplay(display);
#else
                QSKIP("XTest is unavailable");
#endif
            } else {
                QTest::keyClick(view.focusProxy(), key, modifier, 0);
            }
            struct State { QString text; int pastes = 0; int reads = 0; };
            auto state = std::make_shared<State>();
            auto read = [&] {
                view.page()->runJavaScript(u"[document.querySelector('#editor').textContent,window.pastes]"_s,
                                          [state](const QVariant& value) {
                    ++state->reads;
                    const auto values = value.toList();
                    if (values.size() == 2) {
                        state->text = values.at(0).toString();
                        state->pastes = values.at(1).toInt();
                    }
                });
                return state->text;
            };
            const QString expected = u"Test Alpha 731"_s.repeated(iteration + 1);
            QTRY_COMPARE(read(), expected);
            QTest::qWait(150);
            const int previousReads = state->reads;
            read();
            QTRY_VERIFY(state->reads > previousReads);
            QCOMPARE(state->pastes, iteration + 1);
            QCOMPARE(state->text, expected);
        }
        if (native) {
            clipboardOwner.terminate();
            QVERIFY(clipboardOwner.waitForFinished());
        }
    }
};

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setApplicationName(u"whatsie-paste-test"_s);
    if (app.arguments().contains(u"--clipboard-owner"_s)) {
        QApplication::clipboard()->setText(u"Test Alpha 731"_s);
        std::puts("ready");
        std::fflush(stdout);
        return app.exec();
    }
    TestPaste test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_paste.moc"
