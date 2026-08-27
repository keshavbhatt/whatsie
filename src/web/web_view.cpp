#include "web/web_view.h"

#include "core/settings/settings.h"
#include "web/logging.h"
#include "web/web_page.h"
#include "web/web_profile.h"

#include <QUrl>

using namespace Qt::StringLiterals;

namespace whatsie::web {

namespace {
const QUrl kWhatsAppUrl(u"https://web.whatsapp.com/"_s);
} // namespace

WebView::WebView(core::Settings& settings, QWidget* parent)
    : QWebEngineView(parent)
    , m_settings(settings)
    , m_profile(new WebProfile(settings, this))
    , m_page(new WebPage(*m_profile, this))
{
    setPage(m_page);
    setZoomFactor(m_settings.zoomFactor());
    connect(&m_settings, &core::Settings::zoomFactorChanged, this, &WebView::setZoomFactor);
}

void WebView::loadWhatsApp()
{
    if (url() == kWhatsAppUrl) {
        return;
    }
    qCInfo(lcWeb) << "loading" << kWhatsAppUrl;
    load(kWhatsAppUrl);
}

} // namespace whatsie::web
