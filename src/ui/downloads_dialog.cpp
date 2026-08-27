#include "ui/downloads_dialog.h"

#include "core/downloads/download_model.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;
using whatsie::core::DownloadModel;
using whatsie::core::DownloadState;

namespace whatsie::ui {

DownloadsDialog::DownloadsDialog(core::DownloadModel& model, QWidget* parent)
    : QDialog(parent, Qt::Window)
    , m_model(model)
{
    setupUi();
    updateSummary();
    connect(&m_model, &QAbstractItemModel::rowsInserted, this, &DownloadsDialog::updateSummary);
    connect(&m_model, &QAbstractItemModel::rowsRemoved, this, &DownloadsDialog::updateSummary);
    connect(&m_model, &QAbstractItemModel::modelReset, this, &DownloadsDialog::updateSummary);
    connect(&m_model, &DownloadModel::entryFinished, this,
            [this](quint64, DownloadState) { updateSummary(); });

    // Relative times ("5 min ago") need a periodic repaint.
    auto* tick = new QTimer(this);
    tick->setInterval(30'000);
    connect(tick, &QTimer::timeout, m_list->viewport(), qOverload<>(&QWidget::update));
    tick->start();
}

void DownloadsDialog::setupUi()
{
    setWindowTitle(tr("Downloads"));
    resize(560, 420);

    m_list = new QListView(this);
    m_list->setModel(&m_model);
    m_delegate = new DownloadsDelegate(m_list);
    m_list->setItemDelegate(m_delegate);
    m_list->setMouseTracking(true);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setUniformItemSizes(true);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(m_delegate, &DownloadsDelegate::actionRequested, this, &DownloadsDialog::actionRequested);
    connect(m_list, &QListView::customContextMenuRequested, this, &DownloadsDialog::showContextMenu);
    connect(m_list, &QListView::doubleClicked, this, [this](const QModelIndex& index) {
        if (index.data(DownloadModel::StateRole).value<DownloadState>() == DownloadState::Completed) {
            Q_EMIT actionRequested(index.data(DownloadModel::IdRole).toULongLong(),
                                   DownloadsDelegate::Action::Open);
        }
    });

    auto* empty = new QLabel(tr("Files you download from WhatsApp will appear here."), this);
    empty->setAlignment(Qt::AlignCenter);
    empty->setWordWrap(true);
    empty->setStyleSheet(u"color: palette(placeholder-text); font-size: 11pt;"_s);

    m_stack = new QStackedWidget(this);
    m_stack->addWidget(empty);
    m_stack->addWidget(m_list);

    m_summary = new QLabel(this);
    m_summary->setStyleSheet(u"color: palette(placeholder-text);"_s);
    auto* openFolder = new QPushButton(QIcon::fromTheme(u"folder-open"_s), tr("Open folder"), this);
    connect(openFolder, &QPushButton::clicked, this, &DownloadsDialog::openFolderRequested);
    m_clear = new QPushButton(QIcon::fromTheme(u"edit-clear-list"_s), tr("Clear finished"), this);
    connect(m_clear, &QPushButton::clicked, this, &DownloadsDialog::clearFinishedRequested);

    auto* bar = new QHBoxLayout;
    bar->addWidget(m_summary, 1);
    bar->addWidget(openFolder);
    bar->addWidget(m_clear);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_stack, 1);
    layout->addLayout(bar);
}

void DownloadsDialog::updateSummary()
{
    const int total = m_model.rowCount();
    const int active = m_model.activeCount();
    m_stack->setCurrentIndex(total == 0 ? 0 : 1);
    if (active > 0) {
        m_summary->setText(tr("%n download(s) in progress", nullptr, active));
    } else {
        m_summary->setText(total == 0 ? QString() : tr("%n download(s)", nullptr, total));
    }
    m_clear->setEnabled(total > active);
}

void DownloadsDialog::showContextMenu(const QPoint& pos)
{
    const QModelIndex index = m_list->indexAt(pos);
    if (!index.isValid()) {
        return;
    }
    const quint64 id = index.data(DownloadModel::IdRole).toULongLong();
    const auto state = index.data(DownloadModel::StateRole).value<DownloadState>();
    QMenu menu(this);
    if (state == DownloadState::InProgress) {
        menu.addAction(tr("Cancel"), this,
                       [this, id] { Q_EMIT actionRequested(id, DownloadsDelegate::Action::Cancel); });
    } else {
        if (state == DownloadState::Completed) {
            menu.addAction(tr("Open"), this,
                           [this, id] { Q_EMIT actionRequested(id, DownloadsDelegate::Action::Open); });
            menu.addAction(tr("Show in folder"), this,
                           [this, id] { Q_EMIT actionRequested(id, DownloadsDelegate::Action::Reveal); });
        }
        menu.addAction(tr("Remove from list"), this,
                       [this, id] { Q_EMIT actionRequested(id, DownloadsDelegate::Action::Remove); });
    }
    menu.exec(m_list->viewport()->mapToGlobal(pos));
}

} // namespace whatsie::ui
