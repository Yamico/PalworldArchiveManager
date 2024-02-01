#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon("icon.ico"));

    QTranslator translator;
    translator.load(":/PalArchiveManager_en_US.qm");
    a.installTranslator(&translator);
    MainWindow w;
    w.show();
    return a.exec();

}
