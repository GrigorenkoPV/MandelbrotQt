#pragma once
#include <QImage>
#include <QObject>
#include <QSize>

namespace mandelbrot {
class Painter : public QObject {
  Q_OBJECT

  QSize current_size;

 public:
  Painter() = default;

 signals:
  void send_new_image(QImage image);

 public slots:
  void new_image_requested(QSize size);
};
}  // namespace mandelbrot
