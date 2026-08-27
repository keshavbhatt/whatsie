#include "core/downloads/download_model.h"

#include "core/logging.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace whatsie::core {

namespace {

QLatin1StringView stateName(DownloadState state)
{
    switch (state) {
    case DownloadState::InProgress:
        return "inProgress"_L1;
    case DownloadState::Completed:
        return "completed"_L1;
    case DownloadState::Cancelled:
        return "cancelled"_L1;
    case DownloadState::Failed:
        return "failed"_L1;
    }
    return "failed"_L1;
}

DownloadState stateFromName(const QString& name)
{
    if (name == "inProgress"_L1) {
        return DownloadState::InProgress;
    }
    if (name == "completed"_L1) {
        return DownloadState::Completed;
    }
    if (name == "cancelled"_L1) {
        return DownloadState::Cancelled;
    }
    return DownloadState::Failed;
}

} // namespace

// ---- DownloadEntry ---------------------------------------------------------

QString DownloadEntry::filePath() const
{
    return QDir(directory).filePath(fileName);
}

QJsonObject DownloadEntry::toJson() const
{
    return {
        {u"id"_s, static_cast<qint64>(id)},
        {u"fileName"_s, fileName},
        {u"directory"_s, directory},
        {u"mimeType"_s, mimeType},
        {u"totalBytes"_s, totalBytes},
        {u"receivedBytes"_s, receivedBytes},
        {u"state"_s, QString(stateName(state))},
        {u"startedAt"_s, startedAt.toString(Qt::ISODateWithMs)},
        {u"finishedAt"_s, finishedAt.isValid() ? finishedAt.toString(Qt::ISODateWithMs) : QString()},
        {u"error"_s, error},
    };
}

DownloadEntry DownloadEntry::fromJson(const QJsonObject& o)
{
    DownloadEntry e;
    e.id = static_cast<quint64>(o.value(u"id"_s).toInteger());
    e.fileName = o.value(u"fileName"_s).toString();
    e.directory = o.value(u"directory"_s).toString();
    e.mimeType = o.value(u"mimeType"_s).toString();
    e.totalBytes = o.value(u"totalBytes"_s).toInteger(-1);
    e.receivedBytes = o.value(u"receivedBytes"_s).toInteger();
    e.state = stateFromName(o.value(u"state"_s).toString());
    e.startedAt = QDateTime::fromString(o.value(u"startedAt"_s).toString(), Qt::ISODateWithMs);
    e.finishedAt = QDateTime::fromString(o.value(u"finishedAt"_s).toString(), Qt::ISODateWithMs);
    e.error = o.value(u"error"_s).toString();
    return e;
}

// ---- DownloadModel ---------------------------------------------------------

DownloadModel::DownloadModel(QObject* parent)
    : QAbstractListModel(parent)
{}

int DownloadModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
}

QVariant DownloadModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return {};
    }
    const DownloadEntry& e = m_entries.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case FileNameRole:
        return e.fileName;
    case IdRole:
        return e.id;
    case DirectoryRole:
        return e.directory;
    case FilePathRole:
        return e.filePath();
    case MimeTypeRole:
        return e.mimeType;
    case TotalBytesRole:
        return e.totalBytes;
    case ReceivedBytesRole:
        return e.receivedBytes;
    case StateRole:
        return QVariant::fromValue(e.state);
    case StartedAtRole:
        return e.startedAt;
    case FinishedAtRole:
        return e.finishedAt;
    case ErrorRole:
        return e.error;
    case SpeedRole:
        return e.bytesPerSecond;
    default:
        return {};
    }
}

QHash<int, QByteArray> DownloadModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {FileNameRole, "fileName"},
        {DirectoryRole, "directory"},
        {FilePathRole, "filePath"},
        {MimeTypeRole, "mimeType"},
        {TotalBytesRole, "totalBytes"},
        {ReceivedBytesRole, "receivedBytes"},
        {StateRole, "state"},
        {StartedAtRole, "startedAt"},
        {FinishedAtRole, "finishedAt"},
        {ErrorRole, "error"},
        {SpeedRole, "speed"},
    };
}

quint64 DownloadModel::add(DownloadEntry entry)
{
    entry.id = m_nextId++;
    if (!entry.startedAt.isValid()) {
        entry.startedAt = QDateTime::currentDateTime();
    }
    beginInsertRows({}, 0, 0);
    m_entries.prepend(entry);
    endInsertRows();

    // Keep the history bounded; drop the oldest *finished* entries first.
    while (m_entries.size() > kMaxEntries) {
        int victim = -1;
        for (int i = static_cast<int>(m_entries.size()) - 1; i >= 0; --i) {
            if (!m_entries.at(i).isActive()) {
                victim = i;
                break;
            }
        }
        if (victim < 0) {
            break;
        }
        beginRemoveRows({}, victim, victim);
        m_entries.removeAt(victim);
        endRemoveRows();
    }
    Q_EMIT changed();
    return entry.id;
}

