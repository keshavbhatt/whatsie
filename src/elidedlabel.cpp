#include "elidedlabel.h"

#include <QDebug>
#include <QPainter>
#include <QResizeEvent>
#include <QStyle>

ElidedLabel::ElidedLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent, f), m_elide_mode(Qt::ElideRight) {
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
}

ElidedLabel::ElidedLabel(const QString& txt, QWidget* parent, Qt::WindowFlags f)
    : QLabel(txt, parent, f), m_elide_mode(Qt::ElideRight) {
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
}

ElidedLabel::ElidedLabel(const QString& txt, Qt::TextElideMode elideMode, QWidget* parent, Qt::WindowFlags f)
    : QLabel(txt, parent, f), m_elide_mode(elideMode) {
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

}

void ElidedLabel::setText(const QString& txt) {
    QLabel::setText(txt);
    cacheElidedText(geometry().width());
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
}


void ElidedLabel::cacheElidedText(int w) {
    m_cached_elided_text = fontMetrics().elidedText(text(), m_elide_mode, w, (buddy() == nullptr)? 0 : Qt::TextShowMnemonic);
}

void ElidedLabel::resizeEvent(QResizeEvent* e) {
    QLabel::resizeEvent(e);
    cacheElidedText(e->size().width());
}

void ElidedLabel::paintEvent(QPaintEvent* e) {
    if(m_elide_mode == Qt::ElideNone) {
        QLabel::paintEvent(e);
    } else {
        QPainter p(this);
        p.drawText(0, 0, geometry().width(), geometry().height(),
                   QStyle::visualAlignment(text().isRightToLeft()? Qt::RightToLeft : Qt::LeftToRight, alignment()) | ((buddy() == nullptr)? 0 : Qt::TextShowMnemonic),
                   m_cached_elided_text);
    }
}
