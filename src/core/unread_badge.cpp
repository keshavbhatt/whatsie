#include "core/unread_badge.h"

#include <QFont>
#include <QPainter>
#include <QRegularExpression>

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

} // namespace whatsie::core
