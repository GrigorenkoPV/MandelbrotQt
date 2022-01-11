#include "canvas.h"

#include <QPainter>
#include <QThread>

namespace mandelbrot {

Canvas::Canvas(QWidget* parent)
    : QWidget(parent), painter(new Painter()), current_image() {
  // todo: improve focus policy when mouse controls or other widgets are ready
  setFocusPolicy(Qt::StrongFocus);
  auto thread = new QThread();
  painter->moveToThread(thread);
  QObject::connect(thread, &QThread::started, painter, &Painter::start);
  QObject::connect(painter, &Painter::sendImage, this, &Canvas::receiveImage);
  QObject::connect(painter, &Painter::finished, thread, &QThread::quit);
  QObject::connect(painter, &Painter::finished, painter, &Painter::deleteLater);
  QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

Canvas::~Canvas() { emit stopPainter(); }

void Canvas::paintEvent(QPaintEvent* event) {
  if (painter_initialized) {
    painter->setImageSize(this->size());
  } else {
    painter->reset(this->size());
    painter_initialized = true;
  }
  QPainter{this}.drawImage(0, 0, current_image);
}

void Canvas::keyPressEvent(QKeyEvent* event) {
  constexpr auto DOTS_PER_MOVE = 20;
  switch (event->key()) {
    case Qt::Key_Home:
      painter->reset(this->size());
      break;
    case Qt::Key_Down:
      painter->changeCenterPositionBy({0, DOTS_PER_MOVE});
      break;
    case Qt::Key_Up:
      painter->changeCenterPositionBy({0, -DOTS_PER_MOVE});
      break;
    case Qt::Key_Left:
      painter->changeCenterPositionBy({-DOTS_PER_MOVE, 0});
      break;
    case Qt::Key_Right:
      painter->changeCenterPositionBy({DOTS_PER_MOVE, 0});
      break;
    case Qt::Key_Minus:
      painter->multiplyZoomBy(1.2);
      break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
      painter->multiplyZoomBy(1 / 1.2);
      break;
    default:
      QWidget::keyPressEvent(event);
      return;
  }
}

void Canvas::receiveImage(QImage image) {
  current_image = std::move(image);
  this->update();
}

}  // namespace mandelbrot
