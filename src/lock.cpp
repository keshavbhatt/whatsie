#include "lock.h"
#include "ui_lock.h"

#include "X11/XKBlib.h" // keep this header at bottom

Lock::Lock(QWidget *parent) : QWidget(parent), ui(new Ui::Lock) {
  ui->setupUi(this);
  ui->setPass->setEnabled(false);
  ui->wrong->hide();

  MoreApps *moreApps =
      new MoreApps(this, nullptr, "keshavnrj",
                   QUrl("https://raw.githubusercontent.com/keshavbhatt/appdata/"
                        "main/moreapps.txt"),
                   false);
  moreApps->setWindowTitle("More Applications by developer");
  moreApps->setFixedHeight(98);
  ui->moreAppsLayout->addWidget(moreApps);

  passcodeLoginAction = ui->passcodeLogin->addAction(
      QIcon(":/icons/green_arrow-right-line.png"), QLineEdit::TrailingPosition);
  passcodeLoginAction->setEnabled(false);
  connect(
      passcodeLoginAction, &QAction::triggered, passcodeLoginAction, [this]() {
        QString password = QByteArray::fromBase64(SettingsManager::instance()
                                                      .settings()
                                                      .value("asdfg")
                                                      .toByteArray());
        if (ui->passcodeLogin->text() == password && check_password_set()) {
          isLocked = false;
          this->animateOut();
          emit unLocked();
        } else {
          ui->wrong->show();
        }
      });

  QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
  ui->centerWidget->setGraphicsEffect(eff);

  animateIn();

  if (SettingsManager::instance().settings().value("asdfg").isValid() ==
      false) {
    signUp();
  } else {
    lock_app();
  }
  checkCaps();
  QString capsStyle = QString(R"(background-color: palette(window);
                                padding: 4px;
                                border-radius: 2px;
                                color: palette(window-text);)");
  ui->caps1->setStyleSheet(capsStyle);
  ui->caps2->setStyleSheet(capsStyle);
  ui->signup_warning->setStyleSheet(capsStyle);
  ui->wrong->setStyleSheet(capsStyle);
  foreach (QLineEdit *le, this->findChildren<QLineEdit *>()) {
    le->setStyleSheet(R"(QLineEdit[echoMode="2"]{
                            lineedit-password-character: 9899;
                        })");
  }
}

void Lock::signUp() {
  isLocked = false;
  ui->signup->show();
  ui->login->hide();
  animateIn();
  ui->passcode1->setFocus();
}

void Lock::animateIn() {
  ui->centerWidget->hide();
  QPropertyAnimation *a =
      new QPropertyAnimation(ui->centerWidget->graphicsEffect(), "opacity");
  a->setDuration(500);
  a->setStartValue(0);
  a->setEndValue(1);
  a->setEasingCurve(QEasingCurve::InCubic);
  a->start(QPropertyAnimation::DeleteWhenStopped);
  ui->centerWidget->show();
}

void Lock::animateOut() {
  ui->centerWidget->show();
  QPropertyAnimation *a =
      new QPropertyAnimation(ui->centerWidget->graphicsEffect(), "opacity");
  a->setDuration(500);
  a->setStartValue(1);
  a->setEndValue(0);
  a->setEasingCurve(QEasingCurve::OutCubic);
  connect(a, &QPropertyAnimation::finished, this, [=] {
    ui->login->hide();
    ui->signup->hide();
    ui->passcodeLogin->clear();
    ui->centerWidget->hide();
    this->hide();
  });
  a->start(QPropertyAnimation::DeleteWhenStopped);
}

