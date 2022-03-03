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

#include "../src/unixcommand.h"
#include "../src/wmhelper.h"
#include "../src/strconstants.h"
#include "../src/argumentlist.h"
#include "mainwindow.h"

#include "../src/QtSolutions/qtsingleapplication.h"
//#include <QApplication>
#include <QtGui>
#include <QMessageBox>
#include <QDebug>

#define NO_GTK_STYLE

int main(int argc, char *argv[])
{  
  bool debugInfo = false;

  ArgumentList *argList = new ArgumentList(argc, argv);
  if (argList->getSwitch(QStringLiteral("-d")))
  {
    //If user chooses to switch debug info on...
    debugInfo = true;
  }

  if (debugInfo)
    qDebug() << QString(QLatin1String("Octopi Notifier - ") + ctn_APPLICATION_VERSION +
                  QLatin1String(" (") + StrConstants::getQtVersion() + QLatin1String(")"));

  /*if (UnixCommand::isAppRunning(QStringLiteral("octopi-notifier")))
  {
    qDebug() << "Aborting notifier as another instance is already running!";
    return (-1);
  }*/

  if (!QFile::exists(ctn_CHECKUPDATES_BINARY))
  {
    qDebug() << "Aborting notifier as 'checkupdates' binary could not be found! [" << ctn_CHECKUPDATES_BINARY << "]";
    return (-2);
  }

  if (!QFile::exists(ctn_OCTOPI_HELPER_PATH))
  {
    qDebug() << "Aborting notifier as 'octphelper' binary could not be found! [" << ctn_OCTOPI_HELPER_PATH << "]";
    return (-3);
  }

  if (!QFile::exists(ctn_OCTOPISUDO))
  {
    qDebug() << "Aborting notifier as 'octopi-sudo' binary could not be found! [" << ctn_OCTOPISUDO << "]";
    return (-4);
  }

  //QApplication a(argc, argv);
  QtSingleApplication a(QLatin1String("NotifierOcto"), argc, argv);

  if (a.isRunning())
  {
    if (argList->getSwitch(QStringLiteral("-checkupdates")))
    {
      a.sendMessage(QStringLiteral("NOTIFIER_CHECKUPDATES"));
    }

    return 0;
  }
  else if (argList->getSwitch(QStringLiteral("-checkupdates")))
  {
    return -7; //We are not running, so nothing to check...
  }

  QTranslator appTranslator;
  appTranslator.load(QLatin1String(":/resources/translations/octopi_") +
                     QLocale::system().name());
  a.installTranslator(&appTranslator);
  a.setQuitOnLastWindowClosed(false);

  if (!UnixCommand::isOctoToolRunning(QStringLiteral("octopi-notifier")))
  {
    QMessageBox::critical(nullptr, StrConstants::getApplicationName(), StrConstants::getErrorRunOctopiNotifierAsUsrBin());
    return (-6);
  }

  if (UnixCommand::isRootRunning()){
    QMessageBox::critical(nullptr, StrConstants::getApplicationName(), StrConstants::getErrorRunningWithRoot());
    return (-5);
  }

  unsetenv("TMPDIR");

  setenv("COLORTERM", "truecolor", 1);
  setenv("TERM", "xterm-256color", 1);

  QString buildDir=SettingsManager::getAURBuildDir();
  if (!buildDir.isEmpty())
  {
    setenv("BUILDDIR", buildDir.toLatin1().data(), 1);
  }

  MainWindow w;

  QObject::connect(&a, SIGNAL(notifierCheckUpdates()), &w, SLOT(doCheckUpdates()));

  a.setActivationWindow(&w);
  a.setQuitOnLastWindowClosed(false);

  if (w.startServer())
  {
    QResource::registerResource(QStringLiteral("./resources.qrc"));

    QGuiApplication::setDesktopFileName(QStringLiteral("octopi-notifier"));

    if (debugInfo)
      w.turnDebugInfoOn();

    return a.exec();
  }
}
