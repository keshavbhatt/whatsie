#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebEngineView>
#include <QSettings>

class WebView: public QWebEngineView
{
    Q_OBJECT

public:
    WebView(QWidget *parent = nullptr, QStringList dictionaries = {});

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QStringList m_dictionaries;
    QSettings settings;
};

#endif // WEBVIEW_H
