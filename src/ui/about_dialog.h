#pragma once

#include <QDialog>

class QPlainTextEdit;
class QPushButton;
class QWebEnginePage;

namespace whatsie::core {
class Settings;
}

namespace whatsie::ui {

/// About box: app info, links, and a copyable diagnostics panel (FEATURES D1, D4).
class AboutDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(AboutDialog)

public:
    AboutDialog(const core::Settings& settings, const QString& userAgent, QWebEnginePage* page,
                QWidget* parent = nullptr);
    ~AboutDialog() override = default;

protected:
    void showEvent(QShowEvent* event) override;

private:
    void setupUi();
    void toggleDebugInfo();
    void copyDiagnostics();
    void reportBug();

    const core::Settings& m_settings;
    QString m_userAgent;
    QWebEnginePage* m_page = nullptr;
    QWidget* m_content = nullptr;
    QPushButton* m_debugToggle = nullptr;
    QPlainTextEdit* m_debugText = nullptr;
};

} // namespace whatsie::ui
