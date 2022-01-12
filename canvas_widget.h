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

 public:
  explicit CanvasWidget(QWidget *parent = nullptr);
  ~CanvasWidget() override;

 protected:
  void paintEvent(QPaintEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

  // todo mouse controls

 private:
 signals:
  void stopRenderer();

 public slots:
  void receiveImage(QImage image);
};
}  // namespace mandelbrot
