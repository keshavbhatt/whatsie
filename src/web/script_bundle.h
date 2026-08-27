#pragma once

#include <QJsonObject>
#include <QString>
#include <QWebEngineScript>

class QWebEngineProfile;

namespace whatsie::web {

/// Installs the injected scripts on the profile-level collection (ADR-006).
/// Scripts are addressed by name; re-installing replaces. The bootstrap
/// bundle (config + qwebchannel.js + bootstrap.js + the DocumentCreation
/// feature scripts) is one script so its order is guaranteed.
class ScriptBundle
{
public:
    explicit ScriptBundle(QWebEngineProfile& profile);

    /// (Re)installs the DocumentCreation bundle with the given config
    /// (exposed to scripts as `window.__whatsie.config`).
    void installBootstrap(const QJsonObject& config);

    void installResource(const QString& name, const QString& resourcePath,
                         QWebEngineScript::InjectionPoint point = QWebEngineScript::DocumentReady);
    void remove(const QString& name);
    [[nodiscard]] bool isInstalled(const QString& name) const;

    [[nodiscard]] static QString readResource(const QString& resourcePath);
    [[nodiscard]] static QString scriptName(const QString& name);

    /// Names of the resources bundled into the bootstrap, in order.
    [[nodiscard]] static QStringList bootstrapResources();

private:
    void installSource(const QString& name, const QString& source, QWebEngineScript::InjectionPoint point);

    QWebEngineProfile& m_profile;
};

} // namespace whatsie::web
