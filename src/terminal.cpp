/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2014 Alexandre Albuquerque Arnt
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

#include "terminal.h"
#include "wmhelper.h"
#include "unixcommand.h"
#include "strconstants.h"

#include <QApplication>
#include <QProcess>
#include <QTextStream>
#include <iostream>

/*
 * This class abstracts all the relevant terminal related code
 */

Terminal::Terminal(QObject *parent) : QObject(parent)
{
}

Terminal::~Terminal()
{
}

/*
 * Executes the given command list with root credentials
 */
void Terminal::runCommandInTerminalWithSudo(const QString& command)
{
  QString cmd = QLatin1String("sudo ") + UnixCommand::getShell() + QLatin1String(" -c \"") + command + QLatin1Char('"');
  emit commandToExecInQTermWidget(cmd);
}

/*
 * Executes the given commandList creating a ROOT temp transaction file and calling "octopi-helper -ts"
 */
void Terminal::runOctopiHelperInTerminalWithSharedMem(const QStringList &commandList, QSharedMemory *sharedMem)
{
  QString out;
  bool removedLines = false;

  foreach(QString line, commandList)
  {
    if (line.contains(QLatin1String("echo -e")) || line.contains(QLatin1String("read -n 1")))
    {
      removedLines = true;
      continue;
    }

    out += line + QLatin1Char('\n');
  }

  if (removedLines) out += QLatin1String("echo \"") + StrConstants::getPressAnyKey() + QLatin1String("\"");

  QString suCommand = WMHelper::getSUCommand();
  QString commandToRun = ctn_OCTOPI_HELPER_PATH + QLatin1String(" -ts");
  QString cmd = QLatin1String("sudo ") + commandToRun;
  QByteArray sharedData=out.toLatin1();

  /*if (sharedMem != nullptr)
  {
    if (sharedMem->isAttached())
      sharedMem->detach();
    delete sharedMem;
    sharedMem=nullptr;
  }*/

  //UnixCommand::removeSharedMemFiles();

  //sharedMem=new QSharedMemory("org.arnt.octopi", this);
  sharedMem->create(sharedData.size());
  sharedMem->lock();
  memcpy(sharedMem->data(), sharedData.data(), sharedData.size());
  sharedMem->unlock();

  emit commandToExecInQTermWidget(cmd);
}

/*
 * Executes the given command list with root credentials using "octopi-helper -t"
 */
/*void Terminal::runOctopiHelperInTerminal(const QStringList &commandList)
{
  QFile *ftemp = UnixCommand::generateTemporaryFile();
  QTextStream out(ftemp);
  bool removedLines = false;

  foreach(QString line, commandList)
  {
    if ((line.contains("echo -e") || line.contains("read -n 1"))) //&& m_selectedTerminal == ctn_QTERMWIDGET)
    {
      removedLines = true;
      continue;
    }

    out << line << "\n";
  }

  if (removedLines) out << "echo \"" << StrConstants::getPressAnyKey() + "\"";

  out.flush();
  ftemp->close();

  QString suCommand = WMHelper::getSUCommand();
  QString commandToRun = ctn_OCTOPI_HELPER + " -t";
  QString cmd = "sudo " + commandToRun;
  emit commandToExecInQTermWidget(cmd);
}*/

/*
 * Executes the given command list as normal user
 */
void Terminal::runCommandInTerminalAsNormalUser(const QStringList &commandList)
{
  //QFile *ftemp = UnixCommand::generateTemporaryFile();
  //QTextStream out(ftemp);
  QString out;
  bool removedLines = false;

  foreach(QString line, commandList)
  {
    if ((line.contains(QLatin1String("echo -e")) || line.contains(QLatin1String("read -n 1")))) //&& m_selectedTerminal == ctn_QTERMWIDGET)
    {
      removedLines = true;
      continue;
    }

    //We must remove the "ccr/" prefix in Chakra, cos this will not work
    if(line.contains(QLatin1String("ccr/")))
    {
      line = line.replace(QLatin1String("ccr/"), QLatin1String(""));
    }

    out += line;
  }

  if (removedLines) out += QLatin1String("echo '") + StrConstants::getPressAnyKey() + QLatin1Char('\'');

  //out.flush();
  //ftemp->close();

  QString cmd;
  cmd = UnixCommand::getShell() + QLatin1String(" -c \"") + out /*ftemp->fileName()*/ + QLatin1Char('"');
  emit commandToExecInQTermWidget(cmd);
}
