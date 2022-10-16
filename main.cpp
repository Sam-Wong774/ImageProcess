#include "stdafx.h"
#include "ImageProcess.h"
#include <QtWidgets/QApplication>
#include <crtdbg.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageProcess w;
    w.show();   
    return a.exec();
     _CrtDumpMemoryLeaks();
}
