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
#include "../src/strconstants.h"
#include "../src/unixcommand.h"
#include "../src/QtSolutions/qtsingleapplication.h"

#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QtGui>

int main( int argc, char *argv[] )
{
  unsetenv("TMPDIR");
  QtSingleApplication app( QStringLiteral("Repository Editor - Octopi"), argc, argv );

  //If there is already an instance running...
  if (app.isRunning())
  {
    app.sendMessage(QStringLiteral("RAISE"));
    return 0;
  }

  app.sendMessage(QStringLiteral("RAISE"));

  QTranslator appTranslator;
  appTranslator.load(QLatin1String(":/resources/translations/octopi_repoeditor_") +
                     QLocale::system().name());
  app.installTranslator(&appTranslator);

  if (UnixCommand::isRootRunning()){
    QMessageBox::critical( 0, QStringLiteral("Repository Editor - Octopi"),
                           QObject::tr("You can not run Repository Editor with administrator's credentials."));
    return (-2);
  }

  if (!QFile::exists(ctn_OCTOPISUDO))
  {
    qDebug() << "Aborting Repository Editor as 'octopi-sudo' binary could not be found! [" << ctn_OCTOPISUDO << "]";
    return (-3);
  }

  RepoEditor w;
  app.setActivationWindow(&w);
  w.show();

  QResource::registerResource(QStringLiteral("./resources.qrc"));

  return app.exec();
}
