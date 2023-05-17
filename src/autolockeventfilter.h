#ifndef AUTOLOCKEVENTFILTER_H
#define AUTOLOCKEVENTFILTER_H

#include <QDebug>
#include <QEvent>
#include <QTimer>

class AutoLockEventFilter : public QObject {
  Q_OBJECT

public:
  explicit AutoLockEventFilter(int timeoutmillis)
      : timeoutmillis(timeoutmillis) {
    autoLockTimer = new QTimer(this);
    connect(autoLockTimer, &QTimer::timeout, this,
            QOverload<>::of(&AutoLockEventFilter::lockApp));
    resetTimer();
  }

  ~AutoLockEventFilter() {
    autoLockTimer->stop();
    delete autoLockTimer;
  }

signals:
  void autoLockTimerTimeout();

private:
  QTimer *autoLockTimer = nullptr;
  int timeoutmillis;

public slots:
  void stopTimer() { autoLockTimer->stop(); }
  void resetTimer() { autoLockTimer->start(timeoutmillis); }
  void lockApp() { emit autoLockTimerTimeout(); }
  void setTimeoutmillis(int newTimeoutmillis) {
    timeoutmillis = newTimeoutmillis;
  }

protected:
  bool eventFilter(QObject *obj, QEvent *ev) {
    if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::MouseMove ||
        ev->type() == QEvent::Wheel || ev->type() == QEvent::TouchUpdate ||
        ev->type() == QEvent::MouseButtonRelease ||
        ev->type() == QEvent::MouseButtonPress ||
        ev->type() == QEvent::MouseButtonDblClick ||
        ev->type() == QEvent::Gesture || ev->type() == QEvent::FocusIn ||
        ev->type() == QEvent::FocusOut || ev->type() == QEvent::Enter) {
      resetTimer();
    }
    return QObject::eventFilter(obj, ev);
  }
};
#endif // AUTOLOCKEVENTFILTER_H
