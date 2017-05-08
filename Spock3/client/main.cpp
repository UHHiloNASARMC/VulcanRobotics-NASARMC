#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
