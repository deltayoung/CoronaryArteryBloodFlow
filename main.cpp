#include "viewerwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ViewerWidget w;
    w.show();

    return a.exec();
}
