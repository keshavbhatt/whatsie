#pragma once

#include <QJsonObject>
#include <QObject>

#include <memory>

class QLocalServer;

namespace whatsie::app {

/// Single-instance guard and command channel (ADR-010, FEATURES S8).
/// The first process to listen on the named local socket is the primary;
/// later processes detect it, forward JSON commands and exit.
/// The key must include the profile name so different profiles can coexist.
class SingleInstance : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(SingleInstance)

public:
    explicit SingleInstance(const QString& key, QObject* parent = nullptr);
    ~SingleInstance() override;

    [[nodiscard]] bool isPrimary() const { return m_primary; }
    [[nodiscard]] QString key() const { return m_key; }

    /// Secondary only. Sends one command and waits briefly for delivery.
    bool sendToPrimary(const QJsonObject& command, int timeoutMs = 2000);

Q_SIGNALS:
    /// Primary only. Emitted for every command a secondary instance sent.
    void commandReceived(const QJsonObject& command);

private:
    [[nodiscard]] static bool primaryIsAlive(const QString& key, int timeoutMs);
    void startServer();
    void onNewConnection();

    QString m_key;
    bool m_primary = false;
    std::unique_ptr<QLocalServer> m_server;
};

/// Socket name for a profile: "whatsie-<profile>-<uid>" (uid keeps users apart on
/// shared /tmp; profile empty → "default").
[[nodiscard]] QString instanceKeyFor(const QString& profile);

} // namespace whatsie::app
