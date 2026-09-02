#include "ui/downloads_hub.h"

#include "core/downloads/download_model.h"
#include "core/notifications/notification_service.h"
#include "core/settings/settings.h"
#include "platform/file_manager.h"
#include "ui/downloads_dialog.h"
#include "ui/logging.h"
#include "web/download_controller.h"
#include "web/web_view.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

using namespace Qt::StringLiterals;
using whatsie::core::DownloadState;

namespace whatsie::ui {

DownloadsHub::DownloadsHub(core::Settings& settings, core::NotificationService& notifications,
                           web::WebView& view, QWidget* dialogParent, QObject* parent)
    : QObject(parent)
    , m_settings(settings)
    , m_notifications(notifications)
    , m_dialogParent(dialogParent)
    , m_model(std::make_unique<core::DownloadModel>())
    , m_controller(
          std::make_unique<web::DownloadController>(view.profile(), settings, *m_model, dialogParent))
{
    connect(m_controller.get(), &web::DownloadController::downloadStarted, this, [this](quint64) {
        if (m_settings.showDownloadsOnStart()) {
            showWindow();
        }
    });
    connect(m_model.get(), &core::DownloadModel::entryFinished, this,
            [this](quint64 id, DownloadState state) {
                if (state == DownloadState::Completed || state == DownloadState::Failed) {
                    notifyFinished(id);
                }
            });
    connect(&m_notifications, &core::NotificationService::activated, this, [this](quint64 notificationId) {
        const auto it = m_notificationToDownload.constFind(notificationId);
        if (it != m_notificationToDownload.constEnd()) {
            perform(it.value(), DownloadsDelegate::Action::Open);
        }
    });
    connect(&m_notifications, &core::NotificationService::closed, this,
            [this](quint64 notificationId) { m_notificationToDownload.remove(notificationId); });
}

DownloadsHub::~DownloadsHub() = default;

void DownloadsHub::showWindow()
{
    if (!m_dialog) {
        m_dialog = new DownloadsDialog(*m_model, m_dialogParent);
        m_dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_dialog, &DownloadsDialog::actionRequested, this, &DownloadsHub::perform);
        connect(m_dialog, &DownloadsDialog::openFolderRequested, this,
                [this] { platform::openDirectory(m_settings.downloadDirectory()); });
        connect(m_dialog, &DownloadsDialog::clearFinishedRequested, m_model.get(),
                &core::DownloadModel::clearFinished);
    }
    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void DownloadsHub::hideWindow()
{
    if (m_dialog) {
        m_dialog->close();
    }
}

void DownloadsHub::perform(quint64 id, DownloadsDelegate::Action action)
{
    const auto entry = m_model->entry(id);
    if (!entry) {
        return;
    }
    switch (action) {
    case DownloadsDelegate::Action::Open:
        if (!platform::openFile(entry->filePath())) {
            qCWarning(lcUi) << "cannot open" << entry->filePath();
        }
        break;
    case DownloadsDelegate::Action::Reveal:
        platform::revealInFileManager(entry->filePath());
        break;
    case DownloadsDelegate::Action::Cancel:
        m_controller->cancel(id);
        break;
    case DownloadsDelegate::Action::Remove:
        m_model->remove(id);
        break;
    }
}

void DownloadsHub::notifyFinished(quint64 id)
{
    const auto entry = m_model->entry(id);
    if (!entry) {
        return;
    }
    core::Notification n;
    n.category = u"transfer.complete"_s;
    n.sender = u"Whatsie"_s;
    if (entry->state == DownloadState::Completed) {
        n.title = tr("Download complete");
        n.body = entry->fileName;
        n.image = QImage(u":/icons/hicolor/128x128/apps/com.ktechpit.whatsie.png"_s);
    } else {
        n.title = tr("Download failed");
        n.body = entry->error.isEmpty() ? entry->fileName : u"%1 — %2"_s.arg(entry->fileName, entry->error);
        n.category = u"transfer.error"_s;
    }
    n.sound = false;
    const quint64 notificationId = m_notifications.notify(n);
    if (notificationId != 0 && entry->state == DownloadState::Completed) {
        m_notificationToDownload.insert(notificationId, id);
    }
}

} // namespace whatsie::ui
