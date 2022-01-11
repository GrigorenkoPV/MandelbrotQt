#pragma once
#include <QImage>
#include <QObject>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

#include "painter.h"

namespace mandelbrot {
class Canvas : public QWidget {
  Q_OBJECT

  Painter painter;
  QImage current_image;

 public:
  explicit Canvas(QWidget *parent = nullptr);

 protected:
  void paintEvent(QPaintEvent *event) override;

 signals:
  void request_new_image(QSize size);

 public slots:
  void receive_new_image(QImage image);
};
}  // namespace mandelbrot
