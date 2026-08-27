#include "ui/theme_applier.h"

#include "core/theme/theme_service.h"
#include "ui/logging.h"

#include <QApplication>
#include <QStyleFactory>

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
}

} // namespace whatsie::ui
