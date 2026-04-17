#ifndef DICTIONARIES_H
#define DICTIONARIES_H

#include <QString>
#include <QStringList>

class Dictionaries {
public:
    static QString GetDictionaryPath();
    static QStringList GetDictionaries();
};

#endif // DICTIONARIES_H
