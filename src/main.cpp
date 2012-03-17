#include <QApplication>

#include "typeTracker.h"

int main(int argc, char *argv[])
{

   QApplication app(argc, argv);
//app.setOrganizationName("Trolltech");
   app.setApplicationName("typeTracker");
   TypeTracker w;
    w.show();
    return app.exec();
}

