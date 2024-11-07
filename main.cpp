#include "mainwindow.h"
#include "ekran.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MainWindow w;
    //w.show();

    Ekran e;
    e.show();

    return a.exec();
}
