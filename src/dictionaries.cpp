#include "dictionaries.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

// Returns the hunspell dictionary directory, preferring the DICPATH env var
// (set by the snap runtime / user override) then common system locations.
QString Dictionaries::GetDictionaryPath() {
    const QString env = qEnvironmentVariable("DICPATH");
    if (!env.isEmpty() && QDir(env).exists())
        return env;

    for (const char *path : {"/usr/share/hunspell", "/usr/share/myspell/dicts",
                              "/usr/share/myspell"}) {
        if (QDir(QLatin1String(path)).exists())
            return QLatin1String(path);
    }
    return {};
}

// Returns basenames of all *.dic files found in the dictionary directory,
// suitable for passing to QWebEngineProfile::setSpellCheckLanguages().
QStringList Dictionaries::GetDictionaries() {
    const QString path = GetDictionaryPath();
    if (path.isEmpty())
        return {};

    QStringList result;
    const QStringList files =
        QDir(path).entryList({QStringLiteral("*.dic")}, QDir::Files);
    for (const QString &file : files)
        result << QFileInfo(file).baseName();

    result.removeDuplicates();
    result.sort();
    return result;
}
