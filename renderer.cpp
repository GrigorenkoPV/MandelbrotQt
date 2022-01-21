#include "renderer.h"

#include <QMutexLocker>
#include <QPoint>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <atomic>
#include <cstring>
#include <memory>
#include <vector>

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

Renderer::pixel_t Renderer::renderPointInRGB(qreal x0, qreal y0,
                                             unsigned max_iterations,
                                             qreal threshold) {
  static constexpr pixel_t palette[] = {
      0x19071a, 0x09012f, 0x040449, 0x000764, 0x0c2c8a, 0x1852b1,
      0x397dd1, 0x86b5e5, 0xd3ecf8, 0xf1e9bf, 0xf8c95f, 0xffaa00,
      0xcc8000, 0x995700, 0x6a3403, 0x421e0f,
  };
  static constexpr size_t palette_size = sizeof(palette) / sizeof(quint32);
  static thread_local unsigned total_iterations = 0;
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

void operator_delete(void* ptr) noexcept(noexcept(operator delete(ptr))) {
  operator delete(ptr);
}

void Renderer::doJob(const RenderingJob& job) {
  assert(job.canvas_size.isValid());
  ++debt;
  unsigned start_square_side;
  if (debt > 16) {
    start_square_side = 64;
  } else if (debt > 4) {
    start_square_side = 16;
  } else {
    start_square_side = 4;
  }
  unsigned const width = job.canvas_size.width();
  unsigned const height = job.canvas_size.height();
  auto const start_point = job.center - QPointF(width, height) / 2 * job.zoom;
  size_t const line_size = width * sizeof(pixel_t);
  for (auto square_side = start_square_side; square_side != 0;
       square_side /= 4) {
#ifndef NDEBUG
    auto t_start = std::chrono::high_resolution_clock::now();
#endif
    std::unique_ptr<void, decltype(&operator_delete)> buffer_owner(
        operator new (line_size* height, std::align_val_t{alignof(pixel_t)}),
        operator_delete);
    auto const buffer = static_cast<uchar*>(buffer_owner.get());

    std::vector<unsigned> etalon_rows;
    etalon_rows.reserve(height / square_side + 1);
    for (unsigned y = 0; y < height; y += square_side) {
      etalon_rows.push_back(y);
    }

    std::atomic<bool> cancelled{false};

    QtConcurrent::map(etalon_rows, [&](unsigned y) {
      try {
        auto const etalon_row = buffer + line_size * y;
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
        std::atomic_thread_fence(std::memory_order_release);
      } catch (JobCancelled const&) {
        cancelled.store(true, std::memory_order_release);
      }
    }).waitForFinished();

    if (cancelled.load(std::memory_order_acquire)) {
#ifndef NDEBUG
      std::cerr << "Cancelled" << std::endl;
#endif
      return;
    }
    std::atomic_thread_fence(std::memory_order_acquire);

#ifndef NDEBUG
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cerr
        << "Rendered " << job << " with " << square_side - 1
        << " duplicates in "
        << std::chrono::duration<double, std::milli>(t_end - t_start).count()
        << "ms" << std::endl;
#endif
    emit sendImage(QImage(static_cast<uchar*>(buffer_owner.release()),
                          job.canvas_size.width(), job.canvas_size.height(),
                          IMAGE_FORMAT, operator_delete));
    debt = 0;
  }
}

void Renderer::run() {
  for (auto job = getNextJob(); job.has_value(); job = getNextJob()) {
    doJob(*job);
  }
  emit finished();
}

void Renderer::stop() {
  QMutexLocker qml(&mutex);
  jobs_ended = true;
  jumpToNextJob();
}

}  // namespace mandelbrot
