/************************************************************
*
*qt5.10.0  多文档编辑器   MartinPei  2019.6.15
*
************************************************************/

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
