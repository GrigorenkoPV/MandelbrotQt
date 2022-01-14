#pragma once

#include <QPointF>
#include <cmath>
#include <limits>

namespace mandelbrot {

constexpr QPointF DEFAULT_CENTER{-0.5, 0};
constexpr unsigned MINIMAL_MAX_ITERATIONS = 0;
constexpr unsigned DEFAULT_MAX_ITERATIONS = 200;
constexpr unsigned MAXIMAL_MAX_ITERATIONS = std::numeric_limits<int>::max();
constexpr qreal MINIMAL_THRESHOLD = 1 << 1;
constexpr qreal DEFAULT_THRESHOLD = 1 << 15;
constexpr qreal MAXIMAL_THRESHOLD = 1 << 30;

}  // namespace mandelbrot
