#include "canvas_widget.h"

#include <QPainter>
#include <QThread>
#include <cmath>

namespace mandelbrot {

CanvasWidget::CanvasWidget(QWidget* parent)
    : QWidget(parent),
      renderer(new Renderer()),
      current_image(),
      last_mouse_position() {
  // todo: improve focus policy when mouse controls or other widgets are ready
  setFocusPolicy(Qt::StrongFocus);
  auto thread = new QThread();
  renderer->moveToThread(thread);
  QObject::connect(thread, &QThread::started, renderer, &Renderer::start);
  QObject::connect(renderer, &Renderer::sendImage, this,
                   &CanvasWidget::receiveImage);
  QObject::connect(this, &CanvasWidget::stopRenderer, renderer,
                   &Renderer::stop);
  QObject::connect(renderer, &Renderer::finished, thread, &QThread::quit);
  QObject::connect(renderer, &Renderer::finished, renderer,
                   &Renderer::deleteLater);
  QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

CanvasWidget::~CanvasWidget() { emit stopRenderer(); }

void CanvasWidget::resizeEvent(QResizeEvent* event) {
  renderer->setImageSize(this->size());
}
void CanvasWidget::paintEvent(QPaintEvent* event) {
  QPainter{this}.drawImage(0, 0, current_image);
}

void CanvasWidget::keyPressEvent(QKeyEvent* event) {
  constexpr auto DOTS_PER_MOVE = 20;
  switch (event->key()) {
    case Qt::Key_Home:
      renderer->setImageSize(this->size(), true);
      break;
    case Qt::Key_Down:
      renderer->changeCenterPositionBy({0, DOTS_PER_MOVE});
      break;
    case Qt::Key_Up:
      renderer->changeCenterPositionBy({0, -DOTS_PER_MOVE});
      break;
    case Qt::Key_Left:
      renderer->changeCenterPositionBy({-DOTS_PER_MOVE, 0});
      break;
    case Qt::Key_Right:
      renderer->changeCenterPositionBy({DOTS_PER_MOVE, 0});
      break;
    case Qt::Key_Minus:
      renderer->multiplyZoomBy(1.2);
      break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
      renderer->multiplyZoomBy(1 / 1.2);
      break;
    default:
      QWidget::keyPressEvent(event);
      return;
  }
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    last_mouse_position = event->pos();
  }
}
void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    renderer->changeCenterPositionBy(last_mouse_position - event->pos());
    last_mouse_position = event->pos();
  }
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
  // todo: cooler zoom by moving params into canvas
  //  this will also reduce the number of mutex locking
  renderer->multiplyZoomBy(std::exp(event->angleDelta().y() * -0.002));
}

void CanvasWidget::receiveImage(QImage image) {
  current_image = std::move(image);
  this->update();
}

}  // namespace mandelbrot
