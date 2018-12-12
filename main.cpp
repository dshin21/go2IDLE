/*------------------------------------------------------------------------------------------------------------------
--SOURCE FILE:  main.cpp -  Entry point of the program
--
--PROGRAM:      Radio Modem Protocol Driver
--
--FUNCTIONS:
--              int main(int argc, char *argv[])
--
--DATE:         Nov 22, 2018    Daniel Shin - created
--
--DESIGNER:     Daniel Shin
--
--PROGRAMMER:   Daniel Shin
--
--NOTES:
--Automatic generated entry point for Qt applications.
----------------------------------------------------------------------------------------------------------------------*/
#include "go2idle.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    go2IDLE w;
    w.show();

    return a.exec();
}
