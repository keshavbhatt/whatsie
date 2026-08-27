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
    // Sampled from WhatsApp Web's light theme (2026-08-28) so the native chrome
    // matches the page (ADR-031).
    QPalette p;
    p.setColor(QPalette::Window, QColor(0xDB, 0xD8, 0xD4));
    p.setColor(QPalette::WindowText, QColor(0x0A, 0x0A, 0x0A));
    p.setColor(QPalette::Base, QColor(0xFF, 0xFF, 0xFF));
    p.setColor(QPalette::AlternateBase, QColor(0xF7, 0xF5, 0xF3));
    p.setColor(QPalette::ToolTipBase, QColor(0xFF, 0xFF, 0xFF));
    p.setColor(QPalette::ToolTipText, QColor(0x0A, 0x0A, 0x0A));
    p.setColor(QPalette::Text, QColor(0x0A, 0x0A, 0x0A));
    p.setColor(QPalette::Button, QColor(0xFF, 0xFF, 0xFF));
    p.setColor(QPalette::ButtonText, QColor(0x0A, 0x0A, 0x0A));
    p.setColor(QPalette::BrightText, Qt::white);
    p.setColor(QPalette::Link, QColor(0x02, 0x7E, 0xB5));
    p.setColor(QPalette::Highlight, QColor(0x1D, 0xAA, 0x61));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::PlaceholderText, QColor(0x54, 0x65, 0x6F));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(0x8A, 0x9A, 0xA0));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x8A, 0x9A, 0xA0));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(0x8A, 0x9A, 0xA0));
    return p;
}

QPalette ThemeService::darkPalette()
{
    // Sampled from WhatsApp Web's dark theme (2026-08-28); ADR-031.
    QPalette p;
    p.setColor(QPalette::Window, QColor(0x26, 0x25, 0x24));
    p.setColor(QPalette::WindowText, QColor(0xED, 0xED, 0xED));
    p.setColor(QPalette::Base, QColor(0x1D, 0x1F, 0x1F));
    p.setColor(QPalette::AlternateBase, QColor(0x16, 0x17, 0x17));
    p.setColor(QPalette::ToolTipBase, QColor(0x2A, 0x2C, 0x2C));
    p.setColor(QPalette::ToolTipText, QColor(0xED, 0xED, 0xED));
    p.setColor(QPalette::Text, QColor(0xED, 0xED, 0xED));
    p.setColor(QPalette::Button, QColor(0x2A, 0x2C, 0x2C));
    p.setColor(QPalette::ButtonText, QColor(0xED, 0xED, 0xED));
    p.setColor(QPalette::BrightText, Qt::white);
    p.setColor(QPalette::Link, QColor(0x53, 0xBD, 0xEB));
    p.setColor(QPalette::Highlight, QColor(0x21, 0xC0, 0x63));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::PlaceholderText, QColor(0x8E, 0x96, 0x98));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(0x6A, 0x6E, 0x6F));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0x6A, 0x6E, 0x6F));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(0x6A, 0x6E, 0x6F));
    return p;
}

} // namespace whatsie::core
