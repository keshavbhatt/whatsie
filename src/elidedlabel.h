#pragma once
#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QString>
#include <QFont>

// A label that elides its text when not enough geometry is available to show all of the text.
// Currently only capable of one-line.
class ElidedLabel : public QLabel {
    Q_OBJECT

private:
    Qt::TextElideMode m_elide_mode;
    QString m_cached_elided_text;


public:
    ElidedLabel(QWidget* parent = NULL, Qt::WindowFlags f = 0);
    ElidedLabel(const QString& txt, QWidget* parent = NULL, Qt::WindowFlags f = 0);
    ElidedLabel(const QString& txt, Qt::TextElideMode elideMode = Qt::ElideRight, QWidget* parent = NULL, Qt::WindowFlags f = 0);

public:
    // Set the elide mode used for displaying text.
    inline void setElideMode(Qt::TextElideMode elideMode) {
        m_elide_mode = elideMode;
        updateGeometry();
    }

    // Get the elide mode currently used to display text.
    inline Qt::TextElideMode elideMode() const {
        return m_elide_mode;
    }



public: // QLabel overrides
    void setText(const QString&); // note: not virtual so no polymorphism ...


protected: // QLabel overrides
    virtual void paintEvent(QPaintEvent*) override;
    virtual void resizeEvent(QResizeEvent*) override;

protected:
    // Cache the elided text so as to not recompute it every paint event
    void cacheElidedText(int w);

};
