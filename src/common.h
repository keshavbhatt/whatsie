#ifndef COMMON_H
#define COMMON_H
#include <QString>
#include <QIcon>

// userAgent
extern QString defaultUserAgentStr;

// appAutoLock
extern int defaultAppAutoLockDuration;
extern bool defaultAppAutoLock;
extern double defaultZoomFactorMaximized;

QIcon themeIcon(const QString& name, const QString& fallback);


#endif // COMMON_H

