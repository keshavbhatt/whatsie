#include "ui/downloads_delegate.h"

#include "core/downloads/download_model.h"
#include "core/downloads/file_naming.h"

#include <QApplication>
#include <QFileInfo>
#include <QHelpEvent>
#include <QMimeDatabase>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionProgressBar>
#include <QToolTip>

using namespace Qt::StringLiterals;
using whatsie::core::DownloadModel;
using whatsie::core::DownloadState;

namespace whatsie::ui {

namespace {
constexpr int kPadding = 10;
constexpr int kIconSize = 40;
constexpr int kButtonSize = 28;
constexpr int kButtonGap = 6;
constexpr int kProgressHeight = 6;

QIcon iconFor(const QModelIndex& index, const QFileIconProvider& provider)
{
    const QString path = index.data(DownloadModel::FilePathRole).toString();
    const QFileInfo info(path);
    if (info.exists()) {
        const QIcon icon = provider.icon(info);
        if (!icon.isNull()) {
            return icon;
        }
    }
    const QMimeDatabase db;
    const QString mime = index.data(DownloadModel::MimeTypeRole).toString();
    const QMimeType type =
        mime.isEmpty() ? db.mimeTypeForFile(path, QMimeDatabase::MatchExtension) : db.mimeTypeForName(mime);
    QIcon icon = QIcon::fromTheme(type.iconName(), QIcon::fromTheme(type.genericIconName()));
    if (icon.isNull()) {
        icon = provider.icon(QFileIconProvider::File);
    }
    return icon;
}
} // namespace

DownloadsDelegate::DownloadsDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{}

QString DownloadsDelegate::relativeTime(const QDateTime& when, const QDateTime& now)
{
    if (!when.isValid()) {
        return {};
    }
    const qint64 secs = when.secsTo(now);
    if (secs < 60) {
        return tr("just now");
    }
    if (secs < 3600) {
        return tr("%n min ago", nullptr, static_cast<int>(secs / 60));
    }
    if (secs < 6 * 3600) {
        return tr("%n h ago", nullptr, static_cast<int>(secs / 3600));
    }
    if (when.date() == now.date()) {
        return tr("today %1").arg(when.toString(u"HH:mm"_s));
    }
    if (when.date() == now.date().addDays(-1)) {
        return tr("yesterday %1").arg(when.toString(u"HH:mm"_s));
    }
    return when.toString(u"d MMM yyyy"_s);
}

QString DownloadsDelegate::statusText(const QModelIndex& index, const QDateTime& now)
{
    const auto state = index.data(DownloadModel::StateRole).value<DownloadState>();
    const qint64 received = index.data(DownloadModel::ReceivedBytesRole).toLongLong();
    const qint64 total = index.data(DownloadModel::TotalBytesRole).toLongLong();
    const QDateTime finished = index.data(DownloadModel::FinishedAtRole).toDateTime();
    switch (state) {
    case DownloadState::InProgress: {
        const double speed = index.data(DownloadModel::SpeedRole).toDouble();
        QString text = total > 0 ? tr("%1 of %2").arg(core::humanSize(received), core::humanSize(total))
                                 : core::humanSize(received);
        if (speed > 0) {
            text += u" — "_s + tr("%1/s").arg(core::humanSize(static_cast<qint64>(speed)));
        }
        return text;
    }
    case DownloadState::Completed:
        return u"%1 · %2"_s.arg(core::humanSize(total >= 0 ? total : received), relativeTime(finished, now));
    case DownloadState::Cancelled:
        return tr("Cancelled") + u" · "_s + relativeTime(finished, now);
    case DownloadState::Failed: {
        const QString error = index.data(DownloadModel::ErrorRole).toString();
        return (error.isEmpty() ? tr("Failed") : tr("Failed: %1").arg(error)) + u" · "_s +
               relativeTime(finished, now);
    }
    }
    return {};
}

QSize DownloadsDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex&) const
{
    const int textHeight = option.fontMetrics.height() * 2 + 6 + kProgressHeight;
    return {option.rect.width(), std::max(kIconSize, textHeight) + 2 * kPadding};
}

