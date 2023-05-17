#include "scrolltext.h"
#include <QHoverEvent>
#include <QPainter>

ScrollText::ScrollText(QWidget *parent) : QWidget(parent), scrollPos(0) {

  staticText.setTextFormat(Qt::PlainText);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  setMinimumHeight(fontMetrics().height() + 10);
  setMaximumHeight(this->minimumHeight() + 6);

  leftMargin = 0; // height() / 3;

  setSeparator("        ");

  connect(&timer, &QTimer::timeout, this, &ScrollText::timer_timeout);
  timer.setInterval(50);
}

QString ScrollText::text() const { return _text; }

void ScrollText::setText(QString text) {
  _text = text;
  updateText();
  update();
}

QString ScrollText::separator() const { return _separator; }

void ScrollText::setSeparator(QString separator) {
  _separator = separator;
  updateText();
  update();
}

void ScrollText::setLeftMargin(int pixels) {
  leftMargin = pixels;
  update();
}

void ScrollText::pause() {
  if (scrollEnabled) {
    timer.stop();
  }
}

void ScrollText::resume() {
  if (scrollEnabled) {
    timer.start();
    scrolledOnce = false;
  }
}

void ScrollText::updateText() {
  scrolledOnce = false;
  timer.stop();

  singleTextWidth = fontMetrics().horizontalAdvance(_text);
  //    scrollEnabled = true;
  scrollEnabled = (singleTextWidth > width() - leftMargin);

  if (scrollEnabled) {
    scrollPos = -64;
    staticText.setText(_text + _separator);
    timer.start();
  } else {
    staticText.setText(_text);
  }
  staticText.prepare(QTransform(), font());
  // wholeTextSize = QSize(fontMetrics().width(staticText.text()),
  // fontMetrics().height());
  wholeTextSize = QSize(fontMetrics().horizontalAdvance(staticText.text()),
                        fontMetrics().height());
}

void ScrollText::paintEvent(QPaintEvent *) {
  QPainter p(this);

  if (scrollEnabled) {
    buffer.fill(qRgba(0, 0, 0, 0));
    QPainter pb(&buffer);
    pb.setPen(p.pen());
    pb.setFont(p.font());

    int x = qMin(-scrollPos, 0) + leftMargin;
    while (x < width()) {
      pb.drawStaticText(QPointF(x, (height() - wholeTextSize.height()) / 2) +
                            QPoint(2, 2),
                        staticText);
      x += wholeTextSize.width();
    }

    // Apply Alpha Channel
    pb.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    pb.setClipRect(width() - 15, 0, 15, height());
    pb.drawImage(0, 0, alphaChannel);
    pb.setClipRect(0, 0, 15, height());
    // initial situation: don't apply alpha channel in the left half of the
    // image at all; apply it more and more until scrollPos gets positive
    if (scrollPos < 0)
      pb.setOpacity((qreal)(qMax(-8, scrollPos) + 8) / 8.0);
    pb.drawImage(0, 0, alphaChannel);
    p.drawImage(0, 0, buffer);
  } else {
    p.drawText(QRectF(0, 0, width(), height()), Qt::AlignCenter, text());
    //      p.drawStaticText(QPointF(leftMargin, (height() -
    //      wholeTextSize.height()) / 2), staticText);
  }
}

void ScrollText::resizeEvent(QResizeEvent *) {
  // When the widget is resized, we need to update the alpha channel.

  alphaChannel = QImage(size(), QImage::Format_ARGB32_Premultiplied);
  buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);

  // Create Alpha Channel:
  if (width() > 64) {
    // create first scanline
    QRgb *scanline1 = (QRgb *)alphaChannel.scanLine(0);
    for (int x = 1; x < 16; ++x)
      scanline1[x - 1] = scanline1[width() - x] = qRgba(0, 0, 0, x << 4);
    for (int x = 15; x < width() - 15; ++x)
      scanline1[x] = qRgb(0, 0, 0);
    // copy scanline to the other ones
    for (int y = 1; y < height(); ++y)
      memcpy(alphaChannel.scanLine(y), (uchar *)scanline1, width() * 4);
  } else
    alphaChannel.fill(qRgb(0, 0, 0));

  // Update scrolling state
  bool newScrollEnabled = (singleTextWidth > width() - leftMargin);
  if (newScrollEnabled != scrollEnabled)
    updateText();
}

void ScrollText::timer_timeout() {
  scrollPos = (scrollPos + 2) % wholeTextSize.width();
  pauseAfterOneRotation(scrollPos);
  update();
}

void ScrollText::pauseAfterOneRotation(int scrollPos) {
  if (scrolledOnce == false && scrollPos + 2 == wholeTextSize.width()) {
    scrolledOnce = true;
  }
  if (scrolledOnce) {
    pause();
  }
}
