/*
* This file is part of Octopi, an open-source GUI for ArchLinux pacman.
* Copyright (C) 2013  Alexandre Albuquerque Arnt
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

#include "QtSolutions/qtsingleapplication.h"
#include "mainwindow.h"
#include "argumentlist.h"
#include "strconstants.h"

#include <iostream>
#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
  ArgumentList *argList = new ArgumentList(argc, argv);
  QApplication::setGraphicsSystem(QLatin1String("raster"));

  if (!argList->getSwitch("-style"))
    QApplication::setStyle(new QCleanlooksStyle());

  QtSingleApplication app( StrConstants::getApplicationName(), argc, argv );

  //This sends a message just to awake the socket-based QtSinleApplication engine
  app.sendMessage("ping app...");

  if (app.isRunning())
    return 0;

  /*QTranslator appTranslator;
    appTranslator.load(":/resources/translations/qtgzmanager_" +
      QLocale::system().name());
    app.installTranslator(&appTranslator);*/

  if (argList->getSwitch("-help")){
    std::cout << StrConstants::getApplicationCliHelp().toAscii().data() << std::endl;
    return(0);
  }
  else if (argList->getSwitch("-version")){
    std::cout << "\n" << StrConstants::getApplicationName().toAscii().data() << "\n" << std::endl;
    return(0);
  }

  MainWindow w;
  app.setActivationWindow(&w);
  w.show();

  QResource::registerResource("./resources.qrc");

  return app.exec();
}
