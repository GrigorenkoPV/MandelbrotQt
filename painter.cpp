#include "painter.h"

#include <QMutexLocker>

namespace mandelbrot {

[[noreturn]] void Painter::run() {
  while (true) {
    Params current_params;
    {
      QMutexLocker qml(&params_mutex);
      current_params = this->params;
    }
    // todo
    //  for степень сжатия по шкале шакалов:
    //     if canceled:
    //       continue while
    //     create image
    //     render using some concurrent qt magic
    //     emit sendNewImage(std::move(image));
    //  await event???
  }
}

// caller must hold the mutex
void Painter::initiateRedraw() {
  // todo: set up a flag or maybe some semaphore
}

void Painter::reset(QSize new_size) {
  QMutexLocker qml(&params_mutex);
  params = Params();
  params.canvas_size = new_size;
  initiateRedraw();
}

void Painter::setImageSize(QSize new_size) {
  QMutexLocker qml(&params_mutex);
  if (params.canvas_size != new_size) {
    params.canvas_size = new_size;
    initiateRedraw();
  }
}

void Painter::changeCenterPositionBy(QSize pixel_offset) {
  if (!pixel_offset.isNull()) {
    QMutexLocker qml(&params_mutex);
    // todo: recalculate to QPointF (using zoom) and add to params.center
    initiateRedraw();
  }
}

void Painter::setIterations(unsigned new_iterations) {
  QMutexLocker qml(&params_mutex);
  if (params.iterations != new_iterations) {
    params.iterations = new_iterations;
    initiateRedraw();
  }
}

}  // namespace mandelbrot
