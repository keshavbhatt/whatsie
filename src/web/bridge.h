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

Q_SIGNALS:
    void scriptFailure(const QString& name, const QString& message);
};

} // namespace whatsie::web
