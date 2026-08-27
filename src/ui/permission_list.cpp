#include "ui/permission_list.h"

#include "ui/logging.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QWebEnginePermission>
#include <QWebEngineProfile>

using namespace Qt::StringLiterals;
using PermissionType = QWebEnginePermission::PermissionType;

namespace whatsie::ui {

namespace {
struct Row
{
    PermissionType type;
    const char* label;
};
bool defaultOn(PermissionType type)
{
    // Camera/mic are allowed by default for a WhatsApp client (PermissionController
    // grants them on first use); Location is off until granted.
    return type == PermissionType::MediaVideoCapture || type == PermissionType::MediaAudioCapture;
}

bool checkedFor(QWebEngineProfile& profile, const QUrl& origin, PermissionType type)
{
    const auto state = profile.queryPermission(origin, type).state();
    if (state == QWebEnginePermission::State::Granted) {
        return true;
    }
    if (state == QWebEnginePermission::State::Denied) {
        return false;
    }
    return defaultOn(type); // Ask/Invalid
}

const Row kRows[] = {
    {.type = PermissionType::MediaVideoCapture, .label = QT_TRANSLATE_NOOP("PermissionList", "Camera")},
    {.type = PermissionType::MediaAudioCapture, .label = QT_TRANSLATE_NOOP("PermissionList", "Microphone")},
    {.type = PermissionType::Geolocation, .label = QT_TRANSLATE_NOOP("PermissionList", "Location")},
};
} // namespace

PermissionList::PermissionList(QWebEngineProfile& profile, QUrl origin, QWidget* parent)
    : QWidget(parent)
    , m_profile(profile)
    , m_origin(std::move(origin))
{
    auto* form = new QFormLayout(this);
    form->setContentsMargins(0, 0, 0, 0);
    for (const Row& row : kRows) {
        auto* check = new QCheckBox(tr(row.label), this);
        check->setTristate(false);
        check->setChecked(checkedFor(m_profile, m_origin, row.type));
        const PermissionType type = row.type;
        connect(check, &QCheckBox::toggled, this, [this, type](bool allow) {
            QWebEnginePermission permission = m_profile.queryPermission(m_origin, type);
            if (allow) {
                permission.grant();
            } else {
                permission.deny();
            }
            qCInfo(lcUi) << "permission" << type << (allow ? "granted" : "denied") << "via settings";
        });
        form->addRow(check);
    }
}

void PermissionList::reload()
{
    const auto boxes = findChildren<QCheckBox*>();
    int i = 0;
    for (QCheckBox* box : boxes) {
        if (i >= static_cast<int>(std::size(kRows))) {
            break;
        }
        const QSignalBlocker blocker(box);
        box->setChecked(checkedFor(m_profile, m_origin, kRows[i].type));
        ++i;
    }
}

} // namespace whatsie::ui
