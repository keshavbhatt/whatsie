#include "platform/autostart.h"
#include "platform/logging.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

using namespace Qt::StringLiterals;

namespace whatsie::platform {

namespace {

constexpr auto kRunKey =
    R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)";
constexpr auto kValueName = "Whatsie";

QString launchCommand(bool startMinimized)
{
    const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    QString command = u"\"%1\""_s.arg(exe);
    if (startMinimized) {
        command += u" --minimized"_s;
    }
    return command;
}

} // namespace

bool autostartEnabled()
{
    QSettings run(QString::fromLatin1(kRunKey), QSettings::NativeFormat);
    return run.contains(QString::fromLatin1(kValueName));
}

bool setAutostartEnabled(bool enabled, bool startMinimized)
{
    QSettings run(QString::fromLatin1(kRunKey), QSettings::NativeFormat);
    if (enabled) {
        run.setValue(QString::fromLatin1(kValueName), launchCommand(startMinimized));
    } else {
        run.remove(QString::fromLatin1(kValueName));
    }
    run.sync();
    const bool ok = run.status() == QSettings::NoError;
    qCInfo(lcPlatform) << "autostart" << (enabled ? "enabled" : "disabled") << "->" << ok;
    return ok;
}

} // namespace whatsie::platform
