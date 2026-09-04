#include <QtWidgets/QApplication>
#include "GUI/MainWindow.h"
//#include "GUI/MonstaWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //MonstaWidget *m;
    w.setWindowTitle("Monsta Simulator");
    w.show();
     
    return a.exec();
}
