#pragma once
#include <QImage>
#include <QObject>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

#include "renderer.h"

namespace mandelbrot {
class CanvasWidget : public QWidget {
  Q_OBJECT

  Renderer *renderer;
  QImage current_image;
  QPoint last_mouse_position;

 public:
  explicit CanvasWidget(QWidget *parent = nullptr);
  ~CanvasWidget() override;

 protected:
  void resizeEvent(QResizeEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;

 private:
 signals:
  void stopRenderer();

 public slots:
  void receiveImage(QImage image);
};
}  // namespace mandelbrot
