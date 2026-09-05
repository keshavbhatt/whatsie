#include "web/file_drop.h"

#include "web/logging.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QMimeDatabase>

using namespace Qt::StringLiterals;

namespace whatsie::web {

QString guessMimeType(const QString& path)
{
    static QMimeDatabase db;
    const QMimeType type = db.mimeTypeForFile(path);
    return type.isValid() ? type.name() : u"application/octet-stream"_s;
}

DropOutcome buildDropPayload(const QStringList& paths, qint64 maxBytesPerFile)
{
    DropOutcome out;
    for (const QString& path : paths) {
        const QFileInfo info(path);
        if (info.isDir()) {
            continue; // folders aren't attachable; nothing to report
        }
        if (!info.exists()) {
            // Under a sandbox a dropped path outside the exported dirs (e.g. /tmp,
            // external drives) is simply not visible here — report it so the drop
            // does not fail silently.
            qCWarning(lcWeb) << "drop: cannot access" << path << "(missing or outside sandbox)";
            out.unreadable << info.fileName();
            continue;
        }
        if (info.size() > maxBytesPerFile) {
            qCWarning(lcWeb) << "drop: skipping" << path << "(" << info.size() << "bytes > cap)";
            out.tooLarge << info.fileName();
            continue;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qCWarning(lcWeb) << "drop: cannot read" << path << file.errorString();
            out.unreadable << info.fileName();
            continue;
        }
        const QByteArray bytes = file.readAll();
        out.totalBytes += bytes.size();
        out.files.append(QJsonObject{
            {u"name"_s, info.fileName()},
            {u"type"_s, guessMimeType(path)},
            {u"b64"_s, QString::fromLatin1(bytes.toBase64())},
        });
    }
    return out;
}

} // namespace whatsie::web
