#pragma once
#include <QImage>
#include <QMutex>
#include <QThread>

#include "params.h"

namespace mandelbrot {
// fixme:
//  either don't inherit from QThread
//  or implement proper shutdown mechanism
//  (because currently Canvas just destroys the object
//   before the thread is finished, which is not good)
class Painter : public QThread {
  Q_OBJECT

  Params params;
  QMutex params_mutex;

 public:
  Painter() = default;

 protected:
  [[noreturn]] void run() override;

 private:
  // caller must hold the mutex
  void initiateRedraw();

 public:
  void reset(QSize new_size);
  void setImageSize(QSize new_size);
  void changeCenterPositionBy(QSize pixel_offset);
  // todo change zoom
  void setIterations(unsigned new_iterations);

 signals:
  void sendNewImage(QImage image);
};
}  // namespace mandelbrot
