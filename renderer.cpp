#include "renderer.h"

#include <QMutexLocker>
#include <QPoint>
#include <algorithm>
#include <cstring>
#include <vector>

#ifndef NDEBUG
#include <chrono>
#include <iostream>
#endif

namespace mandelbrot {

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

bool Renderer::setImageSize(QSize new_size, bool reset_pan) {
  if (!new_size.isValid()) {
    return false;
  }
  QMutexLocker qml(&mutex);
  if (std::isnan(next_job.zoom)) {
    reset_pan = true;
  }
  if (!reset_pan && next_job.canvas_size == new_size) {
    return true;
  }
  if (reset_pan) {
    if (new_size.isEmpty()) {
      return false;
    }
    next_job = RenderingJob();
    next_job.zoom = std::max(3. / new_size.width(), 2. / new_size.height());
  }
  next_job.canvas_size = new_size;
  jumpToNextJob();
  return true;
}

bool Renderer::changeCenterPositionBy(QSize pixel_offset) {
  if (pixel_offset.isNull()) {
    return false;
  }
  QMutexLocker qml(&mutex);
  next_job.center +=
      QPointF(pixel_offset.width(), pixel_offset.height()) * next_job.zoom;
  jumpToNextJob();
  return true;
}

bool Renderer::multiplyZoomBy(qreal multiple) {
  if (multiple <= 0) {
    return false;
  }
  if (multiple == 1) {
    return true;
  }
  QMutexLocker qml(&mutex);
  next_job.zoom *= multiple;
  jumpToNextJob();
  return true;
}

bool Renderer::setMaxIterations(unsigned new_max_iterations) {
  if (new_max_iterations == 0) {
    return false;
  }
  QMutexLocker qml(&mutex);
  if (next_job.max_iterations != new_max_iterations) {
    next_job.max_iterations = new_max_iterations;
    jumpToNextJob();
  }
  return true;
}

quint32 Renderer::renderPointInRGB(qreal x0, qreal y0, unsigned max_iterations,
                                   qreal threshold) {
  // todo: colors
  static std::vector<quint32> const palette = {
      0x19071a, 0x09012f, 0x040449, 0x000764, 0x0c2c8a, 0x1852b1,
      0x397dd1, 0x86b5e5, 0xd3ecf8, 0xf1e9bf, 0xf8c95f, 0xffaa00,
      0xcc8000, 0x995700, 0x6a3403, 0x421e0f,
  };
  qreal x = 0;
  qreal y = 0;
  qreal x2 = 0;
  qreal y2 = 0;
  for (unsigned iteration = 0; iteration < max_iterations; ++iteration) {
    if (x2 + y2 > threshold) {
      return palette[iteration % palette.size()];
    }
    y = 2 * x * y + y0;
    x = x2 - y2 + x0;
    x2 = x * x;
    y2 = y * y;
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

// todo: parallelism maybe?
void Renderer::doJob(const RenderingJob& job) {
  // todo: get rid of `duplicates` & come up with a better square_side sequnce
  for (unsigned square_side = 1 << 4; square_side != 0; square_side /= 4) {
    auto const duplicates = square_side - 1;
#ifndef NDEBUG
    auto t_start = std::chrono::high_resolution_clock::now();
#endif
    QImage image(job.canvas_size, QImage::Format_RGB32);
    auto const stride = image.bytesPerLine();
    unsigned const width = job.canvas_size.width();
    unsigned const height = job.canvas_size.height();
    auto const start_point_point =
        job.center - QPointF(width, height) / 2 * job.zoom;
    QPointF point = start_point_point;
    for (unsigned y = 0; y < height;) {
      auto const etalon_row = image.bits() + stride * y;
      point.setX(start_point_point.x());
      for (unsigned x = 0; x < width;) {
        auto const color = renderPointInRGB(point.x(), point.y(),
                                            job.max_iterations, job.threshold);
        auto const n = std::min(duplicates + 1, width - x);
        std::fill_n(reinterpret_cast<quint32*>(etalon_row) + x, n, color);
        x += n;
        point.rx() += job.zoom * n;
      }
      {
        QMutexLocker qml(&mutex);
        if (has_new_job_or_jobs_ended) {
          return;
        }
      }
      for (unsigned d = 0; (d < duplicates) && (y + d + 1 < height); ++d) {
        std::memcpy(etalon_row + stride * (d + 1), etalon_row, stride);
      }
      y += duplicates + 1;
      point.ry() += job.zoom * (duplicates + 1);
    }
#ifndef NDEBUG
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cerr
        << "Rendered " << job << " with " << duplicates << " duplicates in "
        << std::chrono::duration<double, std::milli>(t_end - t_start).count()
        << "ms" << std::endl;
#endif
    emit sendImage(std::move(image));
  }
}

void Renderer::start() {
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
