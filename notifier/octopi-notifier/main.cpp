#include "../../src/unixcommand.h"
#include "../../src/wmhelper.h"
#include "mainwindow.h"

#include <QtGui>

int main(int argc, char *argv[])
{
  if (UnixCommand::isAppRunning("octopi-notifier")) return (-1);

  QApplication a(argc, argv);

  QApplication::setGraphicsSystem(QLatin1String("raster"));

  if(UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX &&
          !WMHelper::isKDERunning())
  {
    qApp->setStyle(new QGtkStyle());
  }
  else
    qApp->setStyle(new QCleanlooksStyle);

  QTranslator appTranslator;
  appTranslator.load(":/resources/translations/octopi_" +
                     QLocale::system().name());
  a.installTranslator(&appTranslator);
  a.setQuitOnLastWindowClosed(false);
  MainWindow w;
  QResource::registerResource("./resources.qrc");
  return a.exec();
}
