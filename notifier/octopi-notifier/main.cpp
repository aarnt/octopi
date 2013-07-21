#include "mainwindow.h"
#include <QtGui>

int main(int argc, char *argv[])
{
  if (MainWindow::isAppRunning("octopi-notifier")) return (-1);

  QApplication a(argc, argv);

  QApplication::setGraphicsSystem(QLatin1String("raster"));
  qApp->setStyle(new QGtkStyle());

  QTranslator appTranslator;
  appTranslator.load(":/resources/translations/octopi_" +
                     QLocale::system().name());
  a.installTranslator(&appTranslator);

  MainWindow w;
  QResource::registerResource("./resources.qrc");
  return a.exec();
}
