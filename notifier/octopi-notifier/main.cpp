#include "mainwindow.h"
#include <QtGui>

int main(int argc, char *argv[])
{
  QStringList slParam;
  QProcess proc;
  slParam << "-C";
  slParam << "octopi-notifier";
  proc.start("ps", slParam);
  proc.waitForFinished();
  QString out = proc.readAll();
  proc.close();
  if (out.count("octopi-notifier")>1) return(-1);

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
