#include "utils.h"
#include <QDateTime>
#include <QRegularExpression>

utils::utils(QObject *parent) : QObject(parent)
{
    setParent(parent);
}

utils::~utils()
{
    this->deleteLater();
}

//calculate dir size
quint64 utils::dir_size(const QString & directory)
{
    quint64 sizex = 0;
    QFileInfo str_info(directory);
    if (str_info.isDir())
    {
        QDir dir(directory);
        QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs |  QDir::Hidden | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
                sizex += dir_size(fileInfo.absoluteFilePath());
            }
            else{
                sizex += fileInfo.size();
            }
        }
    }
    return sizex;
}

//get the size of cache folder in human readble format
QString utils::refreshCacheSize(const QString cache_dir)
{
    qint64 cache_size = dir_size(cache_dir);
    QString cache_unit;
    if(cache_size > 1024*1024*1024)
    {
        cache_size = cache_size/(1024*1024*1024);
        cache_unit = " GB";
    }
    if(cache_size > 1024*1024)
    {
        cache_size = cache_size/(1024*1024);
        cache_unit = " MB";
    }
    else if(cache_size > 1024)
    {
        cache_size = cache_size/(1024);
        cache_unit = " kB";
    }
    else
    {
        cache_unit = " B";
    }
    return QString::number(cache_size) + cache_unit;
}

bool utils::delete_cache(const QString cache_dir)
{
    bool deleted = QDir(cache_dir).removeRecursively();
    QDir(cache_dir).mkpath(cache_dir);
    return deleted;
}

//returns string with first letter capitalized
QString utils::toCamelCase(const QString& s)
{
    QStringList parts = s.split(' ', QString::SkipEmptyParts);
    for (int i = 0; i < parts.size(); ++i)
        parts[i].replace(0, 1, parts[i][0].toUpper());
    return parts.join(" ");
}

QString utils::generateRandomId(int length){

    QString str = QUuid::createUuid().toString();
    str.remove(QRegularExpression("{|}|-"));
    if(str.length()<length){
        while(str.length() != length){
            int required_char = length - str.length();
            str = str + str.append(genRand(required_char));
        }
    }
    if(str.length()>length){
        while(str.length() != length){
            str = str.remove(str.length()-1,1);
        }
    }
    return str;
}

QString utils::genRand(int length)
{
    QDateTime cd = QDateTime::currentDateTime();
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"+QString::number(cd.currentMSecsSinceEpoch()).remove(QRegExp("[^a-zA-Z\\d\\s]")));

    const int randomStringLength = length;
    QString randomString;
    qsrand(cd.toTime_t());
    for(int i=0; i<randomStringLength; ++i)
    {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    randomString = randomString.trimmed().simplified().remove(" ");
    return randomString;
}

QString utils::convertSectoDay(qint64 secs)
{
    int day = secs / (24 * 3600);

    secs = secs % (24 * 3600);
    int hour = secs / 3600;

    secs %= 3600;
    int minutes = secs / 60 ;

    secs %= 60;
    int seconds = secs;

    QString days =  QString::number(day) + " " + "days " + QString::number(hour)
         + " " + "hours " + QString::number(minutes) + " "
         + "minutes " + QString::number(seconds) + " "
         + "seconds ";
    return days;
}

//static on demand path maker
QString utils::returnPath(QString pathname)
{
    QString _data_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if(!QDir(_data_path+"/"+pathname).exists()){
        QDir d(_data_path+"/"+pathname);
        d.mkpath(_data_path+"/"+pathname);
    }
    return _data_path+"/"+pathname+"/";
}


QString utils::htmlToPlainText(QString str){
    QString out;
        QTextDocument text;
        text.setHtml(str);
        out = text.toPlainText();
        text.deleteLater();
    return out .replace("\\\"","'")
            .replace("&amp;","&")
            .replace("&gt;",">")
            .replace("&lt;","<")
            .replace("&#39;","'");
}

