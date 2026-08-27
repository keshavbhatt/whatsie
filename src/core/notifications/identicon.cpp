#include "core/notifications/identicon.h"

#include <QFont>
#include <QPainter>
#include <QPainterPath>

using namespace Qt::StringLiterals;

namespace whatsie::core {

namespace {

// FNV-1a over UTF-16 code units of the case-folded label. Deterministic
// across runs and platforms, unlike qHash().
quint32 stableHash(const QString& text)
{
    quint32 hash = 2166136261U;
    for (const QChar c : text.toCaseFolded()) {
        hash ^= c.unicode();
        hash *= 16777619U;
    }
    return hash;
}

} // namespace

QColor identiconColor(const QString& label)
{
    const quint32 hash = stableHash(label);
    // Spread hues; keep saturation/lightness in a range that reads well with white text.
    const int hue = static_cast<int>(hash % 360U);
    return QColor::fromHsl(hue, 150, 110);
}

QString identiconInitials(const QString& label)
{
    const QStringList words = label.simplified().split(u' ', Qt::SkipEmptyParts);
    QString initials;
    for (const QString& word : words) {
        // First letter or digit of the word ("+49" → '4', "(Bob)" → 'B').
        for (const QChar c : word) {
            if (c.isLetterOrNumber()) {
                initials.append(c.toUpper());
                break;
            }
        }
        if (initials.size() == 2) {
            break;
        }
    }
    return initials.isEmpty() ? u"?"_s : initials;
}

QImage identicon(const QString& label, int size)
{
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(identiconColor(label));
    painter.drawEllipse(QRectF(0, 0, size, size));

    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(static_cast<int>(size * 0.42));
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(0, 0, size, size), Qt::AlignCenter, identiconInitials(label));
    return image;
}

} // namespace whatsie::core
