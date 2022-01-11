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

  void keyPressEvent(QKeyEvent *event) override;

  // todo mouse controls

 public slots:
  void receiveNewImage(QImage image);
};
}  // namespace mandelbrot