int DownloadModel::rowOf(quint64 id) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

std::optional<DownloadEntry> DownloadModel::entry(quint64 id) const
{
    const int row = rowOf(id);
    return row < 0 ? std::nullopt : std::optional<DownloadEntry>(m_entries.at(row));
}

void DownloadModel::updateProgress(quint64 id, qint64 receivedBytes, qint64 totalBytes, double bytesPerSecond)
{
    const int row = rowOf(id);
    if (row < 0) {
        return;
    }
    DownloadEntry& e = m_entries[row];
    e.receivedBytes = receivedBytes;
    e.totalBytes = totalBytes;
    e.bytesPerSecond = bytesPerSecond;
    const QModelIndex idx = index(row);
    Q_EMIT dataChanged(idx, idx, {ReceivedBytesRole, TotalBytesRole, SpeedRole});
}

void DownloadModel::finish(quint64 id, DownloadState state, const QString& error)
{
    const int row = rowOf(id);
    if (row < 0 || state == DownloadState::InProgress) {
        return;
    }
    DownloadEntry& e = m_entries[row];
    if (!e.isActive()) {
        return;
    }
    e.state = state;
    e.error = error;
    e.finishedAt = QDateTime::currentDateTime();
    e.bytesPerSecond = 0;
    if (state == DownloadState::Completed && e.totalBytes < 0) {
        e.totalBytes = e.receivedBytes;
    }
    const QModelIndex idx = index(row);
    Q_EMIT dataChanged(idx, idx);
    Q_EMIT entryFinished(id, state);
    Q_EMIT changed();
}

void DownloadModel::remove(quint64 id)
{
    const int row = rowOf(id);
    if (row < 0) {
        return;
    }
    beginRemoveRows({}, row, row);
    m_entries.removeAt(row);
    endRemoveRows();
    Q_EMIT changed();
}

void DownloadModel::clearFinished()
{
    bool removedAny = false;
    for (int i = static_cast<int>(m_entries.size()) - 1; i >= 0; --i) {
        if (!m_entries.at(i).isActive()) {
            beginRemoveRows({}, i, i);
            m_entries.removeAt(i);
            endRemoveRows();
            removedAny = true;
        }
    }
    if (removedAny) {
        Q_EMIT changed();
    }
}

int DownloadModel::activeCount() const
{
    return static_cast<int>(std::count_if(m_entries.cbegin(), m_entries.cend(),
                                          [](const DownloadEntry& e) { return e.isActive(); }));
}

QString DownloadModel::defaultFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/downloads.json"_s;
}

bool DownloadModel::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        return true; // nothing yet
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(lcCore) << "cannot read download history" << filePath << file.errorString();
        return false;
    }
    QJsonParseError error{};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qCWarning(lcCore) << "download history is not valid JSON:" << error.errorString();
        return false;
    }

    QList<DownloadEntry> loaded;
    quint64 maxId = 0;
    const QJsonArray items = doc.object().value(u"downloads"_s).toArray();
    for (const QJsonValue& v : items) {
        DownloadEntry e = DownloadEntry::fromJson(v.toObject());
        if (e.id == 0 || e.fileName.isEmpty()) {
            continue;
        }
        if (e.isActive()) {
            e.state = DownloadState::Failed;
            e.error = u"Interrupted"_s;
            e.finishedAt = e.startedAt;
        }
        maxId = std::max(maxId, e.id);
        loaded.append(e);
        if (loaded.size() >= kMaxEntries) {
            break;
        }
    }

    beginResetModel();
    m_entries = loaded;
    m_nextId = maxId + 1;
    endResetModel();
    return true;
}

bool DownloadModel::save(const QString& filePath) const
{
    QDir().mkpath(QFileInfo(filePath).absolutePath());
    QJsonArray items;
    for (const DownloadEntry& e : m_entries) {
        items.append(e.toJson());
    }
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(lcCore) << "cannot write download history" << filePath << file.errorString();
        return false;
    }
    file.write(QJsonDocument(QJsonObject{{u"version"_s, 1}, {u"downloads"_s, items}})
                   .toJson(QJsonDocument::Compact));
    return file.commit();
}

} // namespace whatsie::core
