#include "core/theme/theme_service.h"

#include "core/logging.h"
#include "core/settings/settings.h"

#include <QGuiApplication>
#include <QStyleHints>

namespace whatsie::core {

namespace {

Qt::ColorScheme platformScheme()
{
    if (QGuiApplication::instance() == nullptr) {
        return Qt::ColorScheme::Light;
    }
    const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
    return scheme == Qt::ColorScheme::Unknown ? Qt::ColorScheme::Light : scheme;
}

Qt::ColorScheme resolve(Theme theme)
{
    switch (theme) {
    case Theme::Light:
        return Qt::ColorScheme::Light;
    case Theme::Dark:
        return Qt::ColorScheme::Dark;
    case Theme::System:
        break;
    }
    return platformScheme();
}

} // namespace

ThemeService::ThemeService(Settings& settings, QObject* parent)
    : QObject(parent)
    , m_settings(settings)
    , m_current(resolve(settings.theme()))
{
    connect(&m_settings, &Settings::themeChanged, this, [this](Theme) { reevaluate(); });
    if (QGuiApplication::instance() != nullptr) {
        connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this,
                [this](Qt::ColorScheme) {
                    if (followsSystem()) {
                        reevaluate();
                    }
                });
    }
}

Qt::ColorScheme ThemeService::effectiveScheme() const
{
    return m_current;
}

bool ThemeService::followsSystem() const
{
    return m_settings.theme() == Theme::System;
}

void ThemeService::reevaluate()
{
    const Qt::ColorScheme next = resolve(m_settings.theme());
    if (next == m_current) {
        return;
    }
    m_current = next;
    qCInfo(lcCore) << "effective colour scheme:" << (isDark() ? "dark" : "light");
    Q_EMIT effectiveSchemeChanged(next);
}

QPalette ThemeService::lightPalette()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(0xF0, 0xF2, 0xF5));
    p.setColor(QPalette::WindowText, QColor(0x11, 0x1B, 0x21));
    p.setColor(QPalette::Base, QColor(0xFF, 0xFF, 0xFF));
    p.setColor(QPalette::AlternateBase, QColor(0xF0, 0xF2, 0xF5));
    p.setColor(QPalette::ToolTipBase, QColor(0xFF, 0xFF, 0xFF));
    p.setColor(QPalette::ToolTipText, QColor(0x11, 0x1B, 0x21));
    p.setColor(QPalette::Text, QColor(0x11, 0x1B, 0x21));
    p.setColor(QPalette::Button, QColor(0xF0, 0xF2, 0xF5));
    p.setColor(QPalette::ButtonText, QColor(0x11, 0x1B, 0x21));
    p.setColor(QPalette::BrightText, Qt::white);
    p.setColor(QPalette::Link, QColor(0x02, 0x7E, 0xB5));
    p.setColor(QPalette::Highlight, QColor(0x00, 0xA8, 0x84));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::PlaceholderText, QColor(0x66, 0x7B, 0x81));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(0x8A, 0x9A, 0xA0));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x8A, 0x9A, 0xA0));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(0x8A, 0x9A, 0xA0));
    return p;
}

QPalette ThemeService::darkPalette()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(0x11, 0x1B, 0x21));
    p.setColor(QPalette::WindowText, QColor(0xE9, 0xED, 0xEF));
    p.setColor(QPalette::Base, QColor(0x20, 0x2C, 0x33));
    p.setColor(QPalette::AlternateBase, QColor(0x18, 0x24, 0x29));
    p.setColor(QPalette::ToolTipBase, QColor(0x20, 0x2C, 0x33));
    p.setColor(QPalette::ToolTipText, QColor(0xE9, 0xED, 0xEF));
    p.setColor(QPalette::Text, QColor(0xE9, 0xED, 0xEF));
    p.setColor(QPalette::Button, QColor(0x20, 0x2C, 0x33));
    p.setColor(QPalette::ButtonText, QColor(0xE9, 0xED, 0xEF));
    p.setColor(QPalette::BrightText, Qt::white);
    p.setColor(QPalette::Link, QColor(0x53, 0xBD, 0xEB));
    p.setColor(QPalette::Highlight, QColor(0x00, 0xA8, 0x84));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::PlaceholderText, QColor(0x8A, 0x9A, 0xA0));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(0x66, 0x7B, 0x81));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x66, 0x7B, 0x81));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(0x66, 0x7B, 0x81));
    return p;
}

} // namespace whatsie::core
