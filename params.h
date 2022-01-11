#pragma once
#include <QColor>
#include <QPointF>
#include <QSize>

namespace mandelbrot {
struct Params {
  static constexpr QSize DEFAULT_SIZE{};
  static constexpr QPointF DEFAULT_CENTER{-0.5, 0};
  static constexpr unsigned DEFAULT_ITERATIONS{};

  QSize canvas_size = DEFAULT_SIZE;
  QPointF center = DEFAULT_CENTER;
  // todo zoom
  unsigned iterations = DEFAULT_ITERATIONS;
  // todo colors? Or maybe just hardcode them lmao
};
}  // namespace mandelbrot
