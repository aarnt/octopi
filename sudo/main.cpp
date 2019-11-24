/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2015 LXQt team
 * Authors:
 *   Palo Kisa <palo.kisa@gmail.com>
 *
 * Copyright: 2019 Octopi team
 * Authors:
 *   Alexandre Albuquerque Arnt
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

/*
 * This code is adapted from LXQt version 0.14.1
 *
 * It is not using LXQt libs and is intended to act as the *only* sudo tool supported in Octopi
 */

#include "sudo.h"
#include "../src/argumentlist.h"
#include <unistd.h>
#include <iostream>

#include <QApplication>
#include <QTranslator>
#include <QFile>
#include <QTextStream>

/*
 * Saves a file called "octopihelper" at /etc/sudoers.d so any user member of wheel group can run Octopi without entering a password
 */
void setNoPasswdUse()
{
  QString cmd = "Cmnd_Alias  OCTOPIHELPER = /bin/sh -c unset LC_ALL; "
      "exec '/usr/lib/octopi/octopi-helper' '-ts', "
      "/usr/lib/octopi/octopi-helper\n\n";
  cmd += "%wheel ALL=(root) NOPASSWD:SETENV:OCTOPIHELPER\n";

  QFile file("/etc/sudoers.d/octopihelper");

  if (file.exists()) file.remove();

  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    QTextStream out(&file);
    out << cmd;
    file.close();

    std::cout << "octopi-sudo" << ": \"/etc/sudoers.d/octopihelper\" was created successfully." << std::endl;
  }
  else
  {
    std::cout << "octopi-sudo" << ": \"/etc/sudoers.d/octopihelper\" could not be created." << std::endl;
  }
}

int main(int argc, char **argv)
{
  //ArgumentList *argList = new ArgumentList(argc, argv);
  QApplication app(argc, argv, true);
  app.setQuitOnLastWindowClosed(false);
  app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

  /*if (argList->getSwitch("-setnopasswd"))
  {
    int uid = geteuid();
    if (uid == 0) //octopi-sudo is running as root
    {
      setNoPasswdUse();
      return 0;
    }
    else
    {
      std::cout << "octopi-sudo" << ": You need to run as root to exec with this parameter." << std::endl;
      return -1;
    }
  }*/

  QTranslator translator;
  // look up e.g. :/translations/myapp_de.qm
  if (translator.load(QLocale(), QLatin1String("lxqt-sudo"), QLatin1String("_"), QLatin1String(":/translations")))
    app.installTranslator(&translator);

  Sudo s;
  return s.main();
}
