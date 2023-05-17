#include "downloadmanagerwidget.h"
#include "downloadwidget.h"

DownloadManagerWidget::DownloadManagerWidget(QWidget *parent)
    : QWidget(parent), m_numDownloads(0) {
  setupUi(this);
}

void DownloadManagerWidget::acceptDownload(QWebEngineDownloadItem *download) {
  download->accept();
  add(new DownloadWidget(download));
  show();
}

void DownloadManagerWidget::downloadRequested(
    QWebEngineDownloadItem *download) {
  Q_ASSERT(download &&
           download->state() == QWebEngineDownloadItem::DownloadRequested);
  QString path =
      SettingsManager::instance().settings().value("defaultDownloadLocation",
                 QStandardPaths::writableLocation(
                     QStandardPaths::DownloadLocation) +
                     QDir::separator() + QApplication::applicationName())
          .toString();

  QDir().mkpath(path);

  auto proposed_file_name =
      path + QDir::separator() + download->downloadFileName();

  QFileInfo p_file_info(proposed_file_name);

  if (p_file_info.exists()) {

    QMessageBox msgBox;
    msgBox.setText("File with same name already exist!");
    msgBox.setInformativeText("Save file with a new name?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    switch (msgBox.exec()) {
    case QMessageBox::Save: {
      QString n_proposed_file_name = path + QDir::separator() +
                                     Utils::generateRandomId(5) + "_" +
                                     download->downloadFileName();
      download->setDownloadFileName(n_proposed_file_name);
      acceptDownload(download);
      break;
    }
    case QMessageBox::Cancel:
      break;
    default:
      break;
    }
  } else {
    download->setDownloadFileName(proposed_file_name);
    acceptDownload(download);
  }
}

void DownloadManagerWidget::add(DownloadWidget *downloadWidget) {
  connect(downloadWidget, &DownloadWidget::removeClicked, this,
          &DownloadManagerWidget::remove);
  m_itemsLayout->insertWidget(0, downloadWidget, 0, Qt::AlignTop);
  if (m_numDownloads++ == 0)
    m_zeroItemsLabel->hide();
}

void DownloadManagerWidget::remove(DownloadWidget *downloadWidget) {
  if (downloadWidget != nullptr) {
    m_itemsLayout->removeWidget(downloadWidget);
    downloadWidget->deleteLater();
  }
  if (--m_numDownloads == 0)
    m_zeroItemsLabel->show();
}

void DownloadManagerWidget::on_open_download_dir_clicked() {
  Utils::desktopOpenUrl(SettingsManager::instance().settings().value("defaultDownloadLocation",
                                   QStandardPaths::writableLocation(
                                       QStandardPaths::DownloadLocation) +
                                       QDir::separator() +
                                       QApplication::applicationName())
                            .toString());
}

void DownloadManagerWidget::keyPressEvent(QKeyEvent *e) {
  if (e->key() == Qt::Key_Escape)
    this->close();

  QWidget::keyPressEvent(e);
}

void DownloadManagerWidget::on_clear_all_downlads_clicked() {
  foreach (auto downloadItem, this->findChildren<DownloadWidget *>()) {
    if (downloadItem != nullptr) {
      downloadItem->remove();
    }
  }
}