QList<DownloadsDelegate::Button> DownloadsDelegate::buttonsFor(const QStyleOptionViewItem& option,
                                                               const QModelIndex& index) const
{
    const auto state = index.data(DownloadModel::StateRole).value<DownloadState>();
    QList<Button> buttons;
    if (state == DownloadState::InProgress) {
        buttons.append({Action::Cancel, {}, u"process-stop"_s, tr("Cancel")});
    } else {
        if (state == DownloadState::Completed) {
            buttons.append({Action::Open, {}, u"document-open"_s, tr("Open")});
            buttons.append({Action::Reveal, {}, u"folder-open"_s, tr("Show in folder")});
        }
        buttons.append({Action::Remove, {}, u"edit-delete"_s, tr("Remove from list")});
    }
    int x = option.rect.right() - kPadding - kButtonSize;
    const int y = option.rect.center().y() - kButtonSize / 2;
    for (auto it = buttons.rbegin(); it != buttons.rend(); ++it) {
        it->rect = QRect(x, y, kButtonSize, kButtonSize);
        x -= kButtonSize + kButtonGap;
    }
    return buttons;
}

void DownloadsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text.clear();
    opt.icon = QIcon();
    QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    const QRect r = opt.rect.adjusted(kPadding, kPadding, -kPadding, -kPadding);
    const bool hovered = (opt.state & QStyle::State_MouseOver) != 0;
    const bool selected = (opt.state & QStyle::State_Selected) != 0;
    const QPalette& pal = opt.palette;
    const QColor textColor = selected ? pal.color(QPalette::HighlightedText) : pal.color(QPalette::Text);
    const QColor mutedColor =
        selected ? pal.color(QPalette::HighlightedText) : pal.color(QPalette::PlaceholderText);

    // Icon
    const QRect iconRect(r.left(), r.center().y() - kIconSize / 2, kIconSize, kIconSize);
    iconFor(index, m_icons).paint(painter, iconRect);

    // Buttons (only on hover/selection) reserve space on the right.
    const QList<Button> buttons = buttonsFor(opt, index);
    int textRight = r.right();
    if (hovered || selected) {
        for (const Button& b : buttons) {
            QStyleOptionToolButton tb;
            tb.rect = b.rect;
            tb.state = QStyle::State_Enabled | QStyle::State_AutoRaise | QStyle::State_Raised;
            tb.palette = pal;
            painter->setPen(Qt::NoPen);
            painter->setBrush(selected ? pal.color(QPalette::Highlight).lighter(120)
                                       : pal.color(QPalette::Button));
            painter->drawRoundedRect(b.rect, 6, 6);
            QIcon::fromTheme(b.iconName).paint(painter, b.rect.adjusted(6, 6, -6, -6));
        }
        if (!buttons.isEmpty()) {
            textRight = buttons.first().rect.left() - kPadding;
        }
    }

    // Text block
    const int textLeft = iconRect.right() + kPadding;
    const int lineHeight = opt.fontMetrics.height();
    const QRect textRect(textLeft, r.top(), textRight - textLeft, r.height());
    QFont bold = opt.font;
    bold.setBold(true);
    painter->setFont(bold);
    painter->setPen(textColor);
    const QString name = opt.fontMetrics.elidedText(index.data(DownloadModel::FileNameRole).toString(),
                                                    Qt::ElideMiddle, textRect.width());
    painter->drawText(QRect(textRect.left(), textRect.top(), textRect.width(), lineHeight),
                      Qt::AlignLeft | Qt::AlignVCenter, name);

    painter->setFont(opt.font);
    painter->setPen(mutedColor);
    const QString status = opt.fontMetrics.elidedText(statusText(index, QDateTime::currentDateTime()),
                                                      Qt::ElideRight, textRect.width());
    painter->drawText(QRect(textRect.left(), textRect.top() + lineHeight + 2, textRect.width(), lineHeight),
                      Qt::AlignLeft | Qt::AlignVCenter, status);

    // Progress bar for active downloads
    const auto state = index.data(DownloadModel::StateRole).value<DownloadState>();
    if (state == DownloadState::InProgress) {
        const qint64 received = index.data(DownloadModel::ReceivedBytesRole).toLongLong();
        const qint64 total = index.data(DownloadModel::TotalBytesRole).toLongLong();
        QStyleOptionProgressBar bar;
        bar.rect =
            QRect(textRect.left(), textRect.top() + 2 * lineHeight + 6, textRect.width(), kProgressHeight);
        bar.minimum = 0;
        bar.maximum = total > 0 ? 1000 : 0; // 0/0 = busy indicator
        bar.progress = total > 0 ? static_cast<int>(received * 1000 / total) : 0;
        bar.textVisible = false;
        bar.palette = pal;
        style->drawControl(QStyle::CE_ProgressBar, &bar, painter, opt.widget);
    }
    painter->restore();
}

bool DownloadsDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() == Qt::LeftButton) {
            for (const Button& b : buttonsFor(option, index)) {
                if (b.rect.contains(mouse->pos())) {
                    Q_EMIT actionRequested(index.data(DownloadModel::IdRole).toULongLong(), b.action);
                    return true;
                }
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

} // namespace whatsie::ui
