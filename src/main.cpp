#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));  // 跨平台统一极简风格
    MainWindow w;
    w.show();
    return app.exec();
}
