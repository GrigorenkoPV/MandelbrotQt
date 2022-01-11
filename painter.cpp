#include "painter.h"

#include <QColor>
#include <QMutexLocker>
#include <chrono>
#include <thread>

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
    for (int detail_level = 0; detail_level < 255; ++detail_level) {
      QImage image(current_params.canvas_size, QImage::Format_RGB888);
      {  // todo: render using some concurrent qt magic
        image.fill(QColor::fromRgb(0, 0, detail_level));
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(5ms);
      }
      QMutexLocker qml(&mutex);
      if (cancellation_requested) {
        break;
      }
      emit sendImage(std::move(image));
    }
    QMutexLocker qml(&mutex);
    if (cancellation_requested) {
      cancellation_requested = false;
    } else {
      condition.wait(&mutex);
    }
  }
}

// caller must hold the mutex
void Painter::cancelCurrent() {
  cancellation_requested = true;
  condition.wakeAll();
}

void Painter::reset(QSize new_size) {
  QMutexLocker qml(&mutex);
  params = Params();
  params.canvas_size = new_size;
  cancelCurrent();
}

void Painter::setImageSize(QSize new_size) {
  QMutexLocker qml(&mutex);
  if (params.canvas_size != new_size) {
    {  // todo: provide with a temporary replacement
      QImage image(new_size, QImage::Format_RGB888);
      image.fill(QColor::fromRgb(0, 255, 0));
      emit sendImage(std::move(image));
    }
    params.canvas_size = new_size;
    cancelCurrent();
  }
}

void Painter::changeCenterPositionBy(QSize pixel_offset) {
  if (!pixel_offset.isNull()) {
    QMutexLocker qml(&mutex);
    // todo: recalculate to QPointF (using zoom) and add to params.center
    cancelCurrent();
  }
}

void Painter::setIterations(unsigned new_iterations) {
  QMutexLocker qml(&mutex);
  if (params.iterations != new_iterations) {
    params.iterations = new_iterations;
    cancelCurrent();
  }
}

void Painter::start() {
  run();
  emit finished();
}

void Painter::stop() {
  {
    QMutexLocker qml(&mutex);
    stop_requested = true;
    cancelCurrent();
  }
}

}  // namespace mandelbrot
