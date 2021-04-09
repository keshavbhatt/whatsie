#ifndef DICTIONARIES_H
#define DICTIONARIES_H

#include <QString>
#include <QStringList>
#include <QObject>


class Dictionaries : public QObject
{
    Q_OBJECT

public:
    Dictionaries(QObject* parent=0);
    virtual ~Dictionaries();
public slots:
    static QString GetDictionaryPath();
    static QStringList GetDictionaries();
};

#endif // DICTIONARIES_H
