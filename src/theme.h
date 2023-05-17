#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QDebug>
#include <QPalette>

class Theme {
public:
  static QPalette &getLightPalette();
  static QPalette &getDarkPalette();

private:
  static QPalette &lightPalette() {
    static QPalette palette = initLightPalette();
    return palette;
  }
  static QPalette &darkPalette() {
    static QPalette palette = initDarkPalette();
    return palette;
  }

  static QPalette initLightPalette();
  static QPalette initDarkPalette();
};

#endif // THEME_H
