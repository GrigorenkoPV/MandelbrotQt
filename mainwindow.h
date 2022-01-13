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

  // todo: polish the menu
  // todo: add threshold selector
  // todo: two reset buttons

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() final;

 private:
  void resetMenu();

 public slots:
  void reset();
  void applySettings();
};
