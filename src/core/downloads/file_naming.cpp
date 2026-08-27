#include "core/downloads/file_naming.h"

#include <QDir>
#include <QFileInfo>

using namespace Qt::StringLiterals;

namespace whatsie::core {

QString uniqueFileName(const QString& directory, const QString& fileName,
                       const std::function<bool(const QString&)>& exists)
{
    const QDir dir(directory);
    if (!exists(dir.filePath(fileName))) {
        return fileName;
    }
    const QFileInfo info(fileName);
    const QString base = info.completeBaseName();
    const QString suffix = info.suffix().isEmpty() ? QString() : u"."_s + info.suffix();
    for (int i = 1; i < 10000; ++i) {
        const QString candidate = u"%1 (%2)%3"_s.arg(base).arg(i).arg(suffix);
        if (!exists(dir.filePath(candidate))) {
            return candidate;
        }
    }
    return fileName;
}

QString uniqueFileName(const QString& directory, const QString& fileName)
{
    return uniqueFileName(directory, fileName, [](const QString& path) { return QFileInfo::exists(path); });
}

QString sanitizeFileName(const QString& suggested)
{
    QString name;
    name.reserve(suggested.size());
    for (const QChar c : suggested) {
        if (c == u'/' || c == u'\\' || c == u':' || c.category() == QChar::Other_Control) {
            name.append(u'_');
        } else {
            name.append(c);
        }
    }
    name = name.trimmed();
    while (name.startsWith(u'.')) {
        name.remove(0, 1);
    }
    return name.isEmpty() ? u"download"_s : name;
}

QString humanSize(qint64 bytes)
{
    if (bytes < 0) {
        return QString();
    }
    constexpr double kUnit = 1024.0;
    if (bytes < 1024) {
        return u"%1 B"_s.arg(bytes);
    }
    double value = static_cast<double>(bytes) / kUnit;
    const char* const units[] = {"KB", "MB", "GB", "TB"};
    int idx = 0;
    while (value >= kUnit && idx < 3) {
        value /= kUnit;
        ++idx;
    }
    return u"%1 %2"_s.arg(value, 0, 'f', idx == 0 ? 0 : 1).arg(QLatin1StringView(units[idx]));
}

} // namespace whatsie::core
