#pragma once
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QPoint>
#include <QRunnable>
#include <QSize>
#include <QWaitCondition>
#include <optional>

#include "rendering_job.h"

namespace mandelbrot {

class JobCancelled : public std::exception {};

class Renderer : public QObject {
  Q_OBJECT

  RenderingJob next_job = {};
  QMutex mutex = {};
  bool has_new_job_or_jobs_ended = false;
  bool jobs_ended = false;
  QWaitCondition waiting_for_job = {};

 public:
  Renderer() = default;

  void setNextJob(RenderingJob job);

 private:
  std::optional<RenderingJob> getNextJob();

  // caller must hold the mutex
  void jumpToNextJob();

  quint32 renderPointInRGB(qreal x0, qreal y0, unsigned max_iterations,
                           qreal threshold);
  void doJob(RenderingJob const& job);

 public:
  void stop();

 signals:
  void sendImage(QImage image);
  void finished();

 public slots:
  void run();
};
}  // namespace mandelbrot
