#include "ui/permission_prompt.h"

#include "ui/logging.h"
#include "web/permission_controller.h"

#include <QMessageBox>
#include <QPushButton>

namespace whatsie::ui {

void askPermission(QWidget* parent, QWebEnginePermission permission)
{
    auto* box = new QMessageBox(parent);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setIcon(QMessageBox::Question);
    box->setWindowTitle(QObject::tr("Permission request"));
    box->setText(QObject::tr("WhatsApp wants to %1.")
                     .arg(web::PermissionController::describe(permission.permissionType())));
    box->setInformativeText(QObject::tr("You can change this later in Settings → Privacy."));
    QPushButton* allow = box->addButton(QObject::tr("Allow"), QMessageBox::AcceptRole);
    box->addButton(QObject::tr("Deny"), QMessageBox::RejectRole);
    box->setDefaultButton(allow);
    QObject::connect(box, &QMessageBox::finished, box, [box, allow, permission]() mutable {
        if (box->clickedButton() == allow) {
            qCInfo(lcUi) << "user allowed" << permission.permissionType();
            permission.grant();
        } else {
            qCInfo(lcUi) << "user denied" << permission.permissionType();
            permission.deny();
        }
    });
    box->open();
}

} // namespace whatsie::ui
