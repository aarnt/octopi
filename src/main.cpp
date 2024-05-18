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
#include <iostream>

#include "QtSolutions/qtsingleapplication.h"
#include <QtGui>
#include <QMessageBox>

int main(int argc, char *argv[])
{
  if (!QFile::exists(ctn_CHECKUPDATES_BINARY))
  {
    qDebug() << "Aborting octopi as 'checkupdates' binary could not be found! [" << ctn_CHECKUPDATES_BINARY << "]";
    return (-1);
  }

  if (!QFile::exists(ctn_OCTOPI_HELPER_PATH))
  {
    qDebug() << "Aborting octopi as 'octphelper' binary could not be found! [" << ctn_OCTOPI_HELPER_PATH << "]";
    return (-2);
  }

  if (!QFile::exists(ctn_OCTOPISUDO))
  {
    qDebug() << "Aborting octopi as 'qt-sudo' binary could not be found! [" << ctn_OCTOPISUDO << "]";
    return (-3);
  }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

  ArgumentList *argList = new ArgumentList(argc, argv);
  QString packagesToInstall;
  QString arg;

  for (int c=1; c<argc; c++)
  {
    arg = QString::fromLocal8Bit(argv[c]);
    if (arg.contains(QRegularExpression(QStringLiteral("pkg.tar.[xz|zst]"))))
    {
      packagesToInstall += arg + QLatin1Char(',');
    }
  }

  unsetenv("TMPDIR");

  QtSingleApplication app( StrConstants::getApplicationName(), argc, argv );

  if (app.isRunning())
  {
    if (argList->getSwitch(QStringLiteral("-checkupdates")))
    {
      app.sendMessage(QStringLiteral("CHECKUPDATES"));
    }
    if (argList->getSwitch(QStringLiteral("-aurupgrade")))
    {
      app.sendMessage(QStringLiteral("AURUPGRADE"));
    }
    else if (argList->getSwitch(QStringLiteral("-sysupgrade")))
    {
      app.sendMessage(QStringLiteral("SYSUPGRADE"));
    }
    else if (argList->getSwitch(QStringLiteral("-sysupgrade-noconfirm")))
    {
      app.sendMessage(QStringLiteral("SYSUPGRADE_NOCONFIRM"));
    }
    else if (argList->getSwitch(QStringLiteral("-close")))
    {
      app.sendMessage(QStringLiteral("CLOSE"));
    }
    else if (argList->getSwitch(QStringLiteral("-hide")))
    {
      app.sendMessage(QStringLiteral("HIDE"));
    }
    else if (argList->getSwitch(QStringLiteral("-options")))
    {
      app.sendMessage(QStringLiteral("OPTIONS"));
    }
    else if (argList->getSwitch(QStringLiteral("-show")))
    {
      app.sendMessage(QStringLiteral("SHOW"));
    }
    else if (!packagesToInstall.isEmpty())
    {
      app.sendMessage(packagesToInstall);
    }
    else
    {
      app.sendMessage(QStringLiteral("RAISE"));
    }

    return 0;
  }
  else if (argList->getSwitch(QStringLiteral("-checkupdates")))
  {
    return -6; //We are not running, so nothing to check...
  }

  //This sends a message just to enable the socket-based QtSingleApplication engine
  app.sendMessage(QStringLiteral("RAISE"));

  QTranslator appTranslator;
  bool success = appTranslator.load(QLatin1String(":/resources/translations/octopi_") +
                     QLocale::system().name());
  if (!success)
  {
    appTranslator.load(QStringLiteral(":/resources/translations/octopi_en.qm"));
  }

  app.installTranslator(&appTranslator);

  if (argList->getSwitch(QStringLiteral("-help"))){
    std::cout << StrConstants::getApplicationCliHelp().toLatin1().data() << std::endl;
    delete argList;
    return(0);
  }
  else if (argList->getSwitch(QStringLiteral("-version"))){
    std::cout << "\n" << StrConstants::getApplicationName().toLatin1().data() <<
                 " " << ctn_APPLICATION_VERSION.toLatin1().data() << "\n" << std::endl;
    delete argList;
    return(0);
  }

  if (!UnixCommand::isOctoToolRunning(QStringLiteral("octopi")))
  {
    QMessageBox::critical(nullptr, StrConstants::getApplicationName(), StrConstants::getErrorRunOctopiAsUsrBin());
    delete argList;
    return (-5);
  }

  if (UnixCommand::isRootRunning())
  {
    QMessageBox::critical(nullptr, StrConstants::getApplicationName(), StrConstants::getErrorRunningWithRoot());
    delete argList;
    return (-4);
  }

  setenv("COLORTERM", "truecolor", 1);
  setenv("TERM", "xterm-256color", 1);

  QString buildDir=SettingsManager::getAURBuildDir();
  if (!buildDir.isEmpty())
  {
    setenv("BUILDDIR", buildDir.toLatin1().data(), 1);
  }

  MainWindow w;
  QObject::connect(&app, SIGNAL(options()), &w, SLOT(onOptions()));
  QObject::connect(&app, SIGNAL(checkUpdates()), &w, SLOT(doCheckUpdates()));
  QObject::connect(&app, SIGNAL(AURUpgrade()), &w, SLOT(doAURUpgrade()));
  QObject::connect(&app, SIGNAL(systemUpgrade(SystemUpgradeOptions)), &w, SLOT(doSystemUpgrade(SystemUpgradeOptions)));
  QObject::connect(&app, SIGNAL(installLocalPackages(QStringList)), &w, SLOT(droppedLocalPackages(QStringList)));

  if (w.startServer())
  {
    app.setActivationWindow(&w);
    app.setQuitOnLastWindowClosed(false);

    if (argList->getSwitch(QStringLiteral("-sysupgrade-noconfirm")))
    {
      w.setCallSystemUpgradeNoConfirm();
    }
    else if (argList->getSwitch(QStringLiteral("-sysupgrade")))
    {
      w.setCallSystemUpgrade();
    }

    if (argList->getSwitch(QStringLiteral("-d")))
    {
      //If user chooses to switch debug info on...
      w.turnDebugInfoOn();
    }

    w.setRemoveCommand(QStringLiteral("Rcs"));
    w.show();

    QResource::registerResource(QStringLiteral("./resources.qrc"));

    return app.exec();
  }
}
