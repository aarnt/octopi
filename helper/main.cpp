/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2019 Alexandre Albuquerque Arnt
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

#include "octopihelper.h"
#include "../src/argumentlist.h"
#include <unistd.h>

#include <QCoreApplication>
#include <QTextStream>

bool isRootRunning()
{
  int uid = geteuid();
  return (uid == 0);
}

int main(int argc, char *argv[])
{
  if (!isRootRunning())
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Only root can run..." << endl;
    return ctn_NO_ROOT_RUNNING;
  }

  ArgumentList *argList = new ArgumentList(argc, argv);
  QCoreApplication a(argc, argv);
  OctopiHelper helper;

  if (argList->getSwitch("-ts"))
    return helper.executePkgTransactionWithSharedMem();
  else
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Suspicious execution method" << endl;
    return ctn_SUSPICIOUS_EXECUTION_METHOD;
  }
}
