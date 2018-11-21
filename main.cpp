#include "go2idle.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    go2IDLE w;
    w.show();

    return a.exec();
}
