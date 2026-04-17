#ifndef IDENTICONS_H
#define IDENTICONS_H

#include <QtTypes>
#include <QSize>
#include <QImage>
#include <QPixmap>
#include <QString>

namespace Identicons {
  QPixmap letterTile(const QString & text, const QSize & size);
  QPixmap clipRRect(const QPixmap & pix);
  quint32 colorCount(const QImage & img);
}
#endif // IDENTICONS_H
