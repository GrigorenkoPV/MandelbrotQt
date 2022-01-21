#include "renderer.h"

#include <QMutexLocker>
#include <QPoint>
#include <algorithm>
#include <cstring>

#include "rendering_job.h"

#ifndef NDEBUG
#include <chrono>
#include <iostream>
#endif

namespace mandelbrot {

void Renderer::setNextJob(RenderingJob job) {
  QMutexLocker qml(&mutex);
  next_job = job;
  jumpToNextJob();
};

std::optional<RenderingJob> Renderer::getNextJob() {
  QMutexLocker qml(&mutex);
  if (has_new_job_or_jobs_ended) {
    has_new_job_or_jobs_ended = false;
  } else {
    waiting_for_job.wait(&mutex);
  }
  if (jobs_ended) {
    return {};
  } else {
    return {next_job};
  }
}

// caller must hold the mutex
void Renderer::jumpToNextJob() {
  has_new_job_or_jobs_ended = true;
  waiting_for_job.wakeAll();
}

quint32 Renderer::renderPointInRGB(qreal x0, qreal y0, unsigned max_iterations,
                                   qreal threshold) {
  static constexpr quint32 palette[] = {
      0x19071a, 0x09012f, 0x040449, 0x000764, 0x0c2c8a, 0x1852b1,
      0x397dd1, 0x86b5e5, 0xd3ecf8, 0xf1e9bf, 0xf8c95f, 0xffaa00,
      0xcc8000, 0x995700, 0x6a3403, 0x421e0f,
  };
  static constexpr size_t palette_size = sizeof(palette) / sizeof(quint32);
  static unsigned total_iterations = 0;
  qreal x = 0;
  qreal y = 0;
  qreal x2 = 0;
  qreal y2 = 0;
  for (unsigned iteration = 0; iteration < max_iterations; ++iteration) {
    if (x2 + y2 > threshold) {
      return palette[iteration % palette_size];
    }
    y = 2 * x * y + y0;
    x = x2 - y2 + x0;
    x2 = x * x;
    y2 = y * y;
    if (++total_iterations == CHECK_CANCEL_EVERY_ITERATIONS) {
      total_iterations = 0;
      QMutexLocker qml(&mutex);
      if (has_new_job_or_jobs_ended) {
        throw JobCancelled{};
      }
    }
  }
  return 0x000000;
}

#ifndef NDEBUG
std::ostream& operator<<(std::ostream& os, RenderingJob const& job) {
  return os << job.canvas_size.width() << 'x' << job.canvas_size.height()
            << ", threshold=" << job.threshold
            << ", iterations=" << job.max_iterations;
}
#endif

void Renderer::doJob(const RenderingJob& job) {
  static unsigned debt = 0;
  ++debt;
  unsigned start_square_side;
  if (debt > 16) {
    start_square_side = 64;
  } else if (debt > 4) {
    start_square_side = 16;
  } else {
    start_square_side = 4;
  }
  for (auto square_side = start_square_side; square_side != 0;
       square_side /= 4) {
#ifndef NDEBUG
    auto t_start = std::chrono::high_resolution_clock::now();
#endif
    QImage image(job.canvas_size, QImage::Format_RGB32);
    auto const line_size = image.bytesPerLine();
    unsigned const width = job.canvas_size.width();
    unsigned const height = job.canvas_size.height();
    auto const start_point = job.center - QPointF(width, height) / 2 * job.zoom;
    for (unsigned y = 0; y < height;) {
      auto const etalon_row = image.bits() + line_size * y;
      auto const y0 = start_point.y() + job.zoom * y;
      for (unsigned x = 0; x < width;) {
        auto const x0 = start_point.x() + job.zoom * x;
        auto const color =
            renderPointInRGB(x0, y0, job.max_iterations, job.threshold);
        auto const n = std::min(square_side, width - x);
        std::fill_n(reinterpret_cast<quint32*>(etalon_row) + x, n, color);
        x += n;
      }
      for (unsigned d = 1; (d < square_side) && (y + d < height); ++d) {
        std::memcpy(etalon_row + line_size * d, etalon_row, line_size);
      }
      y += square_side;
    }
#ifndef NDEBUG
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cerr
        << "Rendered " << job << " with " << square_side - 1
        << " duplicates in "
        << std::chrono::duration<double, std::milli>(t_end - t_start).count()
        << "ms" << std::endl;
#endif
    emit sendImage(std::move(image));
    debt = 0;
  }
}

void Renderer::run() {
  for (auto job = getNextJob(); job.has_value(); job = getNextJob()) {
    try {
      doJob(*job);
    } catch (JobCancelled const&) {
#ifndef NDEBUG
      std::cerr << "Cancelled" << std::endl;
#endif
    }
  }
  emit finished();
}

void Renderer::stop() {
  QMutexLocker qml(&mutex);
  jobs_ended = true;
  jumpToNextJob();
}

}  // namespace mandelbrot
