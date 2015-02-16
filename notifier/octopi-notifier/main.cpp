#include "../../src/unixcommand.h"
#include "../../src/wmhelper.h"
#include "../../src/strconstants.h"
#include "mainwindow.h"

#include <QApplication>
#include <QtGui>
#include <QDebug>

//#define NO_GTK_STYLE

int main(int argc, char *argv[])
{  
  qDebug() << QString("Octopi Notifier - " + StrConstants::getApplicationVersion() +
                " (" + StrConstants::getQtVersion() + ")");

  if (UnixCommand::isAppRunning("octopi-notifier"))
  {
    qDebug() << "Aborting notifier as another instance is already running!";
    return (-1);
  }

  if (!QFile::exists(ctn_PACMANHELPER_BINARY))
  {
    qDebug() << "Aborting notifier as 'pacmanhelper' binary could not be found!";
    return (-2);
  }

  if (!QFile::exists(ctn_DBUS_PACMANHELPER_SERVICE))
  {
    qDebug() << "Aborting notifier as 'pacmanhelper' DBus service could not be found!";
    return (-3);
  }

  QApplication a(argc, argv);

#if QT_VERSION < 0x050000
  #ifndef NO_GTK_STYLE
  QApplication::setGraphicsSystem(QLatin1String("raster"));

  if(!WMHelper::isKDERunning() && (!WMHelper::isLXQTRunning()) && (!WMHelper::isRazorQtRunning()))
  {
    qApp->setStyle(new QGtkStyle());
  }
  else
    qApp->setStyle(new QCleanlooksStyle);
  #endif
#endif

  QTranslator appTranslator;
  appTranslator.load(":/resources/translations/octopi_" +
                     QLocale::system().name());

  a.installTranslator(&appTranslator);
  a.setQuitOnLastWindowClosed(false);

  MainWindow w;
  QResource::registerResource("./resources.qrc");

  return a.exec();
}
