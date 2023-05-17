#ifndef ABOUT_H
#define ABOUT_H

#include <QKeyEvent>
#include <QWidget>

namespace Ui {
class About;
}

class About : public QWidget {
  Q_OBJECT

public:
  explicit About(QWidget *parent = nullptr);
  ~About();

protected:
  void keyPressEvent(QKeyEvent *e) override;
private slots:
  void on_debugInfoButton_clicked();

private:
  Ui::About *ui;

  QString appName, appDescription, appSourceCodeLink, appAuthorLink,
      appAuthorName, appAuthorEmail, donateLink, moreAppsLink, appRateLink;
  bool isOpenSource;
};

#endif // ABOUT_H
