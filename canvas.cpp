#include "canvas.h"

#include <QPainter>

namespace mandelbrot {

Canvas::Canvas(QWidget* parent) : QWidget(parent), painter(), current_image() {
  QObject::connect(this, &Canvas::request_new_image, &painter,
                   &Painter::new_image_requested);
  QObject::connect(&painter, &Painter::send_new_image, this,
                   &Canvas::receive_new_image);
}

void Canvas::paintEvent(QPaintEvent* event) {
  emit request_new_image(this->size());
  QPainter{this}.drawImage(0, 0, current_image);
}

void Canvas::receive_new_image(QImage image) {
  current_image = std::move(image);
  this->update();
}

}  // namespace mandelbrot
