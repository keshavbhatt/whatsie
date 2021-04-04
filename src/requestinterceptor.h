#ifndef REQUESTINTERCEPTOR_H
#define REQUESTINTERCEPTOR_H
#include <QWebEnginePage>
#include <QDebug>
#include <QObject>
#include <QWebEngineView>
#include <QApplication>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>
#include <QSettings>

class RequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

signals:
    void blocked(QString adUrl);
private:
    QSettings m_settings;

public:
    RequestInterceptor(QObject *parent = nullptr) : QWebEngineUrlRequestInterceptor(parent)
    {
    }

    void interceptRequest(QWebEngineUrlRequestInfo &info)
    {
        QString reqUrlStr = info.requestUrl().toString();

//        if(reqUrlStr.contains("mms-type=video")|| reqUrlStr.contains("stream/video?key")){
//            if(m_settings.value("disableVideos",false).toBool()){
//                info.block(true);
//                qDebug()<<"INTERCEPTOR: Blocked video - "<<reqUrlStr;
//            }
//        }else{
//            qDebug()<<"INTERCEPTOR:"<<reqUrlStr;
//        }
        qDebug()<<"INTERCEPTOR:"<<reqUrlStr;
    }
};
#endif // REQUESTINTERCEPTOR_H
