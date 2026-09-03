#include "web/script_bundle.h"

#include "web/logging.h"

#include <QFile>
#include <QJsonDocument>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>

using namespace Qt::StringLiterals;

namespace whatsie::web {

namespace {
const QString kBootstrapName = u"bootstrap"_s;
}

ScriptBundle::ScriptBundle(QWebEngineProfile& profile)
    : m_profile(profile)
{}

QString ScriptBundle::scriptName(const QString& name)
{
    return u"whatsie:"_s + name;
}

QString ScriptBundle::readResource(const QString& resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCCritical(lcWeb) << "missing script resource" << resourcePath;
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QStringList ScriptBundle::bootstrapResources()
{
    return {
        u":/qtwebchannel/qwebchannel.js"_s,    u":/scripts/bootstrap.js"_s,
        u":/scripts/theme-control.js"_s,       u":/scripts/storage-persist.js"_s,
        u":/scripts/connection-watchdog.js"_s, u":/scripts/sw-recovery.js"_s,
        u":/scripts/privacy-blur.js"_s,        u":/scripts/nav-settings.js"_s,
        u":/scripts/linked-device-name.js"_s, u":/scripts/file-drop.js"_s,
        u":/scripts/chat-list-collapse.js"_s,
    };
}

void ScriptBundle::installBootstrap(const QJsonObject& config)
{
    // Config travels as JSON — never string-concatenated user text.
    QString source = u"window.__whatsieConfig = "_s +
                     QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Compact)) + u";\n"_s;
    for (const QString& resource : bootstrapResources()) {
        source += readResource(resource);
        source += u"\n"_s;
    }
    installSource(kBootstrapName, source, QWebEngineScript::DocumentCreation);
}

void ScriptBundle::installResource(const QString& name, const QString& resourcePath,
                                   QWebEngineScript::InjectionPoint point)
{
    installSource(name, readResource(resourcePath), point);
}

void ScriptBundle::installSource(const QString& name, const QString& source,
                                 QWebEngineScript::InjectionPoint point)
{
    remove(name);
    if (source.isEmpty()) {
        return;
    }
    QWebEngineScript script;
    script.setName(scriptName(name));
    script.setSourceCode(source);
    script.setInjectionPoint(point);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setRunsOnSubFrames(false);
    m_profile.scripts()->insert(script);
    qCDebug(lcWeb) << "installed script" << script.name() << "at" << point;
}

void ScriptBundle::remove(const QString& name)
{
    const QList<QWebEngineScript> existing = m_profile.scripts()->find(scriptName(name));
    for (const QWebEngineScript& s : existing) {
        m_profile.scripts()->remove(s);
    }
}

bool ScriptBundle::isInstalled(const QString& name) const
{
    return !m_profile.scripts()->find(scriptName(name)).isEmpty();
}

} // namespace whatsie::web
