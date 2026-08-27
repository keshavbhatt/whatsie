#include "web/permission_controller.h"

#include "web/logging.h"

#include <QCoreApplication>
#include <QWebEnginePage>
#include <QWebEngineProfile>

namespace whatsie::web {

using PermissionType = QWebEnginePermission::PermissionType;
using State = QWebEnginePermission::State;

PermissionController::PermissionController(QObject* parent)
    : QObject(parent)
{}

void PermissionController::attach(QWebEnginePage& page)
{
    m_profile = page.profile();
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

QList<PermissionType> PermissionController::relatedTypes(PermissionType type)
{
    switch (type) {
    case PermissionType::MediaAudioCapture:
    case PermissionType::MediaVideoCapture:
    case PermissionType::MediaAudioVideoCapture:
        return {PermissionType::MediaAudioCapture, PermissionType::MediaVideoCapture,
                PermissionType::MediaAudioVideoCapture};
    default:
        return {type};
    }
}

namespace {
bool isMediaType(PermissionType type)
{
    return type == PermissionType::MediaAudioCapture || type == PermissionType::MediaVideoCapture ||
           type == PermissionType::MediaAudioVideoCapture;
}
} // namespace

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

State PermissionController::storedState(QWebEngineProfile& profile, const QUrl& origin, PermissionType type)
{
    // A decision on any member of the family counts for the whole family;
    // a denial anywhere wins over a grant elsewhere.
    State result = State::Ask;
    for (const PermissionType related : relatedTypes(type)) {
        const State s = profile.queryPermission(origin, related).state();
        if (s == State::Denied) {
            return State::Denied;
        }
        if (s == State::Granted) {
            result = State::Granted;
        }
    }
    return result;
}

void PermissionController::store(QWebEngineProfile& profile, const QUrl& origin, PermissionType type,
                                 bool allow)
{
    for (const PermissionType related : relatedTypes(type)) {
        QWebEnginePermission stored = profile.queryPermission(origin, related);
        if (allow) {
            stored.grant();
        } else {
            stored.deny();
        }
    }
}

void PermissionController::answer(const QWebEnginePermission& permission, bool allow)
{
    QWebEnginePermission request = permission;
    if (allow) {
        request.grant();
    } else {
        request.deny();
    }
    if (m_profile != nullptr) {
        store(*m_profile, permission.origin(), permission.permissionType(), allow);
    }
    qCInfo(lcWeb) << (allow ? "user allowed" : "user denied") << permission.permissionType()
                  << permission.origin() << "(stored)";
}

void PermissionController::handle(QWebEnginePermission permission)
{
    const PermissionType type = permission.permissionType();
    switch (decide(type)) {
    case Decision::Grant:
        qCInfo(lcWeb) << "permission granted by policy:" << type << permission.origin();
        permission.grant();
        return;
    case Decision::Deny:
        qCInfo(lcWeb) << "permission denied by policy:" << type << permission.origin();
        permission.deny();
        return;
    case Decision::Ask:
        break;
    }

    if (m_profile != nullptr) {
        switch (storedState(*m_profile, permission.origin(), type)) {
        case State::Granted:
            qCInfo(lcWeb) << "permission granted from store:" << type << permission.origin();
            permission.grant();
            store(*m_profile, permission.origin(), type, true); // fill in the family
            return;
        case State::Denied:
            qCInfo(lcWeb) << "permission denied from store:" << type << permission.origin();
            permission.deny();
            return;
        case State::Ask:
        case State::Invalid:
            break;
        }
    }

    // Camera and microphone are core to a WhatsApp client and asking again in a
    // call pop-out window (a fresh page) is jarring, so grant on first request
    // and remember it — the user can still revoke via Settings -> Privacy, which
    // is stored and honoured above. Geolocation still asks.
    if (isMediaType(type)) {
        qCInfo(lcWeb) << "media permission auto-granted:" << type << permission.origin();
        permission.grant();
        if (m_profile != nullptr) {
            store(*m_profile, permission.origin(), type, true);
        }
        return;
    }
    qCInfo(lcWeb) << "permission prompt:" << type << permission.origin();
    Q_EMIT promptRequested(permission);
}

} // namespace whatsie::web
