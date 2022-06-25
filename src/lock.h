#ifndef LOCK_H
#define LOCK_H

#include <QSettings>
#include <QWidget>

namespace Ui {
class Lock;
}

class Lock : public QWidget {
  Q_OBJECT

public:
  explicit Lock(QWidget *parent = nullptr);
  ~Lock();
  bool getIsLocked() const;

private slots:
  void on_passcode1_textChanged(const QString &arg1);
  void on_passcode2_textChanged(const QString &arg1);
  void on_setPass_clicked();
  bool check_password_set();
  void on_unlock_clicked();
  void on_passcodeLogin_textChanged(const QString &arg1);
  void on_passcodeLogin_returnPressed();
  bool getCapsLockOn();
  void checkCaps();
  void on_cancelSetting_clicked();
  void animateIn();
  void animateOut();
public slots:
  void lock_app();
  void applyThemeQuirks();
  void signUp();
signals:
  void passwordSet();
  void passwordNotSet();
  void unLocked();

protected slots:
  void keyReleaseEvent(QKeyEvent *event);

  bool event(QEvent *e);

private:
  Ui::Lock *ui;
  bool isLocked = true;
  QSettings settings;
};

#endif // LOCK_H
