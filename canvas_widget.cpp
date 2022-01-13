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
  // todo: improve focus policy when other widgets are ready
  setFocusPolicy(Qt::StrongFocus);
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

bool CanvasWidget::setImageSize(QSize new_size, bool reset_pan) {
  if (!new_size.isValid()) {
    return false;
  }
  if (std::isnan(params.zoom)) {
    reset_pan = true;
  }
  if (!reset_pan && params.canvas_size == new_size) {
    return true;
  }
  if (reset_pan) {
    if (new_size.isEmpty()) {
      return false;
    }
    params = RenderingJob();
    params.zoom = std::max(3. / new_size.width(), 2. / new_size.height());
  }
  params.canvas_size = new_size;
  redraw();
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
      setImageSize(this->size(), true);
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
