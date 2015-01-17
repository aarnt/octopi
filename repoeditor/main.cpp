/*
Copyright 2011 Simone Tobia

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

#include "repoeditor.h"
#include "repoconf.h"

#include "../src/QtSolutions/qtsingleapplication.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QtGui>

int main( int argc, char *argv[] )
{
  QtSingleApplication app( "Repository Editor - Octopi", argc, argv );

  //If there is already an instance running...
  if (app.isRunning())
  {
    app.sendMessage("RAISE");
    return 0;
  }

  app.sendMessage("RAISE");

  QTranslator appTranslator;
  appTranslator.load(":/resources/translations/octopi_repoeditor_" +
                     QLocale::system().name());
  app.installTranslator(&appTranslator);

  RepoEditor w;
  app.setActivationWindow(&w);
  w.show();

  QResource::registerResource("./resources.qrc");

  return app.exec();
}
