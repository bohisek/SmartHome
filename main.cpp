#include "mainwindow.h"
#include <QApplication>
#include <wiringPi.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    system("gpio export 16 in");   // BCM numbering, pin set as input
    system("gpio export 16 up");   // brings pin voltage up to 3.3V

    system("gpio export 6 out");    // BCM numbering, RELAY 1
    system("gpio export 13 out");   // BCM numbering, RELAY 2
    system("gpio export 19 out");   // BCM numbering, RELAY 3
    system("gpio export 26 out");   // BCM numbering, RELAY 4
    wiringPiSetupSys();


    return a.exec();
}
