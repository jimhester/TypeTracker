//#include "typeTracker.h"
//#include <QtGui/QApplication>
//#include <QtGui>
//
//int main(int argc, char *argv[])
// {
//    QApplication a(argc, argv);
//    if (!QSystemTrayIcon::isSystemTrayAvailable()) 
//    {
//        QMessageBox::critical(0, QObject::tr("TypeTracker"),
//        QObject::tr("I couldn't detect any system tray on this system."));
//        return 1;
//    }
//    TypeTracker w;
//    w.show();
//    return a.exec();
//}
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