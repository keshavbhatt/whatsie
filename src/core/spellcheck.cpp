#include "core/spellcheck.h"

#include <QDir>
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

} // namespace whatsie::core
