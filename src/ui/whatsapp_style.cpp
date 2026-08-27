#include "ui/whatsapp_style.h"

#include <QHash>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

namespace {

// WhatsApp Web design tokens, per scheme. Kept in one place so the sheet below
// reads as structure, not colour literals.
QHash<QString, QString> tokens(bool dark)
{
    if (dark) {
        return {
            {u"accent"_s, u"#00a884"_s},   {u"accentHover"_s, u"#06cf9c"_s}, {u"accentText"_s, u"#111b21"_s},
            {u"bg"_s, u"#111b21"_s},       {u"panel"_s, u"#202c33"_s},       {u"elevated"_s, u"#233138"_s},
            {u"input"_s, u"#2a3942"_s},    {u"inputBorder"_s, u"#374248"_s}, {u"border"_s, u"#2a3942"_s},
            {u"text"_s, u"#e9edef"_s},     {u"muted"_s, u"#8696a0"_s},       {u"hover"_s, u"#2a3942"_s},
            {u"selected"_s, u"#2a3942"_s},
        };
    }
    return {
        {u"accent"_s, u"#00a884"_s},   {u"accentHover"_s, u"#017561"_s}, {u"accentText"_s, u"#ffffff"_s},
        {u"bg"_s, u"#f0f2f5"_s},       {u"panel"_s, u"#ffffff"_s},       {u"elevated"_s, u"#ffffff"_s},
        {u"input"_s, u"#ffffff"_s},    {u"inputBorder"_s, u"#d1d7db"_s}, {u"border"_s, u"#e9edef"_s},
        {u"text"_s, u"#111b21"_s},     {u"muted"_s, u"#667781"_s},       {u"hover"_s, u"#f0f2f5"_s},
        {u"selected"_s, u"#f0f2f5"_s},
    };
}

// The sheet, with {{token}} placeholders. Backgrounds are set only on the
// containers and controls that need the WhatsApp look; the palette handles the
// rest, so nothing here paints an opaque block over the web view or dialogs.
const char* kSheet = R"qss(
QDialog, QMainWindow { background: {{bg}}; }
QWidget { color: {{text}}; }
QLabel, QCheckBox, QRadioButton, QGroupBox, QTabWidget, QTabBar { background: transparent; }
QScrollArea, QScrollArea > QWidget > QWidget { background: transparent; border: none; }
QToolTip {
    background: {{elevated}}; color: {{text}};
    border: 1px solid {{border}}; border-radius: 6px; padding: 4px 7px;
}

/* Tabs: WhatsApp underlines the active section. */
QTabWidget::pane { border: none; background: transparent; }
QTabBar::tab {
    background: transparent; color: {{muted}};
    padding: 8px 16px 9px 16px; margin-right: 2px;
    border: none; border-bottom: 2px solid transparent;
}
QTabBar::tab:hover { color: {{text}}; }
QTabBar::tab:selected { color: {{accent}}; border-bottom: 2px solid {{accent}}; }

/* Grouping panels. */
QGroupBox {
    background: {{panel}}; border: 1px solid {{border}}; border-radius: 12px;
    margin-top: 16px; padding: 16px 14px 14px 14px; font-weight: 600;
}
QGroupBox::title {
    subcontrol-origin: margin; subcontrol-position: top left;
    left: 14px; padding: 0 4px; color: {{muted}};
}

/* Text inputs, combos and spin boxes share the pill shape. */
QLineEdit, QComboBox, QAbstractSpinBox {
    background: {{input}}; color: {{text}};
    border: 1px solid {{inputBorder}}; border-radius: 9px;
    padding: 6px 10px; min-height: 22px;
    selection-background-color: {{accent}}; selection-color: #ffffff;
}
QLineEdit:hover, QComboBox:hover, QAbstractSpinBox:hover { border: 1px solid {{muted}}; }
QLineEdit:focus, QComboBox:focus, QAbstractSpinBox:focus { border: 1px solid {{accent}}; }
QLineEdit:read-only { color: {{muted}}; }
QComboBox::drop-down, QComboBox::drop-down:editable {
    subcontrol-origin: padding; subcontrol-position: center right;
    width: 24px; border: none; background: transparent;
}
QComboBox::down-arrow { image: url(:/icons/chevron-down.svg); width: 14px; height: 14px; }
QComboBox QAbstractItemView {
    background: {{elevated}}; color: {{text}};
    border: 1px solid {{border}}; border-radius: 10px; padding: 4px;
    outline: none; selection-background-color: {{accent}}; selection-color: #ffffff;
}
QComboBox QAbstractItemView::item { padding: 6px 8px; border-radius: 6px; min-height: 22px; }

QAbstractSpinBox::up-button, QAbstractSpinBox::down-button {
    subcontrol-origin: border; width: 20px; border: none; background: transparent;
}
QAbstractSpinBox::up-button { subcontrol-position: top right; }
QAbstractSpinBox::down-button { subcontrol-position: bottom right; }
QAbstractSpinBox::up-arrow { image: url(:/icons/chevron-up.svg); width: 12px; height: 12px; }
QAbstractSpinBox::down-arrow { image: url(:/icons/chevron-down.svg); width: 12px; height: 12px; }

/* Checkboxes: rounded, green when checked. */
QCheckBox { spacing: 9px; }
QCheckBox::indicator, QRadioButton::indicator {
    width: 18px; height: 18px;
    border: 1.5px solid {{muted}}; border-radius: 5px; background: {{input}};
}
QRadioButton::indicator { border-radius: 9px; }
QCheckBox::indicator:hover, QRadioButton::indicator:hover { border-color: {{accent}}; }
QCheckBox::indicator:checked {
    background: {{accent}}; border-color: {{accent}}; image: url(:/icons/check.svg);
}
QRadioButton::indicator:checked { background: {{accent}}; border-color: {{accent}}; }
QCheckBox:disabled, QLabel:disabled { color: {{muted}}; }
QCheckBox::indicator:disabled { border-color: {{border}}; background: {{panel}}; }

/* Buttons: neutral by default, green for the primary (default) action. */
QPushButton {
    background: {{input}}; color: {{text}};
    border: 1px solid {{inputBorder}}; border-radius: 9px;
    padding: 7px 16px; min-height: 20px; font-weight: 500;
}
QPushButton:hover { background: {{hover}}; border-color: {{muted}}; }
QPushButton:pressed { background: {{selected}}; }
QPushButton:focus { border: 1px solid {{accent}}; }
/* Only buttons explicitly marked primary go green — never the dialog's shifting
   default, which followed tab order and lit up whichever button happened to be
   first (e.g. "Change…" instead of "Close"). */
QPushButton[whatsiePrimary="true"] {
    background: {{accent}}; color: {{accentText}}; border: 1px solid {{accent}};
}
QPushButton[whatsiePrimary="true"]:hover {
    background: {{accentHover}}; border-color: {{accentHover}};
}
QPushButton:disabled { color: {{muted}}; border-color: {{border}}; background: {{panel}}; }

/* Menus (tray + context) as rounded cards. */
QMenu {
    background: {{elevated}}; color: {{text}};
    border: 1px solid {{border}}; border-radius: 10px; padding: 6px;
}
QMenu::item { padding: 7px 22px 7px 14px; border-radius: 6px; }
QMenu::item:selected { background: {{accent}}; color: #ffffff; }
QMenu::separator { height: 1px; background: {{border}}; margin: 5px 8px; }
QMenu::indicator { width: 16px; height: 16px; }

/* Tidy, thin scrollbars. */
QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: {{inputBorder}}; border-radius: 5px; min-height: 28px; }
QScrollBar::handle:vertical:hover { background: {{muted}}; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px; }
QScrollBar::handle:horizontal { background: {{inputBorder}}; border-radius: 5px; min-width: 28px; }
QScrollBar::handle:horizontal:hover { background: {{muted}}; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

/* List/tree selection (Downloads window, permission list). */
QListView, QTreeView, QListWidget {
    background: {{panel}}; border: 1px solid {{border}}; border-radius: 10px; outline: none;
}
QListView::item, QTreeView::item, QListWidget::item { padding: 4px; border-radius: 6px; }
QListView::item:selected, QTreeView::item:selected, QListWidget::item:selected {
    background: {{accent}}; color: #ffffff;
}
)qss";

} // namespace

QString whatsappStyleSheet(bool dark)
{
    QString sheet = QString::fromUtf8(kSheet);
    const QHash<QString, QString> t = tokens(dark);
    for (auto it = t.constBegin(); it != t.constEnd(); ++it) {
        sheet.replace(u"{{%1}}"_s.arg(it.key()), it.value());
    }
    return sheet;
}

} // namespace whatsie::ui
