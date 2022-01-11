#include "painter.h"

#include <QColor>

namespace mandelbrot {

void Painter::new_image_requested(QSize size) {
  if (size != current_size) {
    QImage image(current_size = size, QImage::Format_RGB32);
    image.fill(QColor::fromRgb(0, 100, 200));
    emit send_new_image(std::move(image));
  }
}

}  // namespace mandelbrot
