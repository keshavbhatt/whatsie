#include "downloadmanagerwidget.h"

#include "downloadwidget.h"

#include <QFileDialog>
#include <QWebEngineDownloadItem>

DownloadManagerWidget::DownloadManagerWidget(QWidget *parent)
    : QWidget(parent)
    , m_numDownloads(0)
{
    setupUi(this);
}

void DownloadManagerWidget::downloadRequested(QWebEngineDownloadItem *download)
{
    //Q_ASSERT(download && download->state() == QWebEngineDownloadItem::DownloadRequested);
    QString path;
    if(settings.value("useNativeFileDialog",true).toBool() == false)
    {
        path = QFileDialog::getSaveFileName(this, tr("Save as"), download->path(),tr("Any file (*)"),nullptr,QFileDialog::DontUseNativeDialog);
    }else{
        path = QFileDialog::getSaveFileName(this, tr("Save as"), download->path(),tr("Any file (*)"),nullptr);
    }

    if (path.isEmpty())
        return;

    download->setPath(path);
    download->accept();
    add(new DownloadWidget(download));

    show();
}

void DownloadManagerWidget::add(DownloadWidget *downloadWidget)
{
    connect(downloadWidget, &DownloadWidget::removeClicked, this, &DownloadManagerWidget::remove);
    m_itemsLayout->insertWidget(0, downloadWidget, 0, Qt::AlignTop);
    if (m_numDownloads++ == 0)
        m_zeroItemsLabel->hide();
}

void DownloadManagerWidget::remove(DownloadWidget *downloadWidget)
{
    m_itemsLayout->removeWidget(downloadWidget);
    downloadWidget->deleteLater();
    if (--m_numDownloads == 0)
        m_zeroItemsLabel->show();
}
