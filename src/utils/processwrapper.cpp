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
#include "../strconstants.h"
#include <iostream>

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

  m_timerSingleShot = new QTimer(parent);
  m_timerSingleShot->setSingleShot(true);
  m_timer = new QTimer(parent);
  m_timer->setInterval(1000);

  connect(m_timerSingleShot, SIGNAL(timeout()), this, SLOT(onSingleShot()));
  connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
  connect(m_process, SIGNAL(started()), SLOT(onProcessStarted()));
}

/*
 * The method that is exposed to the world
 */
void ProcessWrapper::executeCommand(QString command)
{
  m_process->start(command);
}

/*
 * Only when m_process has started...
 */
void ProcessWrapper::onProcessStarted()
{  
  m_pidTerminal = m_process->pid();
  //std::cout << "First PID: " << m_pidTerminal << std::endl;
  m_timerSingleShot->start(2000);
  emit startedTerminal();
}

/*
 * We need this to search for the SH process pid (which spaws yaourt)
 */
void ProcessWrapper::onSingleShot()
{
  QProcess proc;
  QProcess pAux;
  QString saux;

  proc.start("ps -o pid -C sh");
  proc.waitForFinished(-1);
  QString out = proc.readAll();
  proc.close();

  QStringList list = out.split("\n", QString::SkipEmptyParts);

  if (list.count() == 1)
  {
    proc.start("ps -o pid -C bash");
    proc.waitForFinished(-1);
    out = proc.readAll();
    proc.close();

    list = out.split("\n", QString::SkipEmptyParts);
  }

  QStringList slist;

  for (int c=1; c<list.count(); c++)
  {
    int candidatePid = list.at(c).trimmed().toInt();

    if (candidatePid < m_pidTerminal) continue;

    QString cmd = QString("ps --ppid %1").arg(candidatePid);
    proc.start(cmd);
    proc.waitForFinished(-1);
    QString out = proc.readAll();

    if (out.contains(StrConstants::getForeignRepositoryToolName(), Qt::CaseInsensitive))
    {
      pAux.start("ps -o pid -C " + StrConstants::getForeignRepositoryToolName());
      pAux.waitForFinished(-1);
      saux = pAux.readAll();
      slist = saux.split("\n", QString::SkipEmptyParts);

      for (int d=1; d<slist.count(); d++)
      {
        int candidatePid2 = slist.at(d).trimmed().toInt();

        if (candidatePid < candidatePid2)
        {
          m_pidSH = candidatePid;
          m_pidYaourt = candidatePid2;
          m_timer->start();

          return;
        }
      }
    }
  }

  emit finishedTerminal(0, QProcess::NormalExit);
}

/*
 * Whenever the internal timer ticks, let's check if our process has finished
 */
void ProcessWrapper::onTimer()
{
  QProcess proc;
  QString cmd = QString("ps -p %1 %2").arg(m_pidSH).arg(m_pidYaourt);

  //std::cout << "PIDS: " << cmd.toAscii().data() << "\n" << std::endl;

  proc.start(cmd);
  proc.waitForFinished(-1);

  //If any of the processes have finished...
  QString out = proc.readAll();

  //std::cout << "Output: " << out.toAscii().data() << "\n" << std::endl;

  if (!out.contains(".qt_temp_", Qt::CaseInsensitive))
  {
    emit finishedTerminal(0, QProcess::NormalExit);
    m_timer->stop();
  }
}
