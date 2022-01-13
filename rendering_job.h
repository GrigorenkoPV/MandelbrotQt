#pragma once

#include <QPointF>
#include <QSize>
#include <cmath>

namespace mandelbrot {
struct RenderingJob {
  static constexpr QSize DEFAULT_SIZE{};
  static constexpr QPointF DEFAULT_CENTER{-0.5, 0};
  static constexpr unsigned DEFAULT_MAX_ITERATIONS{100};
  static constexpr qreal DEFAULT_THRESHOLD{1 << 16};

  QSize canvas_size = DEFAULT_SIZE;
  QPointF center = DEFAULT_CENTER;
  qreal zoom = NAN;  // units per dot
  unsigned max_iterations = DEFAULT_MAX_ITERATIONS;
  qreal threshold = DEFAULT_THRESHOLD;
};
}  // namespace mandelbrot
