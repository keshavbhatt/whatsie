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
        if (!info.isFile()) {
            continue; // dropped a directory or something unreadable
        }
        if (info.size() > maxBytesPerFile) {
            qCWarning(lcWeb) << "drop: skipping" << info.fileName() << "(" << info.size() << "bytes > cap)";
            out.skipped << info.fileName();
            continue;
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            out.skipped << info.fileName();
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
