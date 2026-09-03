#pragma once

#include "ui/downloads_delegate.h"

#include <QHash>
#include <QObject>
#include <QPointer>

#include <memory>

namespace whatsie::core {
class DownloadModel;
class NotificationService;
class Settings;
} // namespace whatsie::core
namespace whatsie::web {
class DownloadController;
class WebView;
} // namespace whatsie::web

namespace whatsie::ui {

class DownloadsDialog;

/// Owns the download model/controller/window and turns finished downloads
/// into notifications whose click opens the file (FEATURES M8).
class DownloadsHub : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadsHub)

public:
    DownloadsHub(core::Settings& settings, core::NotificationService& notifications, web::WebView& view,
                 QWidget* dialogParent, QObject* parent = nullptr);
    ~DownloadsHub() override;

    void showWindow();
    /// Close the downloads window if open (used when locking, FEATURES P1).
    void hideWindow();
    /// While suppressed the window never opens (and any open one is closed), so a
    /// download starting while the app is locked cannot expose filenames over the
    /// lock screen (FEATURES P1).
    void setSuppressed(bool suppressed);
    [[nodiscard]] core::DownloadModel& model() { return *m_model; }

private:
    void perform(quint64 id, DownloadsDelegate::Action action);
    void notifyFinished(quint64 id);

    core::Settings& m_settings;
    core::NotificationService& m_notifications;
    QWidget* m_dialogParent;
    std::unique_ptr<core::DownloadModel> m_model;
    std::unique_ptr<web::DownloadController> m_controller;
    QPointer<DownloadsDialog> m_dialog;
    QHash<quint64, quint64> m_notificationToDownload;
    bool m_suppressed = false;
};

} // namespace whatsie::ui
