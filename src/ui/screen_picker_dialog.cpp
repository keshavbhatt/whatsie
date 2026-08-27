#include "ui/screen_picker_dialog.h"

#include "ui/logging.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

namespace whatsie::ui {

ScreenPickerDialog::ScreenPickerDialog(QWebEngineDesktopMediaRequest request, QWidget* parent)
    : QDialog(parent)
    , m_request(std::move(request))
{
    setWindowTitle(tr("Share your screen"));
    setModal(true);
    resize(480, 400);

    auto* hint = new QLabel(tr("Choose what WhatsApp may see during the call."), this);
    hint->setWordWrap(true);

    m_screens = new QListView(this);
    m_screens->setModel(m_request.screensModel());
    m_windows = new QListView(this);
    m_windows->setModel(m_request.windowsModel());
    for (QListView* view : {m_screens, m_windows}) {
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(view, &QListView::doubleClicked, this, &QDialog::accept);
    }
    if (m_request.screensModel()->rowCount() > 0) {
        m_screens->setCurrentIndex(m_request.screensModel()->index(0, 0));
    }

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(m_screens, tr("Entire screen"));
    m_tabs->addTab(m_windows, tr("Window"));

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Share"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(hint);
    layout->addWidget(m_tabs, 1);
    layout->addWidget(buttons);
}

void ScreenPickerDialog::accept()
{
    if (!m_answered) {
        const bool screenTab = m_tabs->currentWidget() == m_screens;
        const QModelIndex index = screenTab ? m_screens->currentIndex() : m_windows->currentIndex();
        if (!index.isValid()) {
            return; // nothing chosen yet
        }
        m_answered = true;
        qCInfo(lcUi) << "screen share:" << (screenTab ? "screen" : "window") << index.row();
        if (screenTab) {
            m_request.selectScreen(index);
        } else {
            m_request.selectWindow(index);
        }
    }
    QDialog::accept();
}

void ScreenPickerDialog::reject()
{
    if (!m_answered) {
        m_answered = true;
        qCInfo(lcUi) << "screen share cancelled";
        m_request.cancel();
    }
    QDialog::reject();
}

} // namespace whatsie::ui
