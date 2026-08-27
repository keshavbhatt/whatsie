#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>

// Persistent download history (FEATURES M8, revised per owner on 2026-08-27:
// "must look good and persist"). Pure model: no WebEngine, no widgets.
namespace whatsie::core {

enum class DownloadState
{
    InProgress,
    Completed,
    Cancelled,
    Failed,
};

struct DownloadEntry
{
    quint64 id = 0;
    QString fileName;
    QString directory;
    QString mimeType;
    qint64 totalBytes = -1; ///< -1 when unknown
    qint64 receivedBytes = 0;
    DownloadState state = DownloadState::InProgress;
    QDateTime startedAt;
    QDateTime finishedAt;
    QString error;
    double bytesPerSecond = 0; ///< runtime only, not persisted

    [[nodiscard]] QString filePath() const;
    [[nodiscard]] bool isActive() const { return state == DownloadState::InProgress; }
    [[nodiscard]] QJsonObject toJson() const;
    [[nodiscard]] static DownloadEntry fromJson(const QJsonObject& object);
};

class DownloadModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadModel)

public:
    enum Role
    {
        IdRole = Qt::UserRole + 1,
        FileNameRole,
        DirectoryRole,
        FilePathRole,
        MimeTypeRole,
        TotalBytesRole,
        ReceivedBytesRole,
        StateRole,
        StartedAtRole,
        FinishedAtRole,
        ErrorRole,
        SpeedRole,
    };

    explicit DownloadModel(QObject* parent = nullptr);
    ~DownloadModel() override = default;

    // QAbstractListModel
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /// Newest entries first. Returns the assigned id.
    quint64 add(DownloadEntry entry);
    void updateProgress(quint64 id, qint64 receivedBytes, qint64 totalBytes, double bytesPerSecond);
    void finish(quint64 id, DownloadState state, const QString& error = {});
    void remove(quint64 id);
    void clearFinished();

    [[nodiscard]] std::optional<DownloadEntry> entry(quint64 id) const;
    [[nodiscard]] int rowOf(quint64 id) const;
    [[nodiscard]] const QList<DownloadEntry>& entries() const { return m_entries; }
    [[nodiscard]] int activeCount() const;

    /// Persistence. Entries still InProgress at load time become Failed
    /// ("interrupted") — the transfer did not survive the previous run.
    bool load(const QString& filePath);
    bool save(const QString& filePath) const;
    [[nodiscard]] static QString defaultFilePath();

    static constexpr int kMaxEntries = 200;

Q_SIGNALS:
    void entryFinished(quint64 id, whatsie::core::DownloadState state);
    void changed(); ///< anything worth persisting happened

private:
    QList<DownloadEntry> m_entries;
    quint64 m_nextId = 1;
};

} // namespace whatsie::core

Q_DECLARE_METATYPE(whatsie::core::DownloadState)
