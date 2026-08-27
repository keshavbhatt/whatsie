#include "ui/permission_prompt.h"

#include "web/permission_controller.h"

#include <QMessageBox>
#include <QPushButton>

namespace whatsie::ui {

void askPermission(QWidget* parent, const QWebEnginePermission& permission, std::function<void(bool)> answer)
{
    auto* box = new QMessageBox(parent);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setIcon(QMessageBox::Question);
    box->setWindowTitle(QObject::tr("Permission request"));
    box->setText(QObject::tr("WhatsApp wants to %1.")
                     .arg(web::PermissionController::describe(permission.permissionType())));
    box->setInformativeText(
        QObject::tr("Your choice is remembered. Change it in Settings → Privacy & Advanced."));
    QPushButton* allow = box->addButton(QObject::tr("Allow"), QMessageBox::AcceptRole);
    box->addButton(QObject::tr("Deny"), QMessageBox::RejectRole);
    box->setDefaultButton(allow);
    QObject::connect(box, &QMessageBox::finished, box,
                     [box, allow, answer = std::move(answer)] { answer(box->clickedButton() == allow); });
    box->open();
}

} // namespace whatsie::ui
