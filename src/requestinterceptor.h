#ifndef REQUESTINTERCEPTOR_H
#define REQUESTINTERCEPTOR_H

#include <QApplication>
#include <QDebug>
#include <QObject>
#include <QWebEnginePage>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineView>

#include "settingsmanager.h"

class RequestInterceptor : public QWebEngineUrlRequestInterceptor {
  Q_OBJECT

signals:
  void blocked(QString adUrl);

public:
  RequestInterceptor(QObject *parent = nullptr)
      : QWebEngineUrlRequestInterceptor(parent) {}

  void interceptRequest(QWebEngineUrlRequestInfo &info) {
    QString reqUrlStr = info.requestUrl().toString();

    //        if(reqUrlStr.contains("mms-type=video")||
    //        reqUrlStr.contains("stream/video?key")){
    //            if(SettingsManager::instance().settings().value("disableVideos",false).toBool()){
    //                info.block(true);
    //                qDebug()<<"INTERCEPTOR: Blocked video - "<<reqUrlStr;
    //            }
    //        }else{
    //            qDebug()<<"INTERCEPTOR:"<<reqUrlStr;
    //        }
    qDebug() << "INTERCEPTOR:" << reqUrlStr;
  }
};
#endif // REQUESTINTERCEPTOR_H
