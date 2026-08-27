#pragma once

#include "ui/downloads_delegate.h"

#include <QDialog>

class QLabel;
class QListView;
class QPushButton;
class QStackedWidget;

namespace whatsie::core {
class DownloadModel;
}

namespace whatsie::ui {

/// The downloads window (FEATURES M8): persistent list with progress, hover
/// actions, context menu, "Open folder" and "Clear finished".
class DownloadsDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadsDialog)

public:
    explicit DownloadsDialog(core::DownloadModel& model, QWidget* parent = nullptr);
    ~DownloadsDialog() override = default;

Q_SIGNALS:
    void actionRequested(quint64 id, whatsie::ui::DownloadsDelegate::Action action);
    void openFolderRequested();
    void clearFinishedRequested();

private:
    void setupUi();
    void updateSummary();
    void showContextMenu(const QPoint& pos);

    core::DownloadModel& m_model;
    QStackedWidget* m_stack = nullptr;
    QListView* m_list = nullptr;
    QLabel* m_summary = nullptr;
    QPushButton* m_clear = nullptr;
    DownloadsDelegate* m_delegate = nullptr;
};

} // namespace whatsie::ui
