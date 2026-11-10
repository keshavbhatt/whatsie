// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
#pragma once

#include <QWidget>

class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;

namespace ktechpit {

/// The full list as a widget, for a host that shows it in a window of its own: every Ktechpit
/// app that runs here (installed ones marked), in the site's order, with a search field and a
/// Refresh button that fetches the list now. MoreAppsDialog is this view in a plain dialog.
class MoreAppsView : public QWidget
{
    Q_OBJECT

public:
    /// `place` names where the list is opened from ("menu"); it ends up in the ?ref= of links.
    explicit MoreAppsView(const QString& place, int columns = 2, QWidget* parent = nullptr);

    /// The line above the search field; empty hides it.
    void setIntro(const QString& text);
    /// How many apps the list has, or what the last Refresh did, for the host to show.
    [[nodiscard]] QString status() const { return m_status; }

Q_SIGNALS:
    void statusChanged(const QString& status);

private:
    void rebuild();
    void setStatus(const QString& status);

    QString m_place;
    int m_columns;
    QString m_status;
    QLabel* m_intro = nullptr;
    QLineEdit* m_search = nullptr;
    QPushButton* m_refresh = nullptr;
    QWidget* m_grid = nullptr;
    QGridLayout* m_gridLayout = nullptr;
};

} // namespace ktechpit
