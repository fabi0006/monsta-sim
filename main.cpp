#include <QtWidgets/QApplication>
#include "GUI/MainWindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Monsta Simulator");
    w.show();
    return a.exec();
}
