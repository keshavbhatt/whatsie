#pragma once

#include <QObject>

namespace whatsie::web {

/// The single QWebChannel object exposed to injected scripts as `bridge`
/// (ADR-006). Keep the surface tiny and documented in scripts/README.md.
class Bridge : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Bridge)

public:
    explicit Bridge(QObject* parent = nullptr);
    ~Bridge() override = default;

public Q_SLOTS:
    /// Called by scripts when their try/catch fires.
    void scriptFailed(const QString& name, const QString& message);
    void log(const QString& message);
    /// The WhatsApp WebSocket connection came up / went down (FEATURES S13).
    void connectionChanged(bool up);
    /// The injected nav-rail button was clicked (FEATURES A11).
    void openSettings();
    /// The custom error page's "Try again" was clicked — reload WhatsApp in-app.
    void retry();

Q_SIGNALS:
    void scriptFailure(const QString& name, const QString& message);
    void connectionStateChanged(bool up);
    void settingsRequested();
    void retryRequested();
};

} // namespace whatsie::web
