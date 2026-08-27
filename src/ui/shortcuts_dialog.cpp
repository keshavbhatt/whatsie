#include "ui/shortcuts_dialog.h"

#include "ui/actions.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

ShortcutsDialog::ShortcutsDialog(const Actions& actions, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Keyboard shortcuts"));
    setModal(true);

    auto* form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignLeft);
    for (QAction* action : actions.all()) {
        const QKeySequence seq = action->shortcut();
        if (seq.isEmpty()) {
            continue;
        }
        QString text = action->text();
        text.remove(u'&');
        text.remove(u'…');
        auto* keys =
            new QLabel(u"<b>%1</b>"_s.arg(seq.toString(QKeySequence::NativeText).toHtmlEscaped()), this);
        form->addRow(text.trimmed(), keys);
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
    setMinimumWidth(320);
}

} // namespace whatsie::ui
