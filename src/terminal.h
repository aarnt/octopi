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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <QString>
#include <QObject>
#include <QProcess>

class QSharedMemory;

class Terminal : public QObject
{
  Q_OBJECT

public:
  Terminal(QObject *parent);
  virtual ~Terminal();

  //Executes the given command using sudo
  void runCommandInTerminalWithSudo(const QString& command);
  void runCommandInTerminalWithSudo(const QStringList& commandList);

  //Executes the given commandList creating a ROOT temp transaction file and calling "octopi-helper -ts"
  void runOctopiHelperInTerminalWithSharedMem(const QStringList& commandList, QSharedMemory *sharedMem);
  void runCommandInTerminalAsNormalUser(const QStringList& commandList);

  QString getSudoProgram();

signals:
  void started();
  void finished(int, QProcess::ExitStatus);
  void commandToExecInQTermWidget(QString);
};

#endif // TERMINAL_H
