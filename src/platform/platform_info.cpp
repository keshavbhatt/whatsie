#include "platform/platform_info.h"

#include <QGuiApplication>
#include <QLibraryInfo>
#include <QStringList>
#include <QSysInfo>
#include <QtEnvironmentVariables>

using namespace Qt::StringLiterals;

namespace whatsie::platform {

QString describeHost()
{
    QStringList parts;
    parts << QSysInfo::prettyProductName();
    parts << u"Qt %1 (built against %2)"_s.arg(QString::fromLatin1(qVersion()),
                                               QLibraryInfo::version().toString());
    if (QGuiApplication::instance() != nullptr) {
        parts << u"platform: %1"_s.arg(QGuiApplication::platformName());
    }
    const QByteArray desktop = qgetenv("XDG_CURRENT_DESKTOP");
    if (!desktop.isEmpty()) {
        parts << u"desktop: %1"_s.arg(QString::fromUtf8(desktop));
    }
    if (isSandboxed()) {
        parts << u"sandboxed"_s;
    }
    return parts.join(u", "_s);
}

bool isSandboxed()
{
    return qEnvironmentVariableIsSet("SNAP") || qEnvironmentVariableIsSet("FLATPAK_ID");
}

} // namespace whatsie::platform
