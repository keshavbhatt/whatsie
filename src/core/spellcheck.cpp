#include "core/spellcheck.h"

#include <QDir>
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
