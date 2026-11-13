#include "ui/diagnostics.h"

#include "core/log_sink.h"
#include "core/settings/settings.h"
#include "core/timezone.h"
#include "platform/crash_handler.h"
#include "platform/platform_info.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QRegularExpression>
#include <QPointer>
#include <QStandardPaths>
#include <QTimeZone>
#include <QTimer>
#include <QWebEnginePage>

#include <cmath>
#include <memory>
#include <optional>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

namespace {
QString chromiumVersion(const QString& userAgent)
{
    static const QRegularExpression kChrome(u"Chrome/([\\d.]+)"_s);
    const QRegularExpressionMatch m = kChrome.match(userAgent);
    return m.hasMatch() ? m.captured(1) : u"unknown"_s;
}
} // namespace

QString buildDiagnostics(const core::Settings& settings, const QString& userAgent,
                         QWebEnginePage* page, int logLines, bool includeCrash)
{
    QStringList out;
    out << u"### Whatsie diagnostics"_s;
    out << u"- App: %1 %2"_s.arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion());
    out << u"- Host: %1"_s.arg(platform::describeHost());
    out << u"- Chromium: %1"_s.arg(chromiumVersion(userAgent));
    const QTimeZone systemZone = QTimeZone::systemTimeZone();
    const std::optional<int> systemOffset = systemZone.isValid()
        ? std::optional<int>(systemZone.offsetFromUtc(QDateTime::currentDateTime()))
        : std::nullopt;
    std::optional<int> chromiumOffset;
    if (page != nullptr) {
        QEventLoop loop;
        QPointer<QEventLoop> loopGuard(&loop);
        struct ChromiumOffsetResult
        {
            std::optional<int> offset;
            bool done = false;
        };
        const auto result = std::make_shared<ChromiumOffsetResult>();
        QTimer timeout;
        timeout.setSingleShot(true);
        QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
        page->runJavaScript(u"new Date().getTimezoneOffset()"_s, [result, loopGuard](const QVariant& value) {
            bool ok = false;
            const double minutesWest = value.toDouble(&ok);
            if (ok && std::isfinite(minutesWest) && std::floor(minutesWest) == minutesWest &&
                std::abs(minutesWest) <= 24 * 60) {
                result->offset = static_cast<int>(-minutesWest * 60.0);
            }
            result->done = true;
            if (loopGuard != nullptr) {
                loopGuard->quit();
            }
        });
        timeout.start(1500);
        if (!result->done) {
            loop.exec();
        }
        chromiumOffset = result->offset;
    }
    out << u"- System UTC offset: %1"_s.arg(core::formatUtcOffset(systemOffset));
    out << u"- Chromium UTC offset: %1"_s.arg(core::formatUtcOffset(chromiumOffset));
    out << u"- User agent: `%1`"_s.arg(userAgent);
    out << u"- Settings: `%1`"_s.arg(settings.fileName());
    out << u"- Data: `%1`"_s.arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    out << u"- Log file: `%1`"_s.arg(core::LogSink::logFilePath().isEmpty() ? u"(disabled)"_s
                                                                            : core::LogSink::logFilePath());
    out << QString();
    out << u"<details><summary>Recent log</summary>"_s << QString() << u"```"_s;
    const QStringList lines = core::LogSink::recentLines();
    const qsizetype start = std::max<qsizetype>(0, lines.size() - logLines);
    for (qsizetype i = start; i < lines.size(); ++i) {
        out << lines.at(i);
    }
    out << u"```"_s << u"</details>"_s;

    if (includeCrash) {
        const QString crash = platform::lastCrashReport();
        if (!crash.isEmpty()) {
            out << QString() << u"<details><summary>Previous crash</summary>"_s << QString()
                << u"```"_s;
            for (const QString& line : crash.split(u'\n')) {
                out << line;
            }
            out << u"```"_s << u"</details>"_s;
        }
    }
    return out.join(u'\n');
}

QString bugReportUrl(const QString& userAgent, const QString& title, const QString& description)
{
    // Only the user's title, description and a short environment block go in the
    // URL — logs and the crash backtrace would blow past GitHub's length limit
    // and get the request rejected ("Malformed request"). Diagnostics are copied
    // to the clipboard by the caller; the body must not repeat them, or pasting
    // would duplicate the prefilled sections.
    const QString described = description.trimmed().isEmpty()
                                  ? u"<!-- describe the problem here -->"_s
                                  : description.trimmed().left(1500);
    const QString body =
        u"**What happened?**\n\n"_s + described +
        u"\n\n**Steps to reproduce**\n1. \n2. \n3. \n\n"
        "**Expected behaviour**\n\n\n"
        "**Environment**\n"_s +
        u"- Whatsie: %1\n"_s.arg(QCoreApplication::applicationVersion()) +
        u"- OS: %1\n"_s.arg(platform::describeHost()) +
        u"- Chromium: %1\n\n"_s.arg(chromiumVersion(userAgent)) +
        u"<!-- Paste the diagnostics from your clipboard below. -->"_s;

    QString issueTitle = u"[Bug] "_s;
    issueTitle += title.trimmed().section(u'\n', 0, 0).left(80);

    QUrl url(u"https://github.com/keshavbhatt/whatsie/issues/new"_s);
    QUrlQuery query;
    query.addQueryItem(u"labels"_s, u"bug"_s);
    query.addQueryItem(u"title"_s, issueTitle);
    query.addQueryItem(u"body"_s, body);
    url.setQuery(query);
    return url.toString(QUrl::FullyEncoded);
}

} // namespace whatsie::ui
