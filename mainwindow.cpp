#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "params.h"

using namespace mandelbrot;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  ui->maxiteration_spinbox->setMinimum(MIN_MAX_ITERATIONS);
  ui->maxiteration_spinbox->setMaximum(MAX_MAX_ITERATIONS);
  connect(ui->apply_button, &QPushButton::clicked, this,
          &MainWindow::applySettings);
  connect(ui->reset_button, &QPushButton::clicked, this, &MainWindow::reset);
  resetMenu();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::resetMenu() {
  ui->maxiteration_spinbox->setValue(ui->canvas->getMaxIterations());
}

void MainWindow::reset() {
  ui->canvas->reset();
  resetMenu();
}
void MainWindow::applySettings() {
  ui->canvas->setMaxIterations(ui->maxiteration_spinbox->value());
  ui->canvas->redraw();
}
