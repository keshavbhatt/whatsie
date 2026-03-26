#pragma once

#include <QWebEngineProfile>
#include <QWebEngineSettings>

// Singleton that owns the single persistent QWebEngineProfile for the app.
// Call instance() before creating any QWebEnginePage.
class WebEngineProfileManager {
public:
    static WebEngineProfileManager &instance();

    QWebEngineProfile *profile() const;

    // Re-reads user-configurable settings (UA, spell-check, autoplay) from
    // QSettings and applies them to the profile. Call whenever these settings
    // change without recreating the page.
    void applyUserSettings();

private:
    WebEngineProfileManager();
    ~WebEngineProfileManager();

    QWebEngineProfile *m_profile;
};
