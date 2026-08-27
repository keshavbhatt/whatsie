#pragma once

#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QWebEngineDownloadRequest>

namespace whatsie::core {
class DownloadModel;
class Settings;
} // namespace whatsie::core

namespace whatsie::web {

class WebProfile;

/// Accepts Chromium downloads into the configured folder (unique names,
/// optional "ask where to save"), mirrors progress into the persistent
/// DownloadModel and lets the UI cancel (FEATURES M8).
class DownloadController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadController)

public:
    DownloadController(WebProfile& profile, core::Settings& settings, core::DownloadModel& model,
                       QWidget* dialogParent, QObject* parent = nullptr);
    ~DownloadController() override;

    void cancel(quint64 id);
    [[nodiscard]] core::DownloadModel& model() { return m_model; }

Q_SIGNALS:
    void downloadStarted(quint64 id);

private:
    void handleRequest(QWebEngineDownloadRequest* request);
    void persist();

    struct Live
    {
        QPointer<QWebEngineDownloadRequest> request;
        QElapsedTimer clock;
        qint64 lastBytes = 0;
        qint64 lastMs = 0;
        double speed = 0;
    };

    WebProfile& m_profile;
    core::Settings& m_settings;
    core::DownloadModel& m_model;
    QWidget* m_dialogParent;
    QHash<quint64, Live> m_live;
    QString m_historyPath;
};

} // namespace whatsie::web
