#include "app/cli_options.h"

#include <QCommandLineParser>

using namespace Qt::StringLiterals;

namespace whatsie::app {

bool CliOptions::hasCommands() const
{
    return newChat.has_value() || showSettings || quit || !urls.isEmpty();
}

CliParseResult parseCliOptions(const QStringList& arguments)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(u"WhatsApp Web desktop client"_s);
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    const QCommandLineOption help = parser.addHelpOption();
    const QCommandLineOption version = parser.addVersionOption();
    const QCommandLineOption profile({u"p"_s, u"profile"_s},
                                     u"Use a separate profile (own session and settings)."_s, u"name"_s);
    const QCommandLineOption minimized({u"m"_s, u"minimized"_s}, u"Start hidden in the system tray."_s);
    const QCommandLineOption logFile(
        u"log-file"_s, u"Write the log to <path> instead of the default location."_s, u"path"_s);
    const QCommandLineOption noLogFile(u"no-log-file"_s, u"Do not write a log file."_s);
    const QCommandLineOption newChat({u"n"_s, u"new-chat"_s},
                                     u"Open a chat with a phone number or wa.me / whatsapp:// link."_s,
                                     u"target"_s);
    const QCommandLineOption settings({u"s"_s, u"settings"_s}, u"Open the settings dialog."_s);
    const QCommandLineOption quit({u"q"_s, u"quit"_s}, u"Quit the running instance."_s);
    parser.addOptions({profile, minimized, logFile, noLogFile, newChat, settings, quit});
    parser.addPositionalArgument(u"url"_s, u"whatsapp:// or wa.me link to open."_s, u"[url]"_s);

    CliParseResult result;
    if (!parser.parse(arguments)) {
        result.errorText = parser.errorText();
        return result;
    }
    if (parser.isSet(help)) {
        result.helpRequested = true;
        result.helpText = parser.helpText();
        return result;
    }
    if (parser.isSet(version)) {
        result.versionRequested = true;
        return result;
    }

    CliOptions& o = result.options;
    o.profile = parser.value(profile).trimmed();
    o.startMinimized = parser.isSet(minimized);
    if (parser.isSet(logFile)) {
        o.logFile = parser.value(logFile);
    }
    o.noLogFile = parser.isSet(noLogFile);
    if (parser.isSet(newChat)) {
        o.newChat = parser.value(newChat);
    }
    o.showSettings = parser.isSet(settings);
    o.quit = parser.isSet(quit);
    o.urls = parser.positionalArguments();
    return result;
}

QList<QJsonObject> commandsFor(const CliOptions& options)
{
    QList<QJsonObject> commands;
    if (options.quit) {
        commands.append({{QString::fromLatin1(cmd::kKey), QString::fromLatin1(cmd::kQuit)}});
        return commands;
    }
    for (const QString& url : options.urls) {
        commands.append({{QString::fromLatin1(cmd::kKey), QString::fromLatin1(cmd::kNewChat)},
                         {QString::fromLatin1(cmd::kUrlKey), url}});
    }
    if (options.newChat) {
        commands.append({{QString::fromLatin1(cmd::kKey), QString::fromLatin1(cmd::kNewChat)},
                         {QString::fromLatin1(cmd::kUrlKey), *options.newChat}});
    }
    if (options.showSettings) {
        commands.append({{QString::fromLatin1(cmd::kKey), QString::fromLatin1(cmd::kSettings)}});
    }
    commands.append({{QString::fromLatin1(cmd::kKey), QString::fromLatin1(cmd::kRaise)}});
    return commands;
}

} // namespace whatsie::app
