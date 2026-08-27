#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

#include <optional>

// Command line surface (FEATURES S9). Every option maps to a UI action or a
// startup mode — nothing here automates messaging.
namespace whatsie::app {

struct CliOptions
{
    QString profile;                ///< --profile <name>; empty = default
    bool startMinimized = false;    ///< --minimized
    std::optional<QString> logFile; ///< --log-file <path>; nullopt = default location
    bool noLogFile = false;         ///< --no-log-file
    std::optional<QString> newChat; ///< --new-chat <phone|url>
    bool showSettings = false;      ///< --settings
    bool quit = false;              ///< --quit (tell the running instance to exit)
    QStringList urls;               ///< positional whatsapp:// / wa.me links

    /// True when the invocation only carries commands for a running instance.
    [[nodiscard]] bool hasCommands() const;
};

struct CliParseResult
{
    CliOptions options;
    bool helpRequested = false;
    bool versionRequested = false;
    QString helpText;  ///< populated when helpRequested
    QString errorText; ///< non-empty on a parse error

    [[nodiscard]] bool ok() const { return errorText.isEmpty(); }
};

/// Parses `arguments` (including argv[0]). Pure: does not touch the
/// application object, print, or exit.
[[nodiscard]] CliParseResult parseCliOptions(const QStringList& arguments);

/// IPC commands to forward to an already-running instance, in order.
/// Always ends with a "raise" unless --quit was given.
[[nodiscard]] QList<QJsonObject> commandsFor(const CliOptions& options);

// Command vocabulary shared by sender and receiver (ADR-010).
namespace cmd {
inline constexpr auto kKey = "cmd";
inline constexpr auto kUrlKey = "url";
inline constexpr auto kRaise = "raise";
inline constexpr auto kNewChat = "new-chat";
inline constexpr auto kSettings = "settings";
inline constexpr auto kQuit = "quit";
} // namespace cmd

} // namespace whatsie::app
