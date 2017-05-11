#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
