#include "platform/linux/dbus_image.h"

#include <QDBusMetaType>

#include <cstring>

namespace whatsie::platform::linux_ {

DBusImage toDBusImage(const QImage& source)
{
    // Non-premultiplied RGBA in memory order R,G,B,A regardless of endianness.
    const QImage image = source.convertToFormat(QImage::Format_RGBA8888);
    DBusImage out;
    out.width = image.width();
    out.height = image.height();
    out.rowstride = image.width() * 4;
    out.hasAlpha = true;
    out.bitsPerSample = 8;
    out.channels = 4;
    out.data.reserve(out.rowstride * out.height);
    for (int y = 0; y < image.height(); ++y) {
        // bytesPerLine() may include padding; copy exactly one row of pixels.
        out.data.append(reinterpret_cast<const char*>(image.constScanLine(y)), out.rowstride);
    }
    return out;
}

QImage fromDBusImage(const DBusImage& image)
{
    if (image.width <= 0 || image.height <= 0 || image.channels != 4 || image.bitsPerSample != 8) {
        return {};
    }
    QImage out(image.width, image.height, QImage::Format_RGBA8888);
    for (int y = 0; y < image.height; ++y) {
        const qsizetype offset = static_cast<qsizetype>(y) * image.rowstride;
        if (offset + image.width * 4 > image.data.size()) {
            return {};
        }
        std::memcpy(out.scanLine(y), image.data.constData() + offset, static_cast<size_t>(image.width) * 4);
    }
    return out;
}

void registerDBusImageType()
{
    static const bool registered = [] {
        qDBusRegisterMetaType<DBusImage>();
        return true;
    }();
    Q_UNUSED(registered)
}

QDBusArgument& operator<<(QDBusArgument& argument, const DBusImage& image)
{
    argument.beginStructure();
    argument << image.width << image.height << image.rowstride << image.hasAlpha << image.bitsPerSample
             << image.channels << image.data;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, DBusImage& image)
{
    argument.beginStructure();
    argument >> image.width >> image.height >> image.rowstride >> image.hasAlpha >> image.bitsPerSample >>
        image.channels >> image.data;
    argument.endStructure();
    return argument;
}

} // namespace whatsie::platform::linux_
