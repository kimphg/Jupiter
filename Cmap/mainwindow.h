#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cmap.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    CMap *map;

    bool isPressed;
protected:
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
private slots:
    void on_lineEdit_returnPressed();

private:
    double mScale;
    Ui::MainWindow *ui;
    QPoint pressPos;
    int dxMap;
    int dyMap;

    void drawCrossHairMark(int x, int y, QPainter *p);
};

#endif // MAINWINDOW_H
