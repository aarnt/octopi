/*
Copyright 2015 Michaël Lhomme

This file is part of AppSet.

AppSet is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

AppSet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with AppSet; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "cachecleaner.h"
#include "../../src/strconstants.h"
#include "../src/QtSolutions/qtsingleapplication.h"

#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QtGui>

int main( int argc, char *argv[] )
{
  QtSingleApplication app( "Cache Cleaner - Octopi", argc, argv );

  //If there is already an instance running...
  if (app.isRunning())
  {
    app.sendMessage("RAISE");
    return 0;
  }

  app.sendMessage("RAISE");

  QTranslator appTranslator;
  appTranslator.load(":/resources/translations/octopi_cachecleaner_" +
                     QLocale::system().name());
  app.installTranslator(&appTranslator);

  if (UnixCommand::isRootRunning()){
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorRunningWithRoot());
    return (-2);
  }

  if (!UnixCommand::hasTheExecutable("paccache"))
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getExecutableCouldNotBeFound().arg("\"paccache\""));
    return (-3);
  }

  if (!QFile::exists(ctn_OCTOPI_HELPER_PATH))
  {
    qDebug() << "Aborting cache-cleaner as 'octphelper' binary could not be found! [" << ctn_OCTOPI_HELPER_PATH << "]";
    return (-4);
  }

  CacheCleaner w;
  if (w.startServer())
  {
    app.setActivationWindow(&w);
    w.show();
    QResource::registerResource("./resources.qrc");

    return app.exec();
  }
}
