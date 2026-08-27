#include "ui/theme_applier.h"

#include "core/theme/theme_service.h"
#include "ui/logging.h"

#include <QApplication>
#include <QStyleFactory>
#include <QStyleHints>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

ThemeApplier::ThemeApplier(core::ThemeService& theme, QObject* parent)
    : QObject(parent)
    , m_theme(theme)
{
    if (QStyle* fusion = QStyleFactory::create(u"Fusion"_s)) {
        QApplication::setStyle(fusion);
    } else {
        qCWarning(lcUi) << "Fusion style unavailable; using platform default";
    }
    apply();
    connect(&m_theme, &core::ThemeService::effectiveSchemeChanged, this,
            [this](Qt::ColorScheme) { apply(); });
}

void ThemeApplier::apply()
{
    QApplication::setPalette(m_theme.palette());
    // WhatsApp Web follows prefers-color-scheme live (verified 2026-08-27 via
    // CDP media emulation). Qt WebEngine derives that media query from the
    // application colour scheme, so an explicit Light/Dark choice is pushed
    // into QStyleHints; "System" leaves the platform value alone (ADR-020).
    QStyleHints* hints = QApplication::styleHints();
    if (m_theme.followsSystem()) {
        hints->unsetColorScheme();
    } else {
        hints->setColorScheme(m_theme.effectiveScheme());
    }
}

} // namespace whatsie::ui
