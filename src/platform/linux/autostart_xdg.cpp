#include "platform/autostart.h"
#include "platform/logging.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QtEnvironmentVariables>

using namespace Qt::StringLiterals;

namespace whatsie::platform {

namespace {

QString autostartFilePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + u"/autostart"_s;
    return dir + u"/"_s + QCoreApplication::instance()->property("desktopFileName").toString() + u".desktop"_s;
}

QString desktopId()
{
    // setDesktopFileName() was called with the reverse-DNS id (ADR-014).
    QString id = qApp->property("desktopFileName").toString();
    if (id.isEmpty()) {
        id = u"com.ktechpit.whatsie"_s;
    }
    return id;
}

QString launchCommand()
{
    // In a snap the wrapper command `whatsie` is stable across revisions; the
    // absolute binary path is not. Native builds use the binary path.
    if (qEnvironmentVariableIsSet("SNAP")) {
        return QCoreApplication::applicationName(); // == snap app name "whatsie"
    }
    return QCoreApplication::applicationFilePath();
}

} // namespace

bool autostartEnabled()
{
    return QFile::exists(autostartFilePath());
}

bool setAutostartEnabled(bool enabled, bool startMinimized)
{
    const QString path = autostartFilePath();
    if (!enabled) {
        if (!QFile::exists(path)) {
            return true;
        }
        const bool removed = QFile::remove(path);
        qCInfo(lcPlatform) << "autostart disabled:" << removed;
        return removed;
    }

    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(lcPlatform) << "cannot write autostart entry" << path << file.errorString();
        return false;
    }
    const QString exec = launchCommand() + (startMinimized ? u" --minimized"_s : QString());
    const QString content = u"[Desktop Entry]\n"
                            "Type=Application\n"
                            "Name=%1\n"
                            "Icon=%2\n"
                            "Exec=%3\n"
                            "X-GNOME-Autostart-enabled=true\n"_s.arg(QCoreApplication::applicationName(),
                                                                     desktopId(), exec);
    file.write(content.toUtf8());
    qCInfo(lcPlatform) << "autostart enabled ->" << exec;
    return true;
}

} // namespace whatsie::platform
