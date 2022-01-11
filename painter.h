#pragma once
#include <QImage>
#include <QMutex>
#include <QWaitCondition>

#include "params.h"

namespace mandelbrot {
class Painter : public QObject {
  Q_OBJECT

  Params params = {};
  QMutex mutex = {};
  bool cancellation_requested = false;
  bool stop_requested = false;
  QWaitCondition condition = {};

 public:
  Painter() = default;

 private:
  void run();
  // caller must hold the mutex
  void cancelCurrent();

 public:
  void reset(QSize new_size);
  void setImageSize(QSize new_size);
  void changeCenterPositionBy(QSize pixel_offset);
  // todo change zoom
  void setIterations(unsigned new_iterations);

 signals:
  void sendImage(QImage image);
  void finished();

 public slots:
  void start();
  void stop();
};
}  // namespace mandelbrot
