#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("磁力連結產生器");
    QApplication::setOrganizationName("MagnetTools");

    MainWindow window;
    window.show();

    return QApplication::exec();
}
