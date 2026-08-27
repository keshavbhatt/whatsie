#include "app/application.h"

#include "app/version.h"
#include "core/logging.h"
#include "core/settings/settings.h"
#include "platform/platform_info.h"

#include <QIcon>

using namespace Qt::StringLiterals;

namespace whatsie::app {

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setApplicationName(QString::fromLatin1(version::kApplicationName));
    setApplicationDisplayName(QString::fromLatin1(version::kDisplayName));
    setApplicationVersion(QString::fromLatin1(version::kVersion));
    setOrganizationName(QString::fromLatin1(version::kOrganizationName));
    setOrganizationDomain(QString::fromLatin1(version::kOrganizationDomain));
    setDesktopFileName(QString::fromLatin1(version::kDesktopId));
    setQuitOnLastWindowClosed(true);

    // Settings must be created after the identity is set (QSettings derives
    // its location from it).
    m_settings = std::make_unique<core::Settings>();

    qCInfo(core::lcCore).noquote() << u"whatsie %1 (%2) on %3"_s.arg(
        QString::fromLatin1(version::kVersion), QString::fromLatin1(version::kGitRevision),
        platform::describeHost());
}

Application::~Application()
{
    m_settings->sync();
}

core::Settings& Application::settings()
{
    return *m_settings;
}

} // namespace whatsie::app
