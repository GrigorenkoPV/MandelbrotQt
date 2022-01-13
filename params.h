#pragma once

#include <QPointF>
#include <cmath>
#include <limits>

namespace mandelbrot {

constexpr QPointF DEFAULT_CENTER{-0.5, 0};
constexpr unsigned MIN_MAX_ITERATIONS = 0;
constexpr unsigned DEFAULT_MAX_ITERATIONS = 200;
constexpr unsigned MAX_MAX_ITERATIONS = std::numeric_limits<int>::max();
constexpr qreal DEFAULT_THRESHOLD = 1 << 16;

}  // namespace mandelbrot
