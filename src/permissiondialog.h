#ifndef PERMISSIONDIALOG_H
#define PERMISSIONDIALOG_H

#include <QCheckBox>
#include <QKeyEvent>
#include <QMetaEnum>
#include <QWebEnginePage>
#include <QWidget>

#include "settingsmanager.h"

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
};

#endif // PERMISSIONDIALOG_H
