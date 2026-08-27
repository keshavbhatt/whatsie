#include "core/storage_policy.h"

#include "core/logging.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>

using namespace Qt::StringLiterals;

namespace whatsie::core {

namespace {

QString canonical(const QString& path)
{
    const QFileInfo info(path);
    const QString c = info.canonicalFilePath();
    return c.isEmpty() ? QDir::cleanPath(info.absoluteFilePath()) : c;
}

bool isInside(const QString& child, const QString& root)
{
    if (root.isEmpty() || child == root) {
        return false;
    }
    const QString prefix = root.endsWith(u'/') ? root : root + u'/';
    return child.startsWith(prefix);
}

} // namespace

bool isSafeToDelete(const QString& path, const QStringList& allowedRoots)
{
    if (path.trimmed().isEmpty()) {
        return false;
    }
    const QFileInfo info(path);
    if (!info.exists() || !info.isDir() || info.isSymLink()) {
        return false;
    }
    const QString target = canonical(path);
    if (target.isEmpty() || target == u"/"_s) {
        return false;
    }

    // Never a directory the user sees as theirs.
    QStringList forbidden{QDir::homePath(), QDir::rootPath(), QDir::tempPath()};
    for (const auto location : {QStandardPaths::DocumentsLocation, QStandardPaths::DownloadLocation,
                                QStandardPaths::DesktopLocation, QStandardPaths::PicturesLocation}) {
        forbidden << QStandardPaths::writableLocation(location);
    }
    for (const QString& f : forbidden) {
        if (!f.isEmpty() && canonical(f) == target) {
            return false;
        }
    }

    for (const QString& root : allowedRoots) {
        if (isInside(target, canonical(root))) {
            return true;
        }
    }
    return false;
}

qint64 directorySize(const QString& path)
{
    qint64 total = 0;
    QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        total += it.fileInfo().size();
    }
    return total;
}

bool removeDirectorySafely(const QString& path, const QStringList& allowedRoots)
{
    if (!isSafeToDelete(path, allowedRoots)) {
        qCCritical(lcCore) << "refusing to delete" << path << "- outside the allowed roots";
        return false;
    }
    qCInfo(lcCore) << "removing" << path;
    return QDir(path).removeRecursively();
}

} // namespace whatsie::core
