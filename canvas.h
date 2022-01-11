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

  Painter *painter;
  QImage current_image;

 public:
  explicit Canvas(QWidget *parent = nullptr);
  ~Canvas() override;

 protected:
  void paintEvent(QPaintEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

  // todo mouse controls

 private:
 signals:
  void stopPainter();

 public slots:
  void receiveImage(QImage image);
};
}  // namespace mandelbrot
