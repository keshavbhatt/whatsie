#include "core/spellcheck.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>

using namespace Qt::StringLiterals;

namespace whatsie::core {

QString dictionaryNameForLocale(const QLocale& locale)
{
    const QString name = locale.name(); // "en_US", "de", or "C"
    if (name.isEmpty() || name == "C"_L1) {
        return u"en-US"_s;
    }
    QString dict = name;
    dict.replace(u'_', u'-');
    return dict;
}

QString findDictionariesPath(const QStringList& candidateRoots)
{
    for (const QString& root : candidateRoots) {
        if (root.isEmpty()) {
            continue;
        }
        const QString path = QDir(root).filePath(u"qtwebengine_dictionaries"_s);
        if (QDir(path).exists()) {
            return path;
        }
    }
    return {};
}

QStringList availableDictionaries(const QString& dictionaryDir)
{
    if (dictionaryDir.isEmpty()) {
        return {};
    }
    QStringList names;
    const auto entries = QDir(dictionaryDir).entryInfoList({u"*.bdic"_s}, QDir::Files, QDir::Name);
    for (const QFileInfo& entry : entries) {
        names << entry.completeBaseName();
    }
    names.sort();
    return names;
}

QString prepareDictionaryDir(const QString& bundledDir, const QString& userDir)
{
    if (userDir.isEmpty()) {
        return bundledDir; // nowhere writable — fall back to the bundled set
    }
    QDir dir(userDir);
    if (!dir.exists() && !QDir().mkpath(userDir)) {
        return bundledDir; // could not create the writable directory
    }
    // Drop every bundled symlink we previously created (valid or now-stale, e.g.
    // a gone snap revision) so the set is rebuilt fresh; a user's own real .bdic
    // is left untouched.
    const auto present = dir.entryInfoList({u"*.bdic"_s}, QDir::Files | QDir::System);
    for (const QFileInfo& entry : present) {
        if (entry.isSymLink()) {
            QFile::remove(entry.absoluteFilePath());
        }
    }
    // Link the current bundled dictionaries in beside the user's own, never
    // clobbering a real file the user dropped with the same name.
    if (!bundledDir.isEmpty()) {
        const auto bundled = QDir(bundledDir).entryInfoList({u"*.bdic"_s}, QDir::Files, QDir::Name);
        for (const QFileInfo& entry : bundled) {
            const QString dest = dir.filePath(entry.fileName());
            if (QFileInfo::exists(dest)) {
                continue; // a user override with the same name — keep theirs
            }
            if (!QFile::link(entry.absoluteFilePath(), dest)) {
                QFile::copy(entry.absoluteFilePath(), dest); // symlinks unavailable
            }
        }
    }
    return userDir;
}

QString resolveDictionary(const QString& want, const QStringList& available)
{
    if (want.isEmpty() || available.isEmpty()) {
        return {};
    }
    if (available.contains(want)) {
        return want;
    }
    const QString language = want.section(u'-', 0, 0); // "en-IN" -> "en"
    if (available.contains(language)) {
        return language;
    }
    const QString prefix = language + u'-';
    for (const QString& name : available) { // available is sorted; first variant wins
        if (name.startsWith(prefix)) {
            return name;
        }
    }
    return {}; // no dictionary for this language — spell check stays off
}

} // namespace whatsie::core
