/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2013 Alexandre Albuquerque Arnt
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#include "mainwindow.h"
#include "argumentlist.h"
#include "strconstants.h"
#include "unixcommand.h"
#include "wmhelper.h"
#include <iostream>
#include "QtSolutions/qtsingleapplication.h"
#include <QtGui>

int main(int argc, char *argv[])
{
  ArgumentList *argList = new ArgumentList(argc, argv);
  QApplication::setGraphicsSystem(QLatin1String("raster"));

  if (!argList->getSwitch("-style"))
  {
    if (WMHelper::isKDERunning())
    {
      //QApplication::setStyle(new QPlastiqueStyle());
    }
    else if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX)
    {
      QApplication::setStyle(new QGtkStyle());
    }
    else
    {
      QApplication::setStyle(new QCleanlooksStyle());
    }
  }
  else
  {
    if (argList->contains("cleanlooks", Qt::CaseInsensitive))
    {
      QCleanlooksStyle *qls = new QCleanlooksStyle();
      qls->setObjectName("cleanLooksStyle");
      QApplication::setStyle(qls);
    }
  }

  QtSingleApplication app( StrConstants::getApplicationName(), argc, argv );

  //This sends a message just to awake the socket-based QtSinleApplication engine
  app.sendMessage("ping app...");

  if (app.isRunning()) return 0;

  QTranslator appTranslator;
  appTranslator.load(":/resources/translations/octopi_" +
                     QLocale::system().name());
  app.installTranslator(&appTranslator);

  if (argList->getSwitch("-help")){
    std::cout << StrConstants::getApplicationCliHelp().toAscii().data() << std::endl;
    return(0);
  }
  else if (argList->getSwitch("-version")){
    std::cout << "\n" << StrConstants::getApplicationName().toAscii().data() <<
                 " " << StrConstants::getApplicationVersion().toAscii().data() << "\n" << std::endl;
    return(0);
  }

  if (UnixCommand::isRootRunning() && !WMHelper::isKDERunning()){
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorRunningWithRoot());
    return ( -2 );
  }

  MainWindow w;
  app.setActivationWindow(&w);
  app.setQuitOnLastWindowClosed(false);

  if (argList->getSwitch("-sysupgrade"))
  {
    w.setCallSystemUpgrade();
  }

  w.setRemoveCommand(argList->getSwitchArg("-removecmd", "Rcs"));

  w.show();

  QResource::registerResource("./resources.qrc");

  return app.exec();
}
