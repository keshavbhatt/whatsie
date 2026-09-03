#include "app/single_instance.h"

#include "app/logging.h"

#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

using namespace Qt::StringLiterals;

namespace whatsie::app {

namespace {
// Messages are newline-delimited compact JSON objects.
constexpr char kDelimiter = '\n';
} // namespace

QString instanceKeyFor(const QString& profile)
{
    const QString name = profile.trimmed().isEmpty() ? u"default"_s : profile.trimmed();
    QString suffix;
#ifdef Q_OS_UNIX
    suffix = QString::number(static_cast<qulonglong>(getuid()));
#else
    suffix = qEnvironmentVariable("USERNAME");
#endif
    return u"whatsie-%1-%2"_s.arg(name, suffix);
}

SingleInstance::SingleInstance(const QString& key, QObject* parent)
    : QObject(parent)
    , m_key(key) // NOLINT(modernize-pass-by-value) — small, and callers keep their copy
{
    if (primaryIsAlive(m_key, 500)) {
        m_primary = false;
        qCInfo(lcApp) << "another instance owns" << m_key << "- running as secondary";
        return;
    }
    startServer();
}

SingleInstance::~SingleInstance()
{
    if (m_server) {
        m_server->close();
        QLocalServer::removeServer(m_key);
    }
}

void SingleInstance::release()
{
    if (m_server) {
        m_server->close();
        QLocalServer::removeServer(m_key);
        m_server.reset();
    }
    m_primary = false;
}

bool SingleInstance::primaryIsAlive(const QString& key, int timeoutMs)
{
    QLocalSocket probe;
    probe.connectToServer(key);
    const bool alive = probe.waitForConnected(timeoutMs);
    if (alive) {
        probe.disconnectFromServer();
    }
    return alive;
}

void SingleInstance::startServer()
{
    // A stale socket file from a crashed primary would make listen() fail.
    QLocalServer::removeServer(m_key);
    m_server = std::make_unique<QLocalServer>();
    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    connect(m_server.get(), &QLocalServer::newConnection, this, &SingleInstance::onNewConnection);
    if (!m_server->listen(m_key)) {
        qCWarning(lcApp) << "cannot listen on" << m_key << m_server->errorString()
                         << "- continuing without single-instance guard";
        m_server.reset();
        m_primary = true;
        return;
    }
    m_primary = true;
    qCDebug(lcApp) << "primary instance listening on" << m_key;
}

void SingleInstance::onNewConnection()
{
    while (QLocalSocket* socket = m_server->nextPendingConnection()) {
        connect(socket, &QLocalSocket::readyRead, this, [this, socket] {
            while (socket->canReadLine()) {
                const QByteArray line = socket->readLine().trimmed();
                if (line.isEmpty()) {
                    continue;
                }
                QJsonParseError error{};
                const QJsonDocument doc = QJsonDocument::fromJson(line, &error);
                if (error.error != QJsonParseError::NoError || !doc.isObject()) {
                    qCWarning(lcApp) << "ignoring malformed IPC message:" << error.errorString();
                    continue;
                }
                Q_EMIT commandReceived(doc.object());
            }
        });
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
    }
}

bool SingleInstance::sendToPrimary(const QJsonObject& command, int timeoutMs)
{
    if (m_primary) {
        return false;
    }
    QLocalSocket socket;
    socket.connectToServer(m_key);
    if (!socket.waitForConnected(timeoutMs)) {
        qCWarning(lcApp) << "cannot reach primary instance:" << socket.errorString();
        return false;
    }
    QByteArray payload = QJsonDocument(command).toJson(QJsonDocument::Compact);
    payload.append(kDelimiter);
    socket.write(payload);
    const bool written = socket.waitForBytesWritten(timeoutMs);
    socket.disconnectFromServer();
    if (socket.state() != QLocalSocket::UnconnectedState) {
        socket.waitForDisconnected(timeoutMs);
    }
    return written;
}

} // namespace whatsie::app
