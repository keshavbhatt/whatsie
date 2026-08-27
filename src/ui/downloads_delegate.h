#pragma once

#include <QDateTime>
#include <QFileIconProvider>
#include <QStyledItemDelegate>

namespace whatsie::ui {

/// Draws one download row: file icon, name, status/progress line and hover
/// action buttons (open, show in folder, cancel / remove). Emits requests;
/// the hub performs them.
class DownloadsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadsDelegate)

public:
    enum class Action
    {
        Open,
        Reveal,
        Cancel,
        Remove,
    };

    explicit DownloadsDelegate(QObject* parent = nullptr);
    ~DownloadsDelegate() override = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

    /// "just now", "5 min ago", "yesterday 14:03", "12 Mar 2026" (pure).
    [[nodiscard]] static QString relativeTime(const QDateTime& when, const QDateTime& now);
    /// Status line for an entry (pure).
    [[nodiscard]] static QString statusText(const QModelIndex& index, const QDateTime& now);

Q_SIGNALS:
    void actionRequested(quint64 id, whatsie::ui::DownloadsDelegate::Action action);

private:
    struct Button
    {
        Action action;
        QRect rect;
        QString iconName;
        QString tooltip;
    };
    [[nodiscard]] QList<Button> buttonsFor(const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const;

    QFileIconProvider m_icons;
    QPersistentModelIndex m_hovered;
};

} // namespace whatsie::ui
