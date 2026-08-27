#include "web/bridge.h"

#include "web/logging.h"

namespace whatsie::web {

Bridge::Bridge(QObject* parent)
    : QObject(parent)
{}

void Bridge::scriptFailed(const QString& name, const QString& message)
{
    qCWarning(lcWebJs).noquote() << "script" << name << "failed:" << message;
    Q_EMIT scriptFailure(name, message);
}

void Bridge::log(const QString& message)
{
    qCDebug(lcWebJs).noquote() << "script:" << message;
}

void Bridge::connectionChanged(bool up)
{
    qCInfo(lcWebJs) << "connection" << (up ? "up" : "down");
    Q_EMIT connectionStateChanged(up);
}

void Bridge::openSettings()
{
    qCDebug(lcWebJs) << "nav settings button clicked";
    Q_EMIT settingsRequested();
}

} // namespace whatsie::web
