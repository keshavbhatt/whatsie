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

class Utils : public QObject {
  Q_OBJECT

public:
  Utils(QObject *parent = 0);
  virtual ~Utils();
  static QString getInstallType();
  static QString refreshCacheSize(const QString cache_dir);
  static bool delete_cache(const QString cache_dir);
  static QString toCamelCase(const QString &s);
  static QString generateRandomId(int length);
  static QString convertSectoDay(qint64 secs);
  static QString returnPath(QString pathname, QString standardLocation);
  static QString encodeXML(const QString &encodeMe);
  static QString decodeXML(const QString &decodeMe);
  static QString GetEnvironmentVar(const QString &variable_name);
  static float RoundToOneDecimal(float number);
  static void DisplayExceptionErrorDialog(const QString &error_info);
  static QString appDebugInfo();
  static void desktopOpenUrl(const QString &filePathStr);
  static bool isPhoneNumber(const QString &phoneNumber);
  static QString genRand(int length, bool useUpper = true, bool useLower = true,
                         bool useDigits = true);
  static QString detectDesktopEnvironment();
private slots:
  // use refreshCacheSize
  static quint64 dir_size(const QString &directory);
};

#endif // UTILS_H
