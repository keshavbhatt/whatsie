#include "web/clipboard_fix.h"

#include "web/logging.h"

#include <QBuffer>
#include <QClipboard>
#include <QImage>
#include <QMimeData>

using namespace Qt::StringLiterals;

namespace whatsie::web {

bool needsPngRepresentation(const QMimeData& mime)
{
    return mime.hasImage() && !mime.hasFormat(u"image/png"_s);
}

bool ensureClipboardImageIsPng(QClipboard& clipboard)
{
    const QMimeData* current = clipboard.mimeData();
    if (current == nullptr || !needsPngRepresentation(*current)) {
        return false;
    }
    const QImage image = qvariant_cast<QImage>(current->imageData());
    if (image.isNull()) {
        return false;
    }
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "PNG")) {
        return false;
    }
    auto* rewritten = new QMimeData;
    rewritten->setImageData(image);
    rewritten->setData(u"image/png"_s, png);
    if (current->hasText()) {
        rewritten->setText(current->text());
    }
    clipboard.setMimeData(rewritten); // clipboard takes ownership
    qCDebug(lcWeb) << "clipboard image re-exported as PNG for paste";
    return true;
}

} // namespace whatsie::web
