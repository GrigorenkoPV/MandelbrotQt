#pragma once

#include "params.h"

namespace mandelbrot {
struct RenderingJob {
  QSize canvas_size{};
  QPointF center = DEFAULT_CENTER;
  qreal zoom = NAN;  // units per dot
  unsigned max_iterations = DEFAULT_MAX_ITERATIONS;
  qreal threshold = DEFAULT_THRESHOLD;
};
}  // namespace mandelbrot
