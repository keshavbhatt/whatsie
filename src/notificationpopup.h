#ifndef NOTIFICATIONPOPUP_H
#define NOTIFICATIONPOPUP_H

#include "settingsmanager.h"
#include "widgets/scrolltext/scrolltext.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QScreen>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWebEngineNotification>

#include <memory>

class NotificationPopup : public QWidget {
  Q_OBJECT

  QLabel m_icon, m_title;
  ScrollText m_message;
  std::unique_ptr<QWebEngineNotification> notification;

public:
  NotificationPopup(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::ToolTip);

    auto baseLayout = new QVBoxLayout(this);
    baseLayout->setContentsMargins(0, 0, 0, 0);

    auto rootLayout = new QHBoxLayout;
    baseLayout->addLayout(rootLayout);

    m_icon.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    rootLayout->addWidget(&m_icon);

    auto bodyLayout = new QVBoxLayout;
    rootLayout->addLayout(bodyLayout);

    auto titleLayout = new QHBoxLayout;
    titleLayout->setContentsMargins(0, 0, 9, 0);
    bodyLayout->addLayout(titleLayout);

    titleLayout->addWidget(&m_title);
    titleLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    auto close = new QPushButton(tr("Close"));
    connect(close, &QPushButton::clicked, this, &NotificationPopup::onClosed);
    titleLayout->addWidget(close);

    bodyLayout->addWidget(&m_message);

    auto *baseLineWidget = new QWidget(this);
    baseLineWidget->setFixedHeight(6);
    baseLineWidget->setStyleSheet("background-color: rgb(6, 207, 156);");
    baseLayout->addWidget(baseLineWidget);

    this->adjustSize();
  }

  void present(QScreen *screen, QString title, QString message,
               const QPixmap image) {
    m_title.setText("<b>" + title + "</b>");
    m_message.setText(message);
    m_icon.setPixmap(
        image.scaledToHeight(m_icon.height(), Qt::SmoothTransformation));
    this->adjustSize();

    QTimer::singleShot(SettingsManager::instance()
                           .settings()
                           .value("notificationTimeOut", 9000)
                           .toInt(),
                       this, [=]() { onClosed(); });

    animateIn(screen);
  }

  void present(QScreen *screen,
               std::unique_ptr<QWebEngineNotification> &newNotification) {
    if (notification) {
      notification->close();
      notification.reset();
    }

    notification.swap(newNotification);

    m_title.setText("<b>" + notification->title() + "</b>");
    m_message.setText(notification->message());
    m_icon.setPixmap(QPixmap::fromImage(notification->icon())
                         .scaledToHeight(m_icon.height()));

    notification->show();
    this->adjustSize();

    connect(notification.get(), &QWebEngineNotification::closed, this,
            &NotificationPopup::onClosed);
    QTimer::singleShot(SettingsManager::instance()
                           .settings()
                           .value("notificationTimeOut", 9000)
                           .toInt(),
                       notification.get(), [&]() { onClosed(); });

    animateIn(screen);
  }

protected slots:
  void animateIn(QScreen *screen) {
      if (!screen) {
          return;
      }

      QRect screenRect = screen->availableGeometry();
      int x = (screenRect.x() + screenRect.width() - 20) - this->width();
      int y = 40;

      QPropertyAnimation *a = new QPropertyAnimation(this, "pos");
      a->setDuration(200);
      a->setStartValue(QPoint(x - 10, y));
      a->setEndValue(QPoint(x, y));
      a->setEasingCurve(QEasingCurve::InCubic);
      a->start(QPropertyAnimation::DeleteWhenStopped);

      this->show();
  }

  void onClosed() {
    auto x = this->pos().x();
    auto y = this->pos().y();
    QPropertyAnimation *a = new QPropertyAnimation(this, "pos");
    a->setDuration(150);
    a->setStartValue(QApplication::desktop()->mapToGlobal(QPoint(x, y)));
    a->setEndValue(QApplication::desktop()->mapToGlobal(
        QPoint(x, -(this->height() + 20))));
    a->setEasingCurve(QEasingCurve::Linear);

    connect(a, &QPropertyAnimation::finished, this, [=]() {
      if (notification) {
        notification->close();
        notification.reset();
      }
      this->close();
    });
    a->start(QPropertyAnimation::DeleteWhenStopped);
  }

protected:
  void mouseReleaseEvent(QMouseEvent *event) override {
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton) {
      emit notification_clicked();
      if (notification)
        notification->click();
      onClosed();
    }
  }
signals:
  void notification_clicked();
};

#endif // NOTIFICATIONPOPUP_H
