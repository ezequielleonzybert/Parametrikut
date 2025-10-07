#include "Parametrikut.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Parametrikut window;
    window.setWindowIcon(QIcon(":/Parametrikut/logo/logo.png"));
    window.show();
    return app.exec();
}
