#include "painter.h"

#include <QMutexLocker>
#include <QPoint>
#include <vector>

namespace mandelbrot {

void Painter::run() {
  while (true) {
    Params current_params;
    {
      QMutexLocker qml(&mutex);
      if (stop_requested) {
        return;
      }
      current_params = this->params;
    }
    // todo: шакалы
    unsigned const step = 20;
    unsigned iterations = 0;
    do {
      iterations = std::min(iterations + step, current_params.max_iterations);
      QImage image = render(current_params.canvas_size, current_params.center,
                            current_params.zoom, iterations, 1 << 16);
      QMutexLocker qml(&mutex);
      if (cancellation_requested) {
        break;
      }
      emit sendImage(std::move(image));
    } while (iterations < params.max_iterations);
    QMutexLocker qml(&mutex);
    if (cancellation_requested) {
      cancellation_requested = false;
    } else {
      condition.wait(&mutex);
    }
  }
}

// caller must hold the mutex
void Painter::requestRedraw() {
  cancellation_requested = true;
  condition.wakeAll();
}

bool Painter::reset(QSize new_size) {
  if (!new_size.isValid() || new_size.isEmpty()) {
    return false;
  }
  QMutexLocker qml(&mutex);
  params = Params();
  params.canvas_size = new_size;
  params.zoom = std::max(3. / new_size.width(), 2. / new_size.height());
  requestRedraw();
  return true;
}

bool Painter::setImageSize(QSize new_size) {
  if (!new_size.isValid()) {
    return false;
  }
  QMutexLocker qml(&mutex);
  if (params.canvas_size != new_size) {
    //    {  // todo: provide with a temporary replacement
    //      QImage image(new_size, QImage::Format_RGB888);
    //      image.fill(QColor::fromRgb(0, 255, 0));
    //      emit sendImage(std::move(image));
    //    }
    params.canvas_size = new_size;
    requestRedraw();
  }
  return true;
}

bool Painter::changeCenterPositionBy(QSize pixel_offset) {
  if (pixel_offset.isNull()) {
    return false;
  }
  QMutexLocker qml(&mutex);
  params.center +=
      QPointF(pixel_offset.width(), pixel_offset.height()) * params.zoom;
  requestRedraw();
  return true;
}

bool Painter::multiplyZoomBy(qreal multiple) {
  if (multiple <= 0) {
    return false;
  }
  if (multiple == 1) {
    return true;
  }
  QMutexLocker qml(&mutex);
  params.zoom *= multiple;
  requestRedraw();
  return true;
}

bool Painter::setMaxIterations(unsigned new_max_iterations) {
  if (new_max_iterations == 0) {
    return false;
  }
  QMutexLocker qml(&mutex);
  if (params.max_iterations != new_max_iterations) {
    params.max_iterations = new_max_iterations;
    requestRedraw();
  }
  return true;
}

quint32 Painter::getColor(qreal const x0, qreal const y0,
                          unsigned const max_iterations,
                          qreal const threshold) {
  // todo: colors
  static std::vector<quint32> const palette = {
      0x19071a, 0x09012f, 0x040449, 0x000764, 0x0c2c8a, 0x1852b1,
      0x397dd1, 0x86b5e5, 0xd3ecf8, 0xf1e9bf, 0xf8c95f, 0xffaa00,
      0xcc8000, 0x995700, 0x6a3403, 0x421e0f,
  };
  qreal x = 0;
  qreal y = 0;
  qreal x2 = 0;
  qreal y2 = 0;
  for (unsigned iteration = 0; iteration < max_iterations; ++iteration) {
    if (x2 + y2 > threshold) {
      return palette[iteration % palette.size()];
    }
    y = 2 * x * y + y0;
    x = x2 - y2 + x0;
    x2 = x * x;
    y2 = y * y;
  }
  return 0x000000;
}

QImage Painter::render(QSize const size, QPointF const center, qreal const zoom,
                       unsigned const max_iterations, qreal const threshold) {
  QImage image(size, QImage::Format_RGB32);
  auto const stride = image.bytesPerLine();
  QPointF start = center - QPointF(size.width(), size.height()) / 2 * zoom;
  qreal y = start.y();
  for (int pixel_y = 0; pixel_y < size.height(); ++pixel_y) {
    qreal x = start.x();
    auto p = reinterpret_cast<quint32*>(image.bits() + stride * pixel_y);
    for (int pixel_x = 0; pixel_x < size.width(); ++pixel_x) {
      *(p++) = Painter::getColor(x, y, max_iterations, threshold);
      x += zoom;
    }
    y += zoom;
  }
  return image;
}

void Painter::start() {
  run();
  emit finished();
}

void Painter::stop() {
  {
    QMutexLocker qml(&mutex);
    stop_requested = true;
    requestRedraw();
  }
}

}  // namespace mandelbrot
