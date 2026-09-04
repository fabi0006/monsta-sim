#include <QtWidgets/QApplication>
#include "GUI/MainWindow.h"
#include "GUI/MonstaWidget.h"

void startButtonClicked();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    MonstaWidget m;
    m.startButtonClicked();
    w.setWindowTitle("Monsta Simulator");
    w.show();
    return a.exec();
}
