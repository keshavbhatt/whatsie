#include "webview.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QWebEngineContextMenuData>
#include <QWebEngineProfile>
#include <mainwindow.h>

WebView::WebView(QWidget *parent, QStringList dictionaries)
    : QWebEngineView(parent) {
  m_dictionaries = dictionaries;

  QObject *parentMainWindow = this->parent();
  while (!parentMainWindow->objectName().contains("MainWindow")) {
    parentMainWindow = parentMainWindow->parent();
  }
  MainWindow *mainWindow = dynamic_cast<MainWindow *>(parentMainWindow);

  connect(this, &WebView::titleChanged, mainWindow,
          &MainWindow::handleWebViewTitleChanged);
  connect(this, &WebView::loadFinished, mainWindow,
          &MainWindow::handleLoadFinished);
  connect(this, &WebView::renderProcessTerminated,
          [this](QWebEnginePage::RenderProcessTerminationStatus termStatus,
                 int statusCode) {
            QString status;
            switch (termStatus) {
            case QWebEnginePage::NormalTerminationStatus:
              status = tr("Render process normal exit");
              break;
            case QWebEnginePage::AbnormalTerminationStatus:
              status = tr("Render process abnormal exit");
              break;
            case QWebEnginePage::CrashedTerminationStatus:
              status = tr("Render process crashed");
              break;
            case QWebEnginePage::KilledTerminationStatus:
              status = tr("Render process killed");
              break;
            }
            QMessageBox::StandardButton btn =
                QMessageBox::question(window(), status,
                                      tr("Render process exited with code: %1\n"
                                         "Do you want to reload the page ?")
                                          .arg(statusCode));
            if (btn == QMessageBox::Yes)
              QTimer::singleShot(0, this, [this] { this->reload(); });
          });
}

void WebView::wheelEvent(QWheelEvent *event) {
  if (event->modifiers().testFlag(Qt::ControlModifier)) {
    qDebug() << "skipped ctrl + m_wheel event on webengineview";
    // do nothing
  } else {
    qDebug() << "wheel event on webengine view";
    QWebEngineView::wheelEvent(event);
  }
}

void WebView::contextMenuEvent(QContextMenuEvent *event) {

  QMenu *menu = page()->createStandardContextMenu();
  menu->setAttribute(Qt::WA_DeleteOnClose, true);
  // hide reload, back, forward, savepage, copyimagelink menus
  foreach (auto *action, menu->actions()) {
    if (action == page()->action(QWebEnginePage::SavePage) ||
        action == page()->action(QWebEnginePage::Reload) ||
        action == page()->action(QWebEnginePage::Back) ||
        action == page()->action(QWebEnginePage::Forward) ||
        action == page()->action(QWebEnginePage::CopyImageUrlToClipboard)) {
      action->setVisible(false);
    }
  }

  const QWebEngineContextMenuData &data = page()->contextMenuData();
  Q_ASSERT(data.isValid());

  // allow context menu on image
  if (data.mediaType() == QWebEngineContextMenuData::MediaTypeImage) {
    QWebEngineView::contextMenuEvent(event);
    return;
  }
  // if content is not editable
  if (data.selectedText().isEmpty() && !data.isContentEditable()) {
    event->ignore();
    return;
  }

  QWebEngineProfile *profile = page()->profile();
  const QStringList &languages = profile->spellCheckLanguages();

  menu->addSeparator();

  QAction *spellcheckAction = new QAction(tr("Check Spelling"), menu);
  spellcheckAction->setCheckable(true);
  spellcheckAction->setChecked(profile->isSpellCheckEnabled());
  connect(spellcheckAction, &QAction::toggled, this,
          [profile, this](bool toogled) {
            profile->setSpellCheckEnabled(toogled);
            settings.setValue("sc_enabled", toogled);
          });
  menu->addAction(spellcheckAction);

  if (profile->isSpellCheckEnabled()) {
    QMenu *subMenu = menu->addMenu(tr("Select Language"));
    for (const QString &dict : qAsConst(m_dictionaries)) {
      QAction *action = subMenu->addAction(dict);
      action->setCheckable(true);
      action->setChecked(languages.contains(dict));
      connect(action, &QAction::triggered, this, [profile, dict, this]() {
        profile->setSpellCheckLanguages(QStringList() << dict);
        settings.setValue("sc_dict", dict);
      });
    }
  }
  connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
  menu->popup(event->globalPos());
}
