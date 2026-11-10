// SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
// SPDX-License-Identifier: MIT
#pragma once

#include "AppCatalog.h"

#include <QFrame>

class QLabel;
class QPushButton;

namespace ktechpit {

/// One app: icon, name, one line and a Get button. Clicking the card opens the product page.
/// Object name "ktechpitAppCard", for hosts that style it.
class AppCard : public QFrame
{
    Q_OBJECT

public:
    /// `place` names where the card is shown ("about", "whatsnew"); it ends up in the ?ref=.
    AppCard(const AppInfo& app, const QString& place, QWidget* parent = nullptr);

    [[nodiscard]] const AppInfo& app() const { return m_app; }

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void updateIcon();

    AppInfo m_app;
    QString m_place;
    QLabel* m_icon = nullptr;
    QPushButton* m_get = nullptr;
};

} // namespace ktechpit
