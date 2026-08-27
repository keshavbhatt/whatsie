#include "web/download_controller.h"

#include "core/downloads/download_model.h"
#include "core/downloads/file_naming.h"
#include "core/settings/settings.h"
#include "web/logging.h"
#include "web/web_profile.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>

using namespace Qt::StringLiterals;

namespace whatsie::web {

DownloadController::DownloadController(WebProfile& profile, core::Settings& settings,
                                       core::DownloadModel& model, QWidget* dialogParent, QObject* parent)
    : QObject(parent)
    , m_profile(profile)
    , m_settings(settings)
    , m_model(model)
    , m_dialogParent(dialogParent)
    , m_historyPath(core::DownloadModel::defaultFilePath())
{
    m_model.load(m_historyPath);
    connect(&m_profile, &QWebEngineProfile::downloadRequested, this, &DownloadController::handleRequest);
    connect(&m_model, &core::DownloadModel::changed, this, &DownloadController::persist);
}

DownloadController::~DownloadController()
{
    for (auto& live : m_live) {
        if (live.request) {
            live.request->cancel();
        }
    }
    m_model.save(m_historyPath);
}

void DownloadController::persist()
{
    m_model.save(m_historyPath);
}

void DownloadController::handleRequest(QWebEngineDownloadRequest* request)
{
    QString directory = m_settings.downloadDirectory();
    QString fileName = core::sanitizeFileName(request->downloadFileName());

    if (m_settings.askWhereToSave()) {
        const QString chosen =
            QFileDialog::getSaveFileName(m_dialogParent, tr("Save file"), QDir(directory).filePath(fileName));
        if (chosen.isEmpty()) {
            qCInfo(lcWeb) << "download declined by user:" << fileName;
            request->cancel();
            return;
        }
        const QFileInfo info(chosen);
        directory = info.absolutePath();
        fileName = info.fileName();
        m_settings.setDownloadDirectory(directory);
    } else {
        QDir().mkpath(directory);
        fileName = core::uniqueFileName(directory, fileName);
    }

    request->setDownloadDirectory(directory);
    request->setDownloadFileName(fileName);

    core::DownloadEntry entry;
    entry.fileName = fileName;
    entry.directory = directory;
    entry.mimeType = request->mimeType();
    entry.totalBytes = request->totalBytes();
    const quint64 id = m_model.add(entry);

    Live live;
    live.request = request;
    live.clock.start();
    m_live.insert(id, live);

    connect(request, &QWebEngineDownloadRequest::receivedBytesChanged, this, [this, id, request] {
        auto it = m_live.find(id);
        if (it == m_live.end()) {
            return;
        }
        Live& l = it.value();
        const qint64 now = l.clock.elapsed();
        const qint64 bytes = request->receivedBytes();
        if (now - l.lastMs >= 500) {
            l.speed = static_cast<double>(bytes - l.lastBytes) * 1000.0 / static_cast<double>(now - l.lastMs);
            l.lastBytes = bytes;
            l.lastMs = now;
        }
        m_model.updateProgress(id, bytes, request->totalBytes(), l.speed);
    });
    connect(request, &QWebEngineDownloadRequest::isFinishedChanged, this, [this, id, request] {
        if (!request->isFinished()) {
            return;
        }
        core::DownloadState state = core::DownloadState::Failed;
        QString error;
        switch (request->state()) {
        case QWebEngineDownloadRequest::DownloadCompleted:
            state = core::DownloadState::Completed;
            break;
        case QWebEngineDownloadRequest::DownloadCancelled:
            state = core::DownloadState::Cancelled;
            break;
        case QWebEngineDownloadRequest::DownloadInterrupted:
            error = request->interruptReasonString();
            break;
        case QWebEngineDownloadRequest::DownloadRequested:
        case QWebEngineDownloadRequest::DownloadInProgress:
            break;
        }
        m_model.updateProgress(id, request->receivedBytes(), request->totalBytes(), 0);
        m_model.finish(id, state, error);
        m_live.remove(id);
        qCInfo(lcWeb) << "download" << id << "finished:" << static_cast<int>(state) << error;
    });

    request->accept();
    qCInfo(lcWeb) << "download" << id << "started:" << fileName << "→" << directory;
    Q_EMIT downloadStarted(id);
}

void DownloadController::cancel(quint64 id)
{
    const auto it = m_live.constFind(id);
    if (it != m_live.constEnd() && it->request) {
        it->request->cancel();
    }
}

} // namespace whatsie::web
