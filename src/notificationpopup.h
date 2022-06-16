#ifndef NOTIFICATIONPOPUP_H
#define NOTIFICATIONPOPUP_H

#pragma once

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
#include <QSettings>
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
  QSettings settings;

public:
  NotificationPopup(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::ToolTip);
    auto rootLayout = new QHBoxLayout(this);

    rootLayout->addWidget(&m_icon);

    auto bodyLayout = new QVBoxLayout;
    rootLayout->addLayout(bodyLayout);

    auto titleLayout = new QHBoxLayout;
    bodyLayout->addLayout(titleLayout);

    titleLayout->addWidget(&m_title);
    titleLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

    auto close = new QPushButton(tr("Close"));
    titleLayout->addWidget(close);
    connect(close, &QPushButton::clicked, this, &NotificationPopup::onClosed);

    bodyLayout->addWidget(&m_message);
    adjustSize();
  }

  void present(int screenNumber, QString title, QString message,
               const QPixmap image) {
    m_title.setText("<b>" + title + "</b>");
    m_message.setText(message);
    m_icon.setPixmap(
        image.scaledToHeight(m_icon.height(), Qt::SmoothTransformation));


    QTimer::singleShot(settings.value("notificationTimeOut", 9000).toInt(),
                       this, [=]() { onClosed(); });

    this->adjustSize();
    QRect screenRect = QGuiApplication::screens().at(screenNumber)->geometry();
    int x = (screenRect.x() + screenRect.width() - 30) - this->width();
    int y = 40;

    QPropertyAnimation *a = new QPropertyAnimation(this, "pos");
    a->setDuration(200);
    a->setStartValue(QApplication::desktop()->mapToGlobal(QPoint(x - 20, y)));
    a->setEndValue(QApplication::desktop()->mapToGlobal(QPoint(x, y)));
    a->setEasingCurve(QEasingCurve::Linear);
    a->start(QPropertyAnimation::DeleteWhenStopped);

    this->show();
  }

  void present(int screenNumber,
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

    connect(notification.get(), &QWebEngineNotification::closed, this,
            &NotificationPopup::onClosed);
    QTimer::singleShot(settings.value("notificationTimeOut", 9000).toInt(),
                       notification.get(), [&]() { onClosed(); });
    this->adjustSize();
    QRect screenRect = QGuiApplication::screens().at(screenNumber)->geometry();
    int x = (screenRect.x() + screenRect.width() - 30) - this->width();
    int y = 40;

    QPropertyAnimation *a = new QPropertyAnimation(this, "pos");
    a->setDuration(200);
    a->setStartValue(QApplication::desktop()->mapToGlobal(QPoint(x - 20, y)));
    a->setEndValue(QApplication::desktop()->mapToGlobal(QPoint(x, y)));
    a->setEasingCurve(QEasingCurve::Linear);
    a->start(QPropertyAnimation::DeleteWhenStopped);

    this->show();
  }

protected slots:
  void onClosed() {
    hide();
    if (notification) {
      notification->close();
      notification.reset();
    } else {
      this->deleteLater();
    }
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
