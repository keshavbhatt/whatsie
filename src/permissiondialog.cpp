#include "permissiondialog.h"
#include "ui_permissiondialog.h"


PermissionDialog::PermissionDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::PermissionDialog) {
  ui->setupUi(this);

  ui->featuresTableWidget->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->featuresTableWidget->verticalHeader()->setVisible(true);
  ui->featuresTableWidget->horizontalHeader()->setVisible(true);
  ui->featuresTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
  ui->featuresTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->featuresTableWidget->setFocusPolicy(Qt::NoFocus);

  QMetaEnum en = QMetaEnum::fromType<QWebEnginePage::Feature>();
  for (int i = 0; i < en.keyCount(); i++) {
    QWebEnginePage::Feature feature = (QWebEnginePage::Feature)en.value(i);
    QString featureName = QVariant::fromValue(feature).toString();
    addToFeaturesTable(feature, featureName);
  }
}

void PermissionDialog::addToFeaturesTable(QWebEnginePage::Feature feature,
                                          QString &featureName) {
  if (featureName.isEmpty() == false) {
    int nextRow;
    int row = ui->featuresTableWidget->rowCount();
    if (row == 0) {
      nextRow = 0;
    } else {
      nextRow = row++;
    }

    QStringList columnData;
    columnData << featureName << "status";

    // insertRow
    ui->featuresTableWidget->insertRow(nextRow);
    SettingsManager::instance().settings().beginGroup("permissions");
    // add column
    for (int i = 0; i < columnData.count(); i++) {

      if (columnData.at(i) == "status") {
        QCheckBox *featureCheckBox = new QCheckBox(0);
        featureCheckBox->setStyleSheet(
            "border:0px;margin-left:50%; margin-right:50%;");
        featureCheckBox->setChecked(
            SettingsManager::instance().settings().value(featureName, false).toBool());
        connect(featureCheckBox, &QCheckBox::toggled, [=](bool checked) {
          // save permission
          SettingsManager::instance().settings().setValue("permissions/" + featureName, checked);
          emit webPageFeatureChanged(feature);
        });
        ui->featuresTableWidget->setCellWidget(nextRow, i, featureCheckBox);
      } else {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(columnData.at(i));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        ui->featuresTableWidget->setItem(nextRow, i, item);
      }
      this->update();
    }
    SettingsManager::instance().settings().endGroup();
  }
}

void PermissionDialog::keyPressEvent(QKeyEvent *e)
{
   if (e->key() == Qt::Key_Escape)
      this->close();

   QWidget::keyPressEvent(e);
}

PermissionDialog::~PermissionDialog() { delete ui; }
