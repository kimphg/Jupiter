#include "mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //a.setOverrideCursor(QCursor(QPixmap("img\crossCursor.png")));
    Mainwindow w;
    //w.showFullScreen();
    //w.on_MainWindow_SizeChanged();
    w.show();
    //w.on_MainWindow_SizeChanged();
    return a.exec();
}
