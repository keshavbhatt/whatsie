#pragma once

#include <QByteArray>
#include <QObject>
#include <QSettings>

#include <memory>

namespace whatsie::core {

enum class Theme
{
    System,
    Light,
    Dark,
};

enum class CloseAction
{
    MinimizeToTray,
    Quit,
};

/// Typed facade over QSettings. The only place in the code base allowed to
/// construct a QSettings. Defaults live in one table (settings.cpp), every
/// setter emits a change signal so the rest of the app reacts instead of
/// polling.
class Settings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Settings)

public:
    /// Uses the platform default location (org/app name from the application).
    explicit Settings(QObject* parent = nullptr);
    /// Uses an explicit INI file — for tests and portable mode.
    Settings(const QString& iniFilePath, QObject* parent = nullptr);
    ~Settings() override;

    // window/
    [[nodiscard]] QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);
    [[nodiscard]] QByteArray windowState() const;
    void setWindowState(const QByteArray& state);
    [[nodiscard]] CloseAction closeAction() const;
    void setCloseAction(CloseAction action);
    [[nodiscard]] bool startMinimized() const;
    void setStartMinimized(bool enabled);

    // tray/
    [[nodiscard]] bool trayLeftClickToggles() const;
    void setTrayLeftClickToggles(bool enabled);

    // view/
    [[nodiscard]] double zoomFactor() const;
    void setZoomFactor(double factor);
    [[nodiscard]] double zoomFactorMaximized() const;
    void setZoomFactorMaximized(double factor);
    [[nodiscard]] bool smoothScrolling() const;
    void setSmoothScrolling(bool enabled);

    // appearance/
    [[nodiscard]] Theme theme() const;
    void setTheme(Theme theme);

    /// Flushes pending writes to disk. Normally automatic; explicit before quit.
    void sync();

    /// Location of the backing store (file path or registry key), for diagnostics.
    [[nodiscard]] QString fileName() const;

Q_SIGNALS:
    void closeActionChanged(whatsie::core::CloseAction action);
    void startMinimizedChanged(bool enabled);
    void trayLeftClickTogglesChanged(bool enabled);
    void zoomFactorChanged(double factor);
    void zoomFactorMaximizedChanged(double factor);
    void smoothScrollingChanged(bool enabled);
    void themeChanged(whatsie::core::Theme theme);

private:
    [[nodiscard]] bool boolValue(QLatin1StringView key, bool def) const;
    /// Writes and returns true when the stored value actually changed.
    bool storeBool(QLatin1StringView key, bool def, bool value);

    std::unique_ptr<QSettings> m_store;
};

} // namespace whatsie::core

Q_DECLARE_METATYPE(whatsie::core::Theme)
Q_DECLARE_METATYPE(whatsie::core::CloseAction)
