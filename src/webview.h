#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QSettings>
#include <QWebEngineView>
#include <QKeyEvent>

class WebView : public QWebEngineView {
  Q_OBJECT

public:
  WebView(QWidget *parent = nullptr, QStringList dictionaries = {});

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;

protected slots:
  void wheelEvent(QWheelEvent *event);

private:
  QStringList m_dictionaries;
  QSettings settings;
};

#endif // WEBVIEW_H
