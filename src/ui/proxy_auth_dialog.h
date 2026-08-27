#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;

namespace whatsie::ui {

/// Prompts for proxy credentials when the proxy challenges and none are cached
/// (FEATURES M12b). The password is returned to the caller and kept only for
/// the session (ADR-030) — this dialog never persists it.
class ProxyAuthDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ProxyAuthDialog)

public:
    ProxyAuthDialog(const QString& proxyHost, const QString& knownUser, QWidget* parent = nullptr);
    ~ProxyAuthDialog() override = default;

    [[nodiscard]] QString user() const;
    [[nodiscard]] QString password() const;

private:
    QLineEdit* m_user = nullptr;
    QLineEdit* m_password = nullptr;
};

} // namespace whatsie::ui
