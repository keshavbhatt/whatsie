#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QKeyEvent>
#include <QWebEngineView>

#include "settingsmanager.h"

class WebView : public QWebEngineView {
  Q_OBJECT

public:
  WebView(QWidget *parent = nullptr, QStringList dictionaries = {});

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  QStringList m_dictionaries;
};

#endif // WEBVIEW_H
