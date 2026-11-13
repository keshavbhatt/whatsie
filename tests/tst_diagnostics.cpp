#include "core/settings/settings.h"
#include "ui/diagnostics.h"
#include "ui/settings_dialog.h"

#include <QLabel>
#include <QWebEngineProfile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QWebEnginePage>

#include <memory>
#include <algorithm>

using namespace Qt::StringLiterals;
using namespace whatsie::core;
using namespace whatsie::ui;

class TestDiagnostics : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_dir;
    Settings m_settings{m_dir.filePath(u"settings.ini"_s)};

    void setPageOffset(QWebEnginePage& page, const QString& script)
    {
        const auto completed = std::make_shared<bool>(false);
        page.runJavaScript(script, [completed](const QVariant&) { *completed = true; });
        QTRY_VERIFY_WITH_TIMEOUT(*completed, 5000);
    }

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(m_dir.isValid());
    }

    void advancedTimezoneHintIsVisibleAndDefaultIsSystem()
    {
        QWebEngineProfile profile;
        SettingsDialog dialog(m_settings, false, {}, profile);
        const auto labels = dialog.findChildren<QLabel*>();
        const bool hasHint = std::any_of(labels.cbegin(), labels.cend(), [](const QLabel* label) {
            return label->text() ==
                u"If timestamps appear incorrect after a recent timezone rule change, try selecting "
                "another timezone with the same UTC offset."_s;
        });
        QVERIFY(hasHint);
        QVERIFY(m_settings.timeZone().isEmpty());
    }

    void reportsIndependentSystemAndChromiumOffsets()
    {
        QWebEnginePage page;
        QSignalSpy loaded(&page, &QWebEnginePage::loadFinished);
        page.setHtml(u"<html><body>diagnostics</body></html>"_s);
        QTRY_VERIFY_WITH_TIMEOUT(!loaded.isEmpty(), 10000);

        setPageOffset(page, u"Date.prototype.getTimezoneOffset = () => 300"_s);
        const QString report = buildDiagnostics(m_settings, u"test"_s, &page, 0, false);
        QVERIFY(report.contains(u"- Chromium UTC offset: -05:00"_s));
        QVERIFY(report.contains(u"- System UTC offset: "_s));

        setPageOffset(page, u"Date.prototype.getTimezoneOffset = () => -210"_s);
        const QString positiveReport = buildDiagnostics(m_settings, u"test"_s, &page, 0, false);
        QVERIFY(positiveReport.contains(u"- Chromium UTC offset: +03:30"_s));
    }

    void invalidOrMissingChromiumOffsetIsUnavailable()
    {
        QWebEnginePage page;
        QSignalSpy loaded(&page, &QWebEnginePage::loadFinished);
        page.setHtml(u"<html><body>diagnostics</body></html>"_s);
        QTRY_VERIFY_WITH_TIMEOUT(!loaded.isEmpty(), 10000);

        setPageOffset(page, u"Date.prototype.getTimezoneOffset = () => NaN"_s);
        const QString invalid = buildDiagnostics(m_settings, u"test"_s, &page, 0, false);
        QVERIFY(invalid.contains(u"- Chromium UTC offset: unavailable"_s));

        const QString missing = buildDiagnostics(m_settings, u"test"_s, nullptr, 0, false);
        QVERIFY(missing.contains(u"- Chromium UTC offset: unavailable"_s));
    }
};

QTEST_MAIN(TestDiagnostics)
#include "tst_diagnostics.moc"
