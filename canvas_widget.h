#pragma once
#include <QImage>
#include <QObject>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

#include "renderer.h"
#include "rendering_job.h"

namespace mandelbrot {
class CanvasWidget : public QWidget {
  Q_OBJECT

  Renderer *renderer;
  QImage current_image;
  QPoint last_mouse_position;
  RenderingJob params;

 public:
  explicit CanvasWidget(QWidget *parent = nullptr);
  ~CanvasWidget() override;

  void redraw();

  void resetPan();
  void reset();

  [[nodiscard]] unsigned getMaxIterations() const noexcept;
  bool setMaxIterations(unsigned value) noexcept;
  [[nodiscard]] qreal getThreshold() const noexcept;
  bool setThreshold(qreal threshold) noexcept;

 private:
  bool setImageSize(QSize new_size);

 protected:
  void resizeEvent(QResizeEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;

 public slots:
  void receiveImage(QImage image);
};
}  // namespace mandelbrot
