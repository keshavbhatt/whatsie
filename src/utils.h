#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QTextDocument>
#include <QUuid>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QProcess>

class utils : public QObject
{
    Q_OBJECT

public:
    utils(QObject* parent=0);
    virtual ~utils();
public slots:
    static QString refreshCacheSize(const QString cache_dir);
    static bool delete_cache(const QString cache_dir);
    static QString toCamelCase(const QString &s);
    static QString generateRandomId(int length);
    static QString genRand(int length);
    static QString convertSectoDay(qint64 secs);
    static QString returnPath(QString pathname, QString standardLocation);
    static QString EncodeXML ( const QString& encodeMe );
    static QString DecodeXML ( const QString& decodeMe );
    static QString htmlToPlainText(QString str);
    static QString GetEnvironmentVar(const QString &variable_name);
    static float RoundToOneDecimal(float number);
    void DisplayExceptionErrorDialog(const QString &error_info);
    static QString appDebugInfo();
    static void desktopOpenUrl(const QString str);

private slots:
    //use refreshCacheSize
    static quint64 dir_size(const QString &directory);



};

#endif // UTILS_H
