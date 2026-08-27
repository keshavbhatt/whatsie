#include "ui/main_window.h"

#include "core/settings/settings.h"
#include "ui/logging.h"
#include "web/web_view.h"

#include <QCloseEvent>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

namespace {
constexpr int kDefaultWidth = 1100;
constexpr int kDefaultHeight = 720;
} // namespace

MainWindow::MainWindow(core::Settings& settings, QWidget* parent)
    : QMainWindow(parent)
    , m_settings(settings)
{
    setupUi();
    restoreWindowState();
    m_webView->loadWhatsApp();
}

void MainWindow::setupUi()
{
    setWindowTitle(u"WhatsApp"_s);
    resize(kDefaultWidth, kDefaultHeight);

    m_webView = new web::WebView(m_settings, this);
    setCentralWidget(m_webView);
}

void MainWindow::restoreWindowState()
{
    const QByteArray geometry = m_settings.windowGeometry();
    if (!geometry.isEmpty() && !restoreGeometry(geometry)) {
        qCWarning(lcUi) << "stored window geometry rejected, using defaults";
    }
    const QByteArray state = m_settings.windowState();
    if (!state.isEmpty()) {
        restoreState(state);
    }
}

void MainWindow::saveWindowState()
{
    m_settings.setWindowGeometry(saveGeometry());
    m_settings.setWindowState(saveState());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveWindowState();
    QMainWindow::closeEvent(event);
}

} // namespace whatsie::ui
