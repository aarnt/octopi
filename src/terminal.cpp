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

Terminal::Terminal(QObject *parent, const QString &selectedTerminal) : QObject(parent)
{
  m_process = new QProcess(parent);
  m_process->setInputChannelMode(QProcess::ForwardedInputChannel);
  m_process->setProcessChannelMode(QProcess::ForwardedChannels);

  //Make the needed signal propagations...
  connect(m_process, SIGNAL(started()), this, SIGNAL(started()));
  connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SIGNAL(finished(int,QProcess::ExitStatus)));

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  m_process->setProcessEnvironment(env);
  m_selectedTerminal = selectedTerminal;
}

Terminal::~Terminal()
{
  m_process->close();
  delete m_process;
}

/*
 * Executes the given command list with root credentials
 */
void Terminal::runCommandInTerminal(const QStringList &commandList)
{
  QFile *ftemp = UnixCommand::generateTemporaryFile();
  QTextStream out(ftemp);
  bool removedLines = false;

  foreach(QString line, commandList)
  {
    if ((line.contains("echo -e") || line.contains("read -n 1")) && m_selectedTerminal == ctn_QTERMWIDGET)
    {
      removedLines = true;
      continue;
    }

    out << line;
  }

  if (removedLines) out << "echo \"" << StrConstants::getPressAnyKey() + "\"";

  out.flush();
  ftemp->close();

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove("LANG");
  env.remove("LC_MESSAGES");
  env.insert("LANG", QLocale::system().name() + ".UTF-8");
  env.insert("LC_MESSAGES", QLocale::system().name() + ".UTF-8");
  m_process->setProcessEnvironment(env);
  m_process->close();

  QString cmd = UnixCommand::getShell() + " -c \"" + ftemp->fileName() + "\"";
  emit commandToExecInQTermWidget(cmd);
}

/*
 * Executes the given command list with root credentials using "octopi-helper -t"
 */
void Terminal::runOctopiHelperInTerminal(const QStringList &commandList)
{
  QFile *ftemp = UnixCommand::generateTemporaryFile();
  QTextStream out(ftemp);
  bool removedLines = false;

  foreach(QString line, commandList)
  {
    if ((line.contains("echo -e") || line.contains("read -n 1")) && m_selectedTerminal == ctn_QTERMWIDGET)
    {
      removedLines = true;
      continue;
    }

    out << line << "\n";
  }

  if (removedLines) out << "echo \"" << StrConstants::getPressAnyKey() + "\"";

  out.flush();
  ftemp->close();

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove("LANG");
  env.remove("LC_MESSAGES");
  env.insert("LANG", QLocale::system().name() + ".UTF-8");
  env.insert("LC_MESSAGES", QLocale::system().name() + ".UTF-8");
  m_process->setProcessEnvironment(env);

  QString suCommand = WMHelper::getSUCommand();
  QString commandToRun = ctn_OCTOPI_HELPER + " -t";
  m_process->close();
  QString cmd = "sudo " + commandToRun;
  emit commandToExecInQTermWidget(cmd);
}

/*
 * Executes the given command list as normal user
 */
void Terminal::runCommandInTerminalAsNormalUser(const QStringList &commandList)
{
  QFile *ftemp = UnixCommand::generateTemporaryFile();
  QTextStream out(ftemp);
  bool removedLines = false;

  foreach(QString line, commandList)
  {
    if ((line.contains("echo -e") || line.contains("read -n 1")) && m_selectedTerminal == ctn_QTERMWIDGET)
    {
      removedLines = true;
      continue;
    }

    //We must remove the "ccr/" prefix in Chakra, cos this will not work
    if(line.contains("ccr/"))
    {
      line = line.replace("ccr/", "");
    }

    out << line;
  }

  if (removedLines) out << "echo \"" << StrConstants::getPressAnyKey() + "\"";

  out.flush();
  ftemp->close();

  QString cmd;
  m_process->close();
  cmd = UnixCommand::getShell() + " -c \"" + ftemp->fileName() + "\"";
  emit commandToExecInQTermWidget(cmd);
}
