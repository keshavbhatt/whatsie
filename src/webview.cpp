#include "webview.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QWebEngineProfile>
#include <QWebEngineContextMenuData>

WebView::WebView(QWidget *parent, QStringList dictionaries)
    : QWebEngineView(parent)
{
    m_dictionaries = dictionaries;
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    const QWebEngineContextMenuData &data = page()->contextMenuData();
    Q_ASSERT(data.isValid());

    //allow context menu on image
    if (data.mediaType() == QWebEngineContextMenuData::MediaTypeImage) {
        QWebEngineView::contextMenuEvent(event);
        return;
    }
    // if content is not editable
    if (!data.isContentEditable()) {
        event->ignore();
        return;
    }

    QWebEngineProfile *profile = page()->profile();
    const QStringList &languages = profile->spellCheckLanguages();
    QMenu *menu = page()->createStandardContextMenu();
    menu->addSeparator();

    QAction *spellcheckAction = new QAction(tr("Check Spelling"), nullptr);
    spellcheckAction->setCheckable(true);
    spellcheckAction->setChecked(profile->isSpellCheckEnabled());
    connect(spellcheckAction, &QAction::toggled, this, [profile](bool toogled) {
        profile->setSpellCheckEnabled(toogled);
    });
    menu->addAction(spellcheckAction);

    if (profile->isSpellCheckEnabled()) {
        QMenu *subMenu = menu->addMenu(tr("Select Language"));
        for (const QString &dict : m_dictionaries) {
            QAction *action = subMenu->addAction(dict);
            action->setCheckable(true);
            action->setChecked(languages.contains(dict));
            connect(action, &QAction::triggered, this, [profile, dict,this](){
                profile->setSpellCheckLanguages(QStringList()<<dict);
                settings.setValue("sc_dict",dict);
            });
        }
    }
    connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
    menu->popup(event->globalPos());
}
