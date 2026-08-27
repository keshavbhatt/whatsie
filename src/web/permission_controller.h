#pragma once

#include <QObject>
#include <QWebEnginePermission>

class QWebEnginePage;

namespace whatsie::web {

/// Decides web permission requests (FEATURES M1, N11). Decisions that need
/// the user are handed to the UI through `promptRequested`; WebEngine
/// persists granted/denied states on disk (profile policy StoreOnDisk).
class PermissionController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(PermissionController)

public:
    explicit PermissionController(QObject* parent = nullptr);
    ~PermissionController() override = default;

    void attach(QWebEnginePage& page);

    enum class Decision
    {
        Grant,
        Deny,
        Ask,
    };
    /// Pure policy, tested.
    [[nodiscard]] static Decision decide(QWebEnginePermission::PermissionType type);
    [[nodiscard]] static QString describe(QWebEnginePermission::PermissionType type);

Q_SIGNALS:
    /// The UI must call grant() or deny() on the permission.
    void promptRequested(QWebEnginePermission permission);

private:
    void handle(QWebEnginePermission permission);
};

} // namespace whatsie::web
