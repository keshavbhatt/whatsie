#ifndef UTILS_H
#define UTILS_H

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextDocument>
#include <QUuid>

class utils : public QObject {
  Q_OBJECT

public:
  utils(QObject *parent = 0);
  virtual ~utils();
public slots:
  static QString refreshCacheSize(const QString cache_dir);
  static bool delete_cache(const QString cache_dir);
  static QString toCamelCase(const QString &s);
  static QString generateRandomId(int length);
  static QString genRand(int length);
  static QString convertSectoDay(qint64 secs);
  static QString returnPath(QString pathname, QString standardLocation);
  static QString EncodeXML(const QString &encodeMe);
  static QString DecodeXML(const QString &decodeMe);
  static QString htmlToPlainText(QString str);
  static QString GetEnvironmentVar(const QString &variable_name);
  static float RoundToOneDecimal(float number);
  static void DisplayExceptionErrorDialog(const QString &error_info);
  static QString appDebugInfo();
  static void desktopOpenUrl(const QString str);

private slots:
  // use refreshCacheSize
  static quint64 dir_size(const QString &directory);
};

#endif // UTILS_H
