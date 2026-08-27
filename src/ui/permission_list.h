#pragma once

#include <QUrl>
#include <QWidget>

class QWebEngineProfile;

namespace whatsie::ui {

/// Per-permission Allow toggles for one origin (FEATURES M1), bound to the
/// real on-disk permission store (QWebEngineProfile::queryPermission). Replaces
/// whatsie's inert checkbox table — every toggle actually grants/denies.
class PermissionList : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(PermissionList)

public:
    PermissionList(QWebEngineProfile& profile, QUrl origin, QWidget* parent = nullptr);
    ~PermissionList() override = default;

    /// Re-reads the store (after an external change).
    void reload();

private:
    QWebEngineProfile& m_profile;
    QUrl m_origin;
};

} // namespace whatsie::ui
