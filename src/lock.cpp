#include "lock.h"
#include "ui_lock.h"
#include <QDebug>
#include <QKeyEvent>
#ifdef Q_OS_WIN32
#include <Windows.h>
#else
#include <X11/XKBlib.h>  // sudo apt install libx11-dev
#endif

Lock::Lock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Lock)
{
    ui->setupUi(this);
    ui->unlock->setEnabled(false);
    ui->setPass->setEnabled(false);
    ui->wrong->hide();

    if(settings.value("asdfg").isValid() == false)
    {
        ui->signup->show();
        ui->login->hide();
        ui->passcode1->setFocus();
    }
    else
    {
        lock_app();
    }
    checkCaps();
    QString capsStyle = QString("background-color: rgba(0,170,0,20);"
                                "padding:4px;"
                                "border:1px solid rgba(78, 154, 6,50);"
                                "border-radius: 2px;");
    ui->caps1->setStyleSheet(capsStyle);
    ui->caps2->setStyleSheet(capsStyle);
    ui->signup_warning->setStyleSheet(capsStyle);
    ui->wrong->setStyleSheet(capsStyle);
}

Lock::~Lock()
{
    delete ui;
}

void Lock::checkCaps()
{
    if(getCapsLockOn()){
        ui->caps1->show();
        ui->caps2->show();
    }else{
       ui->caps1->hide();
       ui->caps2->hide();
    }
}

void Lock::keyReleaseEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_CapsLock){
       checkCaps();
    }
}

bool Lock::event(QEvent* e)
{
    return QWidget::event(e);
}

void Lock::on_passcode1_textChanged(const QString &arg1)
{
    if(arg1.contains(" ")){
        ui->passcode1->setText(arg1.simplified());
    }
    ui->setPass->setEnabled(arg1.length()>4 && arg1== ui->passcode2->text());
}

void Lock::on_passcode2_textChanged(const QString &arg1)
{
    if(arg1.contains(" ")){
        ui->passcode2->setText(arg1.simplified());
    }
    ui->setPass->setEnabled(arg1.length()>4 && arg1== ui->passcode1->text());
}

void Lock::on_setPass_clicked()
{
    QString pass1,pass2;
    pass1 = ui->passcode1->text().trimmed();
    pass2 = ui->passcode2->text().trimmed();
    if(pass1==pass2)
    {
        settings.setValue("asdfg",QByteArray(pass1.toUtf8()).toBase64());
        settings.setValue("lockscreen",true);
        ui->passcode1->clear();
        ui->passcode2->clear();
        emit passwordSet();
        if(check_password_set()){
            ui->signup->hide();
            ui->login->show();
            ui->passcodeLogin->setFocus();
        }
    }else {
        return;
    }
}

bool Lock::check_password_set(){
    return settings.value("asdfg").isValid();
}

void Lock::on_unlock_clicked()
{
    QString password = QByteArray::fromBase64(settings.value("asdfg").toByteArray());
    if(ui->passcodeLogin->text() == password && check_password_set())
    {
        ui->login->hide();
        ui->signup->hide();
        ui->passcodeLogin->clear();
        isLocked = false;
        this->hide();
        emit unLocked();
    }else{
        ui->wrong->show();
    }
}

void Lock::on_passcodeLogin_textChanged(const QString &arg1)
{
    if(arg1.contains(" ")){
        ui->passcodeLogin->setText(arg1.simplified());
    }
    ui->wrong->hide();
    ui->unlock->setEnabled(arg1.length()>4);
}

void Lock::lock_app()
{
    checkCaps();
    ui->wrong->hide();
    ui->signup->hide();
    ui->login->show();
    ui->passcodeLogin->setFocus();
    isLocked = true;
    this->show();
}

void Lock::on_passcodeLogin_returnPressed()
{
    on_unlock_clicked();
}

bool Lock::getCapsLockOn()
{
// platform dependent method of determining if CAPS LOCK is on
#ifdef Q_OS_WIN32 // MS Windows version
    return GetKeyState(VK_CAPITAL) == 1;
#else // X11 version (Linux/Unix/Mac OS X/etc...)
    Display* d = XOpenDisplay((char*)0);
    bool caps_state = false;
    if (d) {
        unsigned n;
        XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
        caps_state = (n & 0x01) == 1;
    }
    return caps_state;
#endif
}

void Lock::on_cancelSetting_clicked()
{
    emit passwordNotSet();
    this->hide();
}
