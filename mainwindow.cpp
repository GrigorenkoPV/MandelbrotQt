#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "params.h"

using namespace mandelbrot;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  ui->maxiteration_spinbox->setMinimum(MINIMAL_MAX_ITERATIONS);
  ui->maxiteration_spinbox->setMaximum(MAXIMAL_MAX_ITERATIONS);
  ui->threshold_spinbox->setMinimum(MINIMAL_THRESHOLD);
  ui->threshold_spinbox->setMaximum(MAXIMAL_THRESHOLD);
  connect(ui->apply_button, &QPushButton::clicked, this, &MainWindow::applySettings);
  connect(ui->reset_all_button, &QPushButton::clicked, this, &MainWindow::reset);
  connect(ui->reset_pan_button, &QPushButton::clicked, this, &MainWindow::resetPan);
  resetMenu();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::resetMenu() {
  ui->maxiteration_spinbox->setValue(ui->canvas->getMaxIterations());
  ui->threshold_spinbox->setValue(ui->canvas->getThreshold());
}

void MainWindow::reset() {
  ui->canvas->reset();
  resetMenu();
}
void MainWindow::resetPan() {
  ui->canvas->resetPan();
}
void MainWindow::applySettings() {
  bool anything_changed = false;
  bool ok = ui->canvas->setMaxIterations(ui->maxiteration_spinbox->value());
  anything_changed |= ok;
  if (!ok) {
    ui->maxiteration_spinbox->setValue(ui->canvas->getMaxIterations());
  }
  ok = ui->canvas->setThreshold(ui->threshold_spinbox->value());
  anything_changed |= ok;
  if (!ok) {
    ui->threshold_spinbox->setValue(ui->canvas->getThreshold());
  }
  if (anything_changed) {
    ui->canvas->redraw();
  }
}
