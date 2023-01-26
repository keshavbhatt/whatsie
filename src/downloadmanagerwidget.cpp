#include "downloadmanagerwidget.h"

#include "downloadwidget.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QWebEngineDownloadItem>

DownloadManagerWidget::DownloadManagerWidget(QWidget *parent)
    : QWidget(parent), m_numDownloads(0) {
  setupUi(this);
}

void DownloadManagerWidget::downloadRequested(
    QWebEngineDownloadItem *download) {
  Q_ASSERT(download &&
           download->state() == QWebEngineDownloadItem::DownloadRequested);
  QString path =
      settings
          .value("defaultDownloadLocation",
                 QStandardPaths::writableLocation(
                     QStandardPaths::DownloadLocation) +
                     QDir::separator() + QApplication::applicationName())
          .toString();
  QDir d;
  d.mkpath(path);

  download->setDownloadFileName(path + QDir::separator() +
                                download->downloadFileName());
  download->accept();
  add(new DownloadWidget(download));
  show();
}

void DownloadManagerWidget::add(DownloadWidget *downloadWidget) {
  connect(downloadWidget, &DownloadWidget::removeClicked, this,
          &DownloadManagerWidget::remove);
  m_itemsLayout->insertWidget(0, downloadWidget, 0, Qt::AlignTop);
  if (m_numDownloads++ == 0)
    m_zeroItemsLabel->hide();
}

void DownloadManagerWidget::remove(DownloadWidget *downloadWidget) {
  m_itemsLayout->removeWidget(downloadWidget);
  downloadWidget->deleteLater();
  if (--m_numDownloads == 0)
    m_zeroItemsLabel->show();
}

void DownloadManagerWidget::on_open_download_dir_clicked() {
  utils::desktopOpenUrl(settings
                            .value("defaultDownloadLocation",
                                   QStandardPaths::writableLocation(
                                       QStandardPaths::DownloadLocation) +
                                       QDir::separator() +
                                       QApplication::applicationName())
                            .toString());
}

void DownloadManagerWidget::keyPressEvent(QKeyEvent *e)
{
   if (e->key() == Qt::Key_Escape)
      this->close();

   QWidget::keyPressEvent(e);
}
