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

    [[nodiscard]] QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);

    [[nodiscard]] QByteArray windowState() const;
    void setWindowState(const QByteArray& state);

    [[nodiscard]] double zoomFactor() const;
    void setZoomFactor(double factor);

    [[nodiscard]] Theme theme() const;
    void setTheme(Theme theme);

    /// Flushes pending writes to disk. Normally automatic; explicit before quit.
    void sync();

    /// Location of the backing store (file path or registry key), for diagnostics.
    [[nodiscard]] QString fileName() const;

Q_SIGNALS:
    void zoomFactorChanged(double factor);
    void themeChanged(whatsie::core::Theme theme);

private:
    std::unique_ptr<QSettings> m_store;
};

} // namespace whatsie::core

Q_DECLARE_METATYPE(whatsie::core::Theme)
