#include "web/web_page.h"

#include "core/navigation_policy.h"
#include "core/settings/settings.h"
#include "platform/file_manager.h"
#include "web/logging.h"
#include "web/popup_window.h"
#include "web/web_profile.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QSet>

using namespace Qt::StringLiterals;

namespace whatsie::web {

QString nameFilterFor(const QStringList& acceptedMimeTypes)
{
    const QString all = QCoreApplication::translate("FileChooser", "All files") + u" (*)"_s;
    if (acceptedMimeTypes.isEmpty()) {
        return all;
    }
    QMimeDatabase db;
    QSet<QString> patterns;
    for (const QString& accepted : acceptedMimeTypes) {
        if (accepted.startsWith(u'.')) {
            patterns.insert(u"*"_s + accepted);
            continue;
        }
        if (accepted.endsWith(u"/*"_s)) {
            const QString prefix = accepted.chopped(1);
            for (const QMimeType& type : db.allMimeTypes()) {
                if (type.name().startsWith(prefix)) {
                    for (const QString& glob : type.globPatterns()) {
                        patterns.insert(glob);
                    }
                }
            }
            continue;
        }
        const QMimeType type = db.mimeTypeForName(accepted);
        for (const QString& glob : type.globPatterns()) {
            patterns.insert(glob);
        }
    }
    if (patterns.isEmpty()) {
        return all;
    }
    QStringList sorted(patterns.cbegin(), patterns.cend());
    sorted.sort();
    return QCoreApplication::translate("FileChooser", "Supported files") + u" ("_s + sorted.join(u' ') +
           u");;"_s + all;
}

WebPage::WebPage(WebProfile& profile, QObject* parent)
    : QWebEnginePage(&profile, parent)
    , m_profile(profile)
{}

bool WebPage::acceptNavigationRequest(const QUrl& url, NavigationType type, bool isMainFrame)
{
    if (isMainFrame && type == NavigationTypeLinkClicked && core::shouldOpenExternally(url)) {
        qCInfo(lcWeb) << "link → system browser:" << url;
        platform::openUrl(url.toString());
        return false;
    }
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

QWebEnginePage* WebPage::createWindow(WebWindowType type)
{
    Q_UNUSED(type)
    // The window decides on its first navigation whether it stays (call
    // pop-out) or hands the URL to the browser and closes.
    auto* window = new PopupWindow(m_profile, m_host);
    window->show();
    Q_EMIT popupOpened(window);
    return window->page();
}

QStringList WebPage::chooseFiles(FileSelectionMode mode, const QStringList& oldFiles,
                                 const QStringList& acceptedMimeTypes)
{
    core::Settings& settings = m_profile.appSettings();
    const QString startDir =
        oldFiles.isEmpty() ? settings.lastOpenDirectory() : QFileInfo(oldFiles.first()).absolutePath();
    QStringList chosen;
    switch (mode) {
    case FileSelectOpen:
        chosen = {QFileDialog::getOpenFileName(m_host, tr("Choose a file"), startDir,
                                               nameFilterFor(acceptedMimeTypes))};
        break;
    case FileSelectOpenMultiple:
        chosen = QFileDialog::getOpenFileNames(m_host, tr("Choose files"), startDir,
                                               nameFilterFor(acceptedMimeTypes));
        break;
    case FileSelectUploadFolder:
        chosen = {QFileDialog::getExistingDirectory(m_host, tr("Choose a folder"), startDir)};
        break;
    case FileSelectSave:
        chosen = {QFileDialog::getSaveFileName(m_host, tr("Save file"), startDir,
                                               nameFilterFor(acceptedMimeTypes))};
        break;
    }
    chosen.removeAll(QString());
    if (!chosen.isEmpty()) {
        settings.setLastOpenDirectory(QFileInfo(chosen.first()).absolutePath());
    }
    return chosen;
}

void WebPage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message,
                                       int lineNumber, const QString& sourceId)
{
    switch (level) {
    case InfoMessageLevel:
        qCDebug(lcWebJs).noquote() << sourceId << lineNumber << message;
        break;
    case WarningMessageLevel:
        qCWarning(lcWebJs).noquote() << sourceId << lineNumber << message;
        break;
    case ErrorMessageLevel:
        qCCritical(lcWebJs).noquote() << sourceId << lineNumber << message;
        break;
    }
}

} // namespace whatsie::web
