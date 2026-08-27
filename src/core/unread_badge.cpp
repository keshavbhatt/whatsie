#include "core/unread_badge.h"

#include <QColor>
#include <QFont>
#include <QPainter>
#include <QRegularExpression>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace whatsie::core {

int unreadCountFromTitle(const QString& title)
{
    static const QRegularExpression kPattern(u"^\\s*\\((\\d+)\\+?\\)"_s);
    const QRegularExpressionMatch match = kPattern.match(title);
    if (!match.hasMatch()) {
        return 0;
    }
    bool ok = false;
    const int value = match.captured(1).toInt(&ok);
    return ok && value > 0 ? value : 0;
}

QImage composeUnreadBadge(const QImage& base, int count)
{
    if (count <= 0 || base.isNull()) {
        return base;
    }

    QImage out = base.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const int size = std::min(out.width(), out.height());
    const QString label = count > 99 ? u"99+"_s : QString::number(count);

    // Bubble occupies ~55 % of the icon; wider for 2-3 digit labels.
    const double diameter = size * 0.55;
    const double extraDigits = static_cast<double>(label.size() - 1);
    const double width = diameter * (1.0 + (0.28 * extraDigits));
    const QRectF bubble(out.width() - width, out.height() - diameter, width, diameter);

    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0xE5, 0x39, 0x35)); // material red 600
    painter.drawRoundedRect(bubble, diameter / 2.0, diameter / 2.0);

    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(static_cast<int>(diameter * 0.68));
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(bubble, Qt::AlignCenter, label);

    return out;
}

QImage dimImage(const QImage& image, qreal amount)
{
    if (image.isNull() || amount <= 0.0) {
        return image;
    }
    const qreal a = std::clamp(amount, 0.0, 1.0);
    QImage out = image.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < out.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < out.width(); ++x) {
            const QRgb p = line[x];
            const int alpha = qAlpha(p);
            if (alpha == 0) {
                continue;
            }
            const int grey = qGray(p);
            const int r = static_cast<int>(qRed(p) * (1.0 - a) + grey * a);
            const int g = static_cast<int>(qGreen(p) * (1.0 - a) + grey * a);
            const int b = static_cast<int>(qBlue(p) * (1.0 - a) + grey * a);
            line[x] = qRgba(r, g, b, static_cast<int>(alpha * (1.0 - 0.35 * a)));
        }
    }
    return out;
}

QImage tintImage(const QImage& image, const QColor& color)
{
    if (image.isNull()) {
        return image;
    }
    QImage out(image.size(), QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::transparent);
    QPainter painter(&out);
    painter.drawImage(0, 0, image);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(out.rect(), color);
    painter.end();
    return out;
}

} // namespace whatsie::core
