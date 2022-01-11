#include "canvas.h"

#include <QPainter>
#include <iostream>

namespace mandelbrot {

Canvas::Canvas(QWidget* parent) : QWidget(parent), painter(), current_image() {
  QObject::connect(&painter, &Painter::sendNewImage, this,
                   &Canvas::receiveNewImage);
  painter.start();
}

void Canvas::paintEvent(QPaintEvent* event) {
  painter.setImageSize(this->size());
  QPainter{this}.drawImage(0, 0, current_image);
}

void Canvas::keyPressEvent(QKeyEvent* event) {
  constexpr auto DOTS_PER_MOVE = 20;
  switch (event->key()) {
    case Qt::Key_Home:
      painter.reset(this->size());
      break;
    case Qt::Key_Down:
      painter.changeCenterPositionBy({0, DOTS_PER_MOVE});
      break;
    case Qt::Key_Up:
      painter.changeCenterPositionBy({0, -DOTS_PER_MOVE});
      break;
    case Qt::Key_Left:
      painter.changeCenterPositionBy({-DOTS_PER_MOVE, 0});
      break;
    case Qt::Key_Right:
      painter.changeCenterPositionBy({DOTS_PER_MOVE, 0});
      break;
    case Qt::Key_Minus:
      // todo zoom out
    case Qt::Key_Plus:
    case Qt::Key_Equal:
      // todo zoom in
    default:
      QWidget::keyPressEvent(event);
      return;
  }
}

void Canvas::receiveNewImage(QImage image) {
  current_image = std::move(image);
  this->update();
}

}  // namespace mandelbrot
