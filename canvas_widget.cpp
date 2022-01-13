#include "canvas_widget.h"

#include <QPainter>
#include <QThread>
#include <cmath>

namespace mandelbrot {

CanvasWidget::CanvasWidget(QWidget* parent)
    : QWidget(parent),
      renderer(new Renderer()),
      current_image(),
      last_mouse_position(),
      params() {
  setFocusPolicy(Qt::ClickFocus);
  auto thread = new QThread();
  renderer->moveToThread(thread);
  QObject::connect(thread, &QThread::started, renderer, &Renderer::run);
  QObject::connect(renderer, &Renderer::sendImage, this,
                   &CanvasWidget::receiveImage);
  QObject::connect(renderer, &Renderer::finished, thread, &QThread::quit);
  QObject::connect(renderer, &Renderer::finished, renderer,
                   &Renderer::deleteLater);
  QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

CanvasWidget::~CanvasWidget() { renderer->stop(); }

void CanvasWidget::redraw() { renderer->setNextJob(params); }

void CanvasWidget::resetPan() {
  params.center = DEFAULT_CENTER;
  params.zoom = std::max(3. / params.canvas_size.width(),
                         2. / params.canvas_size.height());
  redraw();
}

void CanvasWidget::reset() {
  params.max_iterations = DEFAULT_MAX_ITERATIONS;
  params.threshold = DEFAULT_THRESHOLD;
  resetPan();
}

unsigned CanvasWidget::getMaxIterations() const {
  return params.max_iterations;
}
void CanvasWidget::setMaxIterations(unsigned value) {
  params.max_iterations = value;
}

bool CanvasWidget::setImageSize(QSize new_size) {
  if (!new_size.isValid()) {
    return false;
  }
  if (std::isnan(params.zoom)) {
    if (new_size.isEmpty()) {
      return false;
    }
    params.canvas_size = new_size;
    resetPan();
  } else if (params.canvas_size != new_size) {
    params.canvas_size = new_size;
    redraw();
  }
  return true;
}

void CanvasWidget::resizeEvent(QResizeEvent* event) {
  setImageSize(this->size());
}
void CanvasWidget::paintEvent(QPaintEvent* event) {
  QPainter{this}.drawImage(0, 0, current_image);
}

void CanvasWidget::keyPressEvent(QKeyEvent* event) {
  constexpr auto DOTS_PER_MOVE = 20;
  switch (event->key()) {
    case Qt::Key_Home:
      resetPan();
      return;
    case Qt::Key_Down:
      params.center.ry() += DOTS_PER_MOVE * params.zoom;
      break;
    case Qt::Key_Up:
      params.center.ry() -= DOTS_PER_MOVE * params.zoom;
      break;
    case Qt::Key_Left:
      params.center.rx() -= DOTS_PER_MOVE * params.zoom;
      break;
    case Qt::Key_Right:
      params.center.rx() += DOTS_PER_MOVE * params.zoom;
      break;
    case Qt::Key_Minus:
      params.zoom *= 1.2;
      break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
      params.zoom /= 1.2;
      break;
    default:
      QWidget::keyPressEvent(event);
      return;
  }
  redraw();
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    last_mouse_position = event->pos();
  }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    params.center += QPointF(last_mouse_position - event->pos()) * params.zoom;
    redraw();
    last_mouse_position = event->pos();
  }
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
  auto const pixel_delta = event->position() - QPointF(width(), height()) / 2;
  params.center += pixel_delta * params.zoom;
  params.zoom *= std::exp(event->angleDelta().y() * -0.002);
  params.center -= pixel_delta * params.zoom;
  redraw();
}

void CanvasWidget::receiveImage(QImage image) {
  current_image = std::move(image);
  this->update();
}

}  // namespace mandelbrot
