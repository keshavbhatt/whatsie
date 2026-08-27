#pragma once

#include <QByteArray>
#include <QDBusArgument>
#include <QImage>
#include <QMetaType>

// The freedesktop "image-data" hint: (iiibiiay) = width, height, rowstride,
// has_alpha, bits_per_sample, channels, data. Data must be RGB(A) byte order
// — Qt's native ARGB32 is BGRA on little-endian, which produced the inverted
// avatars of W#312 / W#328.
namespace whatsie::platform::linux_ {

struct DBusImage
{
    int width = 0;
    int height = 0;
    int rowstride = 0;
    bool hasAlpha = false;
    int bitsPerSample = 8;
    int channels = 4;
    QByteArray data;
};

[[nodiscard]] DBusImage toDBusImage(const QImage& image);
[[nodiscard]] QImage fromDBusImage(const DBusImage& image);
void registerDBusImageType();

QDBusArgument& operator<<(QDBusArgument& argument, const DBusImage& image);
const QDBusArgument& operator>>(const QDBusArgument& argument, DBusImage& image);

} // namespace whatsie::platform::linux_

Q_DECLARE_METATYPE(whatsie::platform::linux_::DBusImage)
