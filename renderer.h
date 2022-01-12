#pragma once
#include <QColor>
#include <QImage>
#include <QMutex>
#include <QPointF>
#include <QSize>
#include <QWaitCondition>
#include <cmath>
#include <optional>

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

class Renderer : public QObject {
  Q_OBJECT

  RenderingJob next_job = {};
  QMutex mutex = {};
  bool has_new_job_or_jobs_ended = false;
  bool jobs_ended = false;
  QWaitCondition waiting_for_job = {};

 public:
  Renderer() = default;

 private:
  std::optional<RenderingJob> getNextJob();

  // caller must hold the mutex
  void jumpToNextJob();

 public:
  bool setImageSize(QSize new_size, bool reset_pan = false);
  bool changeCenterPositionBy(QPoint pixel_offset);
  bool multiplyZoomBy(qreal multiple);
  bool setMaxIterations(unsigned new_max_iterations);

 private:
  static quint32 renderPointInRGB(qreal x0, qreal y0,
                                  unsigned int max_iterations, qreal threshold);
  void doJob(RenderingJob const& job);

 signals:
  void sendImage(QImage image);
  void finished();

 public slots:
  void start();
  void stop();
};
}  // namespace mandelbrot
