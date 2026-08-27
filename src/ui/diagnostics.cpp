#include "ui/diagnostics.h"

#include "core/log_sink.h"
#include "core/settings/settings.h"
#include "platform/platform_info.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringList>

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
    return out.join(u'\n');
}

} // namespace whatsie::ui
