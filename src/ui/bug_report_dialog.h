#pragma once

#include <QDialog>

class QCheckBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;

namespace whatsie::core {
class Settings;
}

namespace whatsie::ui {

/// Collects a short problem description, previews the diagnostics that will be
/// attached, and opens a pre-filled GitHub issue while copying the full report
/// to the clipboard (the logs are too large for the URL itself).
class BugReportDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(BugReportDialog)

public:
    BugReportDialog(const core::Settings& settings, QString userAgent, QWidget* parent = nullptr);
    ~BugReportDialog() override = default;

private:
    void setupUi();
    void refreshDetails();
    void submit();
    void copyReport();

    const core::Settings& m_settings;
    QString m_userAgent;
    QPlainTextEdit* m_summary = nullptr;
    QCheckBox* m_includeCrash = nullptr;
    QPushButton* m_detailsToggle = nullptr;
    QPlainTextEdit* m_details = nullptr;
    QLabel* m_status = nullptr;
};

} // namespace whatsie::ui
