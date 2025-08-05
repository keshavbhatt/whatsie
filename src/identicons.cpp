#include "identicons.h"

#include <QPainter>
#include <QPainterPath>
#include <QSet>


namespace Identicons {
    static const QRgb letterTileColors[] = {
        qRgb(214,97,97), // default color
        qRgb(201,0,0),     qRgb(206,59,0),
        qRgb(240,107,0),   qRgb(255,142,0),
        qRgb(130,224,13),  qRgb(192,196,72),
        qRgb(194,167,34),  qRgb(202,123,0),
        qRgb(116,193,61),  qRgb(40,183,36),
        qRgb(82,167,16),   qRgb(0,147,77),
        qRgb(0,185,103),   qRgb(84,188,147),
        qRgb(0,149,168),   qRgb(106,179,185),
        qRgb(68,170,231),  qRgb(45,141,205),
        qRgb(69,116,236),  qRgb(59,113,209),
        qRgb(145,33,203),  qRgb(132,0,179),
        qRgb(120,46,204),  qRgb(68,19,185),
        qRgb(182,33,203),  qRgb(170,0,204),
        qRgb(171,0,182),   qRgb(255,32,51),
        qRgb(162,145,149), qRgb(165,119,127),
        qRgb(201,73,121),  qRgb(239,0,90),
        qRgb(68,68,68),    qRgb(54,54,54),
    };

    // LetterTile Identicon generator taken from
    // https://github.com/GermainZ/Identiconizer/blob/.../identicons/LetterTile.java
    QPixmap letterTile(const QString & text, const QSize & size) {
        if (text.length() == 0) return QPixmap();
        QString s;
        QStringList l = text.split(" ", Qt::SkipEmptyParts);
        s += l.first().front();
        if(l.length() > 3)
            s += l.at(l.length() / 2).front();
        if(l.length() > 1)
            s += l.last().front();
        s = s.toUpper();

        qint64 idx = qHash(text) - 16;
        idx = ((idx < 0)?-idx:idx) % 35;

        QFont sans("sans-serif");
        sans.setPixelSize((69 * std::min(size.height(),size.width()) / 100) / 2);

        QPixmap px(size);
        px.fill(letterTileColors[idx]);

        QPainter pnt(&px);

        pnt.setFont(sans);
        pnt.setRenderHint(QPainter::TextAntialiasing, true);
        QRect rtxt = pnt.fontMetrics().boundingRect(s);
        pnt.setPen(Qt::white);
        pnt.drawText(px.rect().center() - rtxt.center(), s);

        return px;
    }

    quint32 colorCount(const QImage & img) {
        QImage sc = img.scaled(32, 32).convertToFormat(QImage::Format_ARGB32);

        QSet<QRgb> list;
        for (int i = 0; i < sc.width(); i++)
            for(int j = 0; j < sc.height(); j++)
                list.insert(sc.pixel(i, j));
        return list.size();
    }

    QPixmap clipRRect(const QPixmap & pix) {
        QPixmap tpx(pix.size());
        tpx.fill(Qt::transparent);

        QPainterPath pth;
        pth.addRoundedRect(pix.rect(), pix.size().width() / 7, pix.size().height() / 7);

        QPainter pnt(&tpx);
        pnt.setRenderHint(QPainter::Antialiasing, true);
        pnt.setClipPath(pth);
        pnt.drawPixmap(QPoint(), pix);
        return tpx.copy();
    }
}
