#include "core/log_sink.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>

#include <deque>

using namespace Qt::StringLiterals;

namespace whatsie::core {

namespace {

constexpr int kRingCapacity = 1000;

struct State
{
    QMutex mutex;
    QtMessageHandler previous = nullptr;
    bool installed = false;
    QString filePath;
    qint64 maxBytes = 0;
    QFile file;
    std::deque<QString> ring;
};

State& state()
{
    static State s;
    return s;
}

QLatin1StringView levelName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return "debug"_L1;
    case QtInfoMsg:
        return "info"_L1;
    case QtWarningMsg:
        return "warning"_L1;
    case QtCriticalMsg:
        return "critical"_L1;
    case QtFatalMsg:
        return "fatal"_L1;
    }
    return "?"_L1;
}

QString formatLine(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    const QString category =
        context.category != nullptr ? QString::fromLatin1(context.category) : u"default"_s;
    return u"%1 [%2] %3: %4"_s.arg(QDateTime::currentDateTime().toString(u"yyyy-MM-dd hh:mm:ss.zzz"_s),
                                   category, levelName(type), message);
}

void rotateIfNeeded(State& s)
{
    if (s.maxBytes <= 0 || !s.file.isOpen() || s.file.size() < s.maxBytes) {
        return;
    }
    s.file.close();
    const QString backup = s.filePath + u".1"_s;
    QFile::remove(backup);
    if (!QFile::rename(s.filePath, backup)) {
        // Could not rotate; keep appending to the current file rather than lose output.
    }
    if (!s.file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        s.filePath.clear();
    }
}

void handler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    State& s = state();
    const QString line = formatLine(type, context, message);
    {
        QMutexLocker lock(&s.mutex);
        s.ring.push_back(line);
        while (s.ring.size() > kRingCapacity) {
            s.ring.pop_front();
        }
        if (s.file.isOpen()) {
            s.file.write(line.toUtf8());
            s.file.write("\n");
            s.file.flush();
            rotateIfNeeded(s);
        }
    }
    if (s.previous != nullptr) {
        s.previous(type, context, message);
    }
}

} // namespace

void LogSink::install()
{
    State& s = state();
    QMutexLocker lock(&s.mutex);
    if (s.installed) {
        return;
    }
    s.installed = true;
    s.previous = qInstallMessageHandler(handler);
}

void LogSink::setLogFile(const QString& path, qint64 maxBytes)
{
    State& s = state();
    QMutexLocker lock(&s.mutex);
    if (s.file.isOpen()) {
        s.file.close();
    }
    s.filePath = path;
    s.maxBytes = maxBytes;
    if (path.isEmpty()) {
        return;
    }
    QDir().mkpath(QFileInfo(path).absolutePath());
    s.file.setFileName(path);
    if (!s.file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        s.filePath.clear();
        return;
    }
    rotateIfNeeded(s);
}

QString LogSink::logFilePath()
{
    State& s = state();
    QMutexLocker lock(&s.mutex);
    return s.filePath;
}

QString LogSink::defaultLogFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/logs/whatsie.log"_s;
}

QStringList LogSink::recentLines()
{
    State& s = state();
    QMutexLocker lock(&s.mutex);
    QStringList lines;
    lines.reserve(static_cast<qsizetype>(s.ring.size()));
    for (const QString& line : s.ring) {
        lines.append(line);
    }
    return lines;
}

} // namespace whatsie::core
