#include "downloadwidget.h"
#include "utils.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>
#include <QWebEngineDownloadItem>

DownloadWidget::DownloadWidget(QWebEngineDownloadItem *download,
                               QWidget *parent)
    : QFrame(parent), m_download(download) {
  setupUi(this);

  static QIcon openIcon(":/icons/eye-line.png");
  openButton->setIcon(openIcon);
  openButton->setEnabled(false);

  auto downloadFileName = m_download->downloadFileName();
  dstName->setText(QFileInfo(downloadFileName).fileName());

  auto url = m_download->url().toDisplayString();
  if (url.startsWith("data:")) {
    srcUrl->setText("data:...");
  } else {
    srcUrl->setText(url);
  }

  connect(openButton, &QPushButton::clicked, m_download, [this](bool) {
    if (m_download->state() == QWebEngineDownloadItem::DownloadCompleted) {
          Utils::desktopOpenUrl(m_download->downloadFileName());
    }
  });

  connect(cancelButton, &QPushButton::clicked, m_download, [this](bool) {
    if (m_download->state() == QWebEngineDownloadItem::DownloadInProgress)
      m_download->cancel();
    else
      emit removeClicked(this);
  });

  connect(m_download, &QWebEngineDownloadItem::downloadProgress, this,
          &DownloadWidget::updateWidget);

  connect(m_download, &QWebEngineDownloadItem::stateChanged, this,
          &DownloadWidget::updateWidget);

  updateWidget();
}

void DownloadWidget::remove() {
  if (!isDownloading())
    emit removeClicked(this);
}

inline QString DownloadWidget::withUnit(qreal bytes) {
  if (bytes < (1 << 10))
    return tr("%L1 B").arg(bytes);
  else if (bytes < (1 << 20))
    return tr("%L1 KiB").arg(bytes / (1 << 10), 0, 'f', 2);
  else if (bytes < (1 << 30))
    return tr("%L1 MiB").arg(bytes / (1 << 20), 0, 'f', 2);
  else
    return tr("%L1 GiB").arg(bytes / (1 << 30), 0, 'f', 2);
}

bool DownloadWidget::isDownloading() {

  return m_download->state() == QWebEngineDownloadItem::DownloadInProgress;
}

void DownloadWidget::updateWidget() {
  qreal totalBytes = m_download->totalBytes();
  qreal receivedBytes = m_download->receivedBytes();
  qreal bytesPerSecond = receivedBytes / m_timeAdded.elapsed() * 1000;

  auto state = m_download->state();
  switch (state) {
  case QWebEngineDownloadItem::DownloadRequested:
    Q_UNREACHABLE();
    break;
  case QWebEngineDownloadItem::DownloadInProgress:
    if (totalBytes >= 0) {
      progressBar->setValue(qRound(100 * receivedBytes / totalBytes));
      progressBar->setDisabled(false);
      progressBar->setFormat(tr("%p% - %1 of %2 downloaded - %3/s")
                                 .arg(withUnit(receivedBytes),
                                      withUnit(totalBytes),
                                      withUnit(bytesPerSecond)));
    } else {
      progressBar->setValue(0);
      progressBar->setDisabled(false);
      progressBar->setFormat(
          tr("unknown size - %1 downloaded - %2/s")
              .arg(withUnit(receivedBytes), withUnit(bytesPerSecond)));
    }
    openButton->setEnabled(false);
    break;
  case QWebEngineDownloadItem::DownloadCompleted:
    progressBar->setValue(100);
    progressBar->setDisabled(true);
    progressBar->setFormat(
        tr("completed - %1 downloaded - %2/s")
            .arg(withUnit(receivedBytes), withUnit(bytesPerSecond)));
    openButton->setEnabled(true);
    break;
  case QWebEngineDownloadItem::DownloadCancelled:
    progressBar->setValue(0);
    progressBar->setDisabled(true);
    progressBar->setFormat(
        tr("cancelled - %1 downloaded - %2/s")
            .arg(withUnit(receivedBytes), withUnit(bytesPerSecond)));
    openButton->setEnabled(false);
    break;
  case QWebEngineDownloadItem::DownloadInterrupted:
    progressBar->setValue(0);
    progressBar->setDisabled(true);
    progressBar->setFormat(
        tr("interrupted: %1").arg(m_download->interruptReasonString()));
    openButton->setEnabled(false);
    break;
  }

  if (state == QWebEngineDownloadItem::DownloadInProgress) {
    static QIcon cancelIcon(":/icons/stop-line.png");
    cancelButton->setIcon(cancelIcon);
    cancelButton->setToolTip(tr("Stop downloading"));
  } else {
    static QIcon removeIcon(":/icons/close-fill.png");
    cancelButton->setIcon(removeIcon);
    cancelButton->setToolTip(tr("Remove from list"));
  }
}
