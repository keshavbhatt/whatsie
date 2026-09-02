#include "ui/diagnostics.h"

#include "core/log_sink.h"
#include "core/settings/settings.h"
#include "platform/crash_handler.h"
#include "platform/platform_info.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QStandardPaths>
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

QString buildDiagnostics(const core::Settings& settings, const QString& userAgent, int logLines)
{
    QStringList out;
    out << u"### Whatsie diagnostics"_s;
    out << u"- App: %1 %2"_s.arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion());
    out << u"- Host: %1"_s.arg(platform::describeHost());
    out << u"- Chromium: %1"_s.arg(chromiumVersion(userAgent));
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

    const QString crash = platform::lastCrashReport();
    if (!crash.isEmpty()) {
        out << QString() << u"<details><summary>Previous crash</summary>"_s << QString() << u"```"_s;
        for (const QString& line : crash.split(u'\n')) {
            out << line;
        }
        out << u"```"_s << u"</details>"_s;
    }
    return out.join(u'\n');
}

QString bugReportUrl(const core::Settings& settings, const QString& userAgent)
{
    const QString body =
        u"**What happened?**\n\n\n"
        "**Steps to reproduce**\n1. \n2. \n3. \n\n"
        "**Expected behaviour**\n\n\n"
        "---\n"_s +
        buildDiagnostics(settings, userAgent, 25);

    // Keep the whole URL comfortably under the ~8k limit browsers/GitHub accept;
    // the caller also copies the full diagnostics to the clipboard as a backup.
    constexpr int kMaxBody = 6000;
    const QString trimmed =
        body.size() > kMaxBody
            ? body.left(kMaxBody) + u"\n\n… truncated — full diagnostics are on your clipboard."_s
            : body;

    QUrl url(u"https://github.com/keshavbhatt/whatsie/issues/new"_s);
    QUrlQuery query;
    query.addQueryItem(u"labels"_s, u"bug"_s);
    query.addQueryItem(u"title"_s, u"[Bug] "_s);
    query.addQueryItem(u"body"_s, trimmed);
    url.setQuery(query);
    return url.toString(QUrl::FullyEncoded);
}

} // namespace whatsie::ui
