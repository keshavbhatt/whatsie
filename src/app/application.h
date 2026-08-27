#pragma once

#include <QApplication>

#include <memory>

namespace whatsie::core {
class Settings;
}

namespace whatsie::app {

/// Application object. Sets identity (names/version), owns the Settings
/// instance and hands it out by reference — the only sanctioned "global".
class Application : public QApplication
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Application)

public:
    Application(int& argc, char** argv);
    ~Application() override;

    [[nodiscard]] core::Settings& settings();

private:
    std::unique_ptr<core::Settings> m_settings;
};

} // namespace whatsie::app
