#include "theme.h"

QPalette Theme::initLightPalette() {
  QPalette lightPalette;
  lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
  lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
  lightPalette.setColor(QPalette::Light, QColor(180, 180, 180));
  lightPalette.setColor(QPalette::Midlight, QColor(200, 200, 200));
  lightPalette.setColor(QPalette::Dark, QColor(225, 225, 225));
  lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::BrightText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::Base, QColor(237, 237, 237));
  lightPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
  lightPalette.setColor(QPalette::Highlight, QColor(76, 163, 224));
  lightPalette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::Link, QColor(0, 162, 232));
  lightPalette.setColor(QPalette::AlternateBase, QColor(225, 225, 225));
  lightPalette.setColor(QPalette::ToolTipBase, QColor(240, 240, 240));
  lightPalette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::LinkVisited, QColor(222, 222, 222));
  lightPalette.setColor(QPalette::Disabled, QPalette::WindowText,
                        QColor(115, 115, 115));
  lightPalette.setColor(QPalette::Disabled, QPalette::Text,
                        QColor(115, 115, 115));
  lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                        QColor(115, 115, 115));
  lightPalette.setColor(QPalette::Disabled, QPalette::Highlight,
                        QColor(190, 190, 190));
  lightPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                        QColor(115, 115, 115));

  return lightPalette;
}

QPalette Theme::initDarkPalette() {
  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(17, 27, 33));
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(32, 44, 51));
  darkPalette.setColor(QPalette::AlternateBase, QColor(95, 108, 115));
  darkPalette.setColor(QPalette::ToolTipBase, QColor(66, 66, 66));
  darkPalette.setColor(QPalette::ToolTipText, QColor(192, 192, 192));
  darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
  darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
  darkPalette.setColor(QPalette::Button, QColor(17, 27, 33));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::Highlight, QColor(38, 140, 196));
  darkPalette.setColor(QPalette::HighlightedText, Qt::white);
  darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                       QColor(127, 127, 127));
  darkPalette.setColor(QPalette::Disabled, QPalette::Window,
                       QColor(65, 65, 67));
  darkPalette.setColor(QPalette::Disabled, QPalette::Highlight,
                       QColor(80, 80, 80));
  darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                       QColor(127, 127, 127));
  darkPalette.setColor(QPalette::Disabled, QPalette::Text,
                       QColor(127, 127, 127));
  return darkPalette;
}

QPalette &Theme::getLightPalette() { return lightPalette(); }

QPalette &Theme::getDarkPalette() { return darkPalette(); }
