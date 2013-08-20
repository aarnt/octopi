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

/*
 * A wrapper for running a QProcess while providing feedback of its state
 *
 * IT ONLY WORKS with terminal commands that start other subcommands, with an "-e" option
 */
#include "processwrapper.h"

#include <QProcess>
#include <QTimer>

/*
 * The needed constructor
 */
ProcessWrapper::ProcessWrapper(QObject *parent) :
  QObject(parent)
{
  m_process = new QProcess(parent);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  m_process->setProcessEnvironment(env);

  m_timer = new QTimer(parent);
  m_timer->setInterval(1000);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

/*
 * The method that is exposed to the world
 */
void ProcessWrapper::executeCommand(QString command)
{
  m_process->start(command);
  m_processPid = m_process->pid();
  emit startedTerminal();
  m_timer->start();
}

/*
 * Whenever the internal timer ticks, let's check if our process has finished
 */
void ProcessWrapper::onTimer()
{
  QProcess proc;
  QString cmd = QString("ps -p %1 %2").arg(m_processPid).arg(m_processPid + 1);
  proc.start(cmd);
  proc.waitForFinished(-1);

  //If any of the processes have finished...
  QString out = proc.readAll();
  QStringList sl = out.split("\n", QString::SkipEmptyParts);
  if (sl.count() != 2)
  {
    emit finishedTerminal(0, QProcess::NormalExit);
    m_timer->stop();
  }
}
