#include "web/permission_controller.h"

#include "web/logging.h"

#include <QCoreApplication>
#include <QWebEnginePage>

namespace whatsie::web {

using PermissionType = QWebEnginePermission::PermissionType;

PermissionController::PermissionController(QObject* parent)
    : QObject(parent)
{}

void PermissionController::attach(QWebEnginePage& page)
{
    connect(&page, &QWebEnginePage::permissionRequested, this, &PermissionController::handle);
}

PermissionController::Decision PermissionController::decide(PermissionType type)
{
    switch (type) {
    case PermissionType::Notifications:       // the point of the app (W#307)
    case PermissionType::DesktopVideoCapture: // the picker dialog is the consent
    case PermissionType::DesktopAudioVideoCapture:
    case PermissionType::MouseLock:
    case PermissionType::ClipboardReadWrite:
        return Decision::Grant;
    case PermissionType::MediaAudioCapture:
    case PermissionType::MediaVideoCapture:
    case PermissionType::MediaAudioVideoCapture:
    case PermissionType::Geolocation:
        return Decision::Ask;
    case PermissionType::LocalFontsAccess:
    case PermissionType::Unsupported:
        return Decision::Deny;
    }
    return Decision::Deny;
}

QString PermissionController::describe(PermissionType type)
{
    switch (type) {
    case PermissionType::MediaAudioCapture:
        return QCoreApplication::translate("Permission", "use your microphone");
    case PermissionType::MediaVideoCapture:
        return QCoreApplication::translate("Permission", "use your camera");
    case PermissionType::MediaAudioVideoCapture:
        return QCoreApplication::translate("Permission", "use your camera and microphone");
    case PermissionType::Geolocation:
        return QCoreApplication::translate("Permission", "know your location");
    case PermissionType::Notifications:
        return QCoreApplication::translate("Permission", "show notifications");
    case PermissionType::DesktopVideoCapture:
    case PermissionType::DesktopAudioVideoCapture:
        return QCoreApplication::translate("Permission", "share your screen");
    case PermissionType::MouseLock:
        return QCoreApplication::translate("Permission", "lock the mouse pointer");
    case PermissionType::ClipboardReadWrite:
        return QCoreApplication::translate("Permission", "read the clipboard");
    case PermissionType::LocalFontsAccess:
        return QCoreApplication::translate("Permission", "list your fonts");
    case PermissionType::Unsupported:
        break;
    }
    return QCoreApplication::translate("Permission", "do something unsupported");
}

void PermissionController::handle(QWebEnginePermission permission)
{
    const PermissionType type = permission.permissionType();
    switch (decide(type)) {
    case Decision::Grant:
        qCInfo(lcWeb) << "permission granted:" << type << permission.origin();
        permission.grant();
        return;
    case Decision::Deny:
        qCInfo(lcWeb) << "permission denied:" << type << permission.origin();
        permission.deny();
        return;
    case Decision::Ask:
        break;
    }
    // WebEngine only asks when nothing is persisted; hand it to the user.
    qCInfo(lcWeb) << "permission prompt:" << type << permission.origin();
    Q_EMIT promptRequested(permission);
}

} // namespace whatsie::web
