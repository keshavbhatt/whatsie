#pragma once

#include <QDialog>

namespace whatsie::core {
class Settings;
}

namespace whatsie::ui {

/// About box with version info, links and "Copy diagnostics" (FEATURES D1, D4).
class AboutDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(AboutDialog)

public:
    AboutDialog(const core::Settings& settings, const QString& userAgent, QWidget* parent = nullptr);
    ~AboutDialog() override = default;

private:
    void setupUi();
    void copyDiagnostics();

    const core::Settings& m_settings;
    QString m_userAgent;
};

} // namespace whatsie::ui
