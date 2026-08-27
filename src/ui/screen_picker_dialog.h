#pragma once

#include <QDialog>
#include <QWebEngineDesktopMediaRequest>

class QListView;
class QTabWidget;

namespace whatsie::ui {

/// "Share your screen" picker (FEATURES M2): screens and windows offered by
/// Chromium. Accept selects, reject cancels the request.
class ScreenPickerDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ScreenPickerDialog)

public:
    ScreenPickerDialog(QWebEngineDesktopMediaRequest request, QWidget* parent = nullptr);
    ~ScreenPickerDialog() override = default;

    void accept() override;
    void reject() override;

private:
    QWebEngineDesktopMediaRequest m_request;
    QTabWidget* m_tabs = nullptr;
    QListView* m_screens = nullptr;
    QListView* m_windows = nullptr;
    bool m_answered = false;
};

} // namespace whatsie::ui
