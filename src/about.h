#ifndef ABOUT_H
#define ABOUT_H

#include <QWidget>

namespace Ui {
class About;
}

class About : public QWidget
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About();

private slots:
    void on_debugInfoButton_clicked();

private:
    Ui::About *ui;

    QString appName, appDescription,
            appSourceCodeLink, appAuthorLink,
            appAuthorName, appAuthorEmail,
            donateLink,moreAppsLink, appRateLink;
    bool isOpenSource;
};

#endif // ABOUT_H
