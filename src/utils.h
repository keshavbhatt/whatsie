#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QTextDocument>
#include <QUuid>

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
    static QString returnPath(QString pathname);

    static QString EncodeXML ( const QString& encodeMe ){

        QString temp;
        int length = encodeMe.size();
        for (int index = 0; index < length; index++)
        {
            QChar character(encodeMe.at(index));

            switch (character.unicode())
            {
            case '&':
                temp += "&amp;"; break;

            case '\'':
                temp += "&apos;"; break;

            case '"':
                temp += "&quot;"; break;

            case '<':
                temp += "&lt;"; break;

            case '>':
                temp += "&gt;"; break;

            default:
                temp += character;
                break;
            }
        }

        return temp;
    }

    static QString DecodeXML ( const QString& decodeMe ) {

        QString temp(decodeMe);

        temp.replace("&amp;", "&");
        temp.replace("&apos;", "'");
        temp.replace("&quot;", "\"");
        temp.replace("&lt;", "<");
        temp.replace("&gt;", ">");

        return temp;
    }

    static QString htmlToPlainText(QString str);

private slots:
    //use refreshCacheSize
    static quint64 dir_size(const QString &directory);



};

#endif // UTILS_H
