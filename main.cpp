#include "Main Window/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(w.width()*1.4, w.height());
    w.show();
    return a.exec();
}