void Lock::applyThemeQuirks() {

  QString lblStyle = QString(R"(color: #c2c5d1;
                                padding: 0px 8px 0px 8px;
                                background: transparent;)");
  ui->label_4->setStyleSheet(lblStyle);
  ui->label_3->setStyleSheet(lblStyle);

  ui->login->setStyleSheet(R"(QWidget#login {
                              background-color: palette(window);
                              background-image: url(":/icons/wa_bg.png");
                            })");

  ui->signup->setStyleSheet(R"(QWidget#signup {
                              background-color: palette(window);
                              background-image: url(":/icons/wa_bg.png");
                            })");

  ui->widget_2->setStyleSheet(R"(QWidget#widget_2 {
                                border-top-left-radius: 4px;
                                border-top-right-radius: 4px;
                                background-image: url(":/icons/texture.png");
                                background-color: palette(shadow);
                            })");

  ui->widget->setStyleSheet(R"(QWidget#widget{
                                border-top-left-radius: 4px;
                                border-top-right-radius: 4px;
                                background-image:url(":/icons/texture.png");
                                background-color:palette(shadow);
                            })");

  ui->centerWidget->setStyleSheet(R"(QWidget#centerWidget {
                              background-image: url(":/icons/wa_bg.png");
                            })");

  // little quirks
  QString border = "border-bottom-right-radius: 4px;"
                   "border-bottom-left-radius: 4px;";
  QString lightBg = "background-color: rgb(37, 211, 102);";
  QString darkBg = "background-color: rgb(0, 117, 96);";
  if (QString::compare(SettingsManager::instance()
                           .settings()
                           .value("windowTheme", "light")
                           .toString(),
                       "dark", Qt::CaseInsensitive) == 0) { // light
    ui->bottomLine->setStyleSheet(darkBg + border);
    ui->bottomLine_2->setStyleSheet(darkBg + border);
  } else { // dark
    ui->bottomLine->setStyleSheet(lightBg + border);
    ui->bottomLine_2->setStyleSheet(lightBg + border);
  }
}

Lock::~Lock() { delete ui; }

void Lock::checkCaps() {
  if (getCapsLockOn()) {
    ui->caps1->show();
    ui->caps2->show();
  } else {
    ui->caps1->hide();
    ui->caps2->hide();
  }
}

void Lock::keyReleaseEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_CapsLock) {
    checkCaps();
  }
}

bool Lock::event(QEvent *e) { return QWidget::event(e); }

bool Lock::getIsLocked() const { return isLocked; }

void Lock::on_passcode1_textChanged(const QString &arg1) {
  if (arg1.contains(" ")) {
    ui->passcode1->setText(arg1.simplified());
  }
  ui->setPass->setEnabled(arg1.length() > 3 && arg1 == ui->passcode2->text());
}

void Lock::on_passcode2_textChanged(const QString &arg1) {
  if (arg1.contains(" ")) {
    ui->passcode2->setText(arg1.simplified());
  }
  ui->setPass->setEnabled(arg1.length() > 3 && arg1 == ui->passcode1->text());
}

void Lock::on_setPass_clicked() {
  QString pass1, pass2;
  pass1 = ui->passcode1->text().trimmed();
  pass2 = ui->passcode2->text().trimmed();
  if (pass1 == pass2) {
    SettingsManager::instance().settings().setValue(
        "asdfg", QByteArray(pass1.toUtf8()).toBase64());
    SettingsManager::instance().settings().setValue("lockscreen", true);
    ui->passcode1->clear();
    ui->passcode2->clear();
    emit passwordSet();
    if (check_password_set()) {
      ui->signup->hide();
      ui->login->show();
      ui->passcodeLogin->setFocus();
    }
  }
}

bool Lock::check_password_set() {
  return SettingsManager::instance().settings().value("asdfg").isValid();
}

void Lock::on_passcodeLogin_textChanged(const QString &arg1) {
  if (arg1.contains(" ")) {
    ui->passcodeLogin->setText(arg1.simplified());
  }
  ui->wrong->hide();
  passcodeLoginAction->setEnabled(arg1.length() > 3);
}

void Lock::lock_app() {
  checkCaps();
  ui->wrong->hide();
  ui->signup->hide();
  ui->login->show();
  this->show();
  isLocked = true;
  animateIn();
  ui->passcodeLogin->setFocus();
}

void Lock::on_passcodeLogin_returnPressed() { passcodeLoginAction->trigger(); }

bool Lock::getCapsLockOn() {
  Display *d = XOpenDisplay(nullptr);
  bool caps_state = false;
  if (d) {
    unsigned n;
    XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
    caps_state = (n & 0x01) == 1;
    XCloseDisplay(d);
  }
  return caps_state;
}

void Lock::on_cancelSetting_clicked() {
  isLocked = false;
  emit passwordNotSet();
  this->hide();
}
