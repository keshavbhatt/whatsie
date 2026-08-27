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

} // namespace whatsie::web
