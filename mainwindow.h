#pragma once
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
  Q_OBJECT

  Ui::MainWindow *ui;

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() final;

 private:
  void resetMenu();

 public slots:
  void reset();
  void resetPan();
  void applySettings();
};
