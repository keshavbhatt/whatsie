// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
#include "MoreAppsDialog.h"

#include "MoreAppsView.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace ktechpit {

MoreAppsDialog::MoreAppsDialog(const QString& place, QWidget* parent)
    : QDialog(parent)
{
    setObjectName(u"ktechpitMoreAppsDialog"_s);
    setWindowTitle(tr("More apps from Ktechpit"));
    resize(720, 560);

    auto* view = new MoreAppsView(place, 2, this);
    auto* status = new QLabel(view->status(), this);
    status->setForegroundRole(QPalette::PlaceholderText);
    connect(view, &MoreAppsView::statusChanged, status, &QLabel::setText);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    auto* bottom = new QHBoxLayout;
    bottom->addWidget(status, 1);
    bottom->addWidget(buttons);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(view, 1);
    layout->addLayout(bottom);
}

} // namespace ktechpit
