#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>

class SettingsManager {
public:
  static SettingsManager &instance() {
    static SettingsManager instance;
    return instance;
  }

  QSettings &settings() { return m_settings; }

private:
  SettingsManager() {}

  QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
