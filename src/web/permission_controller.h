#pragma once

#include <QList>
#include <QObject>
#include <QUrl>
#include <QWebEnginePermission>

class QWebEnginePage;
class QWebEngineProfile;

namespace whatsie::web {

/// Decides web permission requests (FEATURES M1, N11).
///
/// Answers to a media `permissionRequested` are session-scoped in Qt WebEngine;
/// only `QWebEngineProfile::queryPermission(...).grant()` reaches the on-disk
/// store. This controller therefore (a) answers from the store when it can,
/// (b) persists the user's answer, and (c) treats camera / microphone /
/// camera+microphone as one decision — WhatsApp asks for video first and
/// audio+video a moment later, which must not become two prompts.
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
    /// Pure policy for a type with no stored answer, tested.
    [[nodiscard]] static Decision decide(QWebEnginePermission::PermissionType type);
    [[nodiscard]] static QString describe(QWebEnginePermission::PermissionType type);
    /// Types that share one decision with `type` (camera/mic family), tested.
    [[nodiscard]] static QList<QWebEnginePermission::PermissionType>
    relatedTypes(QWebEnginePermission::PermissionType type);

    /// The UI's answer to `promptRequested`: applies it to the request and
    /// stores it for `type` and its related types.
    void answer(const QWebEnginePermission& permission, bool allow);

    /// Stored decisions for `origin`, family-aware (pure over the store).
    [[nodiscard]] static QWebEnginePermission::State
    storedState(QWebEngineProfile& profile, const QUrl& origin, QWebEnginePermission::PermissionType type);

Q_SIGNALS:
    /// The UI must call answer(permission, allow).
    void promptRequested(QWebEnginePermission permission);

private:
    void handle(QWebEnginePermission permission);
    static void store(QWebEngineProfile& profile, const QUrl& origin,
                      QWebEnginePermission::PermissionType type, bool allow);

    QWebEngineProfile* m_profile = nullptr;
};

} // namespace whatsie::web
