// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
#pragma once

#include <QDialog>

namespace ktechpit {

/// The full list (MoreAppsView) in a plain dialog, with its status and a Close button.
class MoreAppsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MoreAppsDialog(const QString& place, QWidget* parent = nullptr);
};

} // namespace ktechpit
