#include "web/user_agent.h"

#include <QRegularExpression>

using namespace Qt::StringLiterals;

namespace whatsie::web {

QString sanitizeUserAgent(const QString& defaultUserAgent)
{
    static const QRegularExpression kQtToken(u"\\s*QtWebEngine/[\\d.]+"_s);
    QString ua = defaultUserAgent;
    ua.remove(kQtToken);
    return ua.simplified();
}

QString effectiveUserAgent(const QString& engineDefault, const QString& userOverride)
{
    const QString trimmed = userOverride.trimmed();
    return trimmed.isEmpty() ? sanitizeUserAgent(engineDefault) : trimmed;
}

} // namespace whatsie::web
