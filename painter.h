#pragma once
#include <QColor>
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
  void requestRedraw();

 public:
  bool reset(QSize new_size);
  bool setImageSize(QSize new_size);
  bool changeCenterPositionBy(QSize pixel_offset);
  bool multiplyZoomBy(qreal multiple);
  bool setMaxIterations(unsigned new_max_iterations);

 private:
  static quint32 getColor(qreal x0, qreal y0, unsigned int max_iterations,
                          qreal threshold);
  static QImage render(QSize size, QPointF center, qreal zoom,
                       unsigned max_iterations, qreal threshold);

 signals:
  void sendImage(QImage image);
  void finished();

 public slots:
  void start();
  void stop();
};
}  // namespace mandelbrot
