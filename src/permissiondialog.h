#ifndef PERMISSIONDIALOG_H
#define PERMISSIONDIALOG_H

#include <QSettings>
#include <QWebEnginePage>
#include <QWidget>
#include <QKeyEvent>

namespace Ui {
class PermissionDialog;
}

class PermissionDialog : public QWidget {
  Q_OBJECT

public:
  explicit PermissionDialog(QWidget *parent = nullptr);
  ~PermissionDialog();

signals:
  void webPageFeatureChanged(QWebEnginePage::Feature feature);

protected slots:
  void keyPressEvent(QKeyEvent *e);
private slots:
  void addToFeaturesTable(QWebEnginePage::Feature feature,
                          QString &featureName);

private:
  Ui::PermissionDialog *ui;
  QSettings settings;
};

#endif // PERMISSIONDIALOG_H
