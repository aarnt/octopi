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

#ifndef PACMANEXEC_H
#define PACMANEXEC_H

#include <QObject>
#include "constants.h"
#include "unixcommand.h"

class PacmanExec : public QObject
{
  Q_OBJECT

private:
  bool m_iLoveCandy;
  bool m_debugMode;
  UnixCommand *m_unixCommand;
  CommandExecuting m_commandExecuting;
  QStringList m_lastCommandList; //run in terminal commands
  QStringList m_textPrinted;

  bool searchForKeyVerbs(QString output);
  bool splitOutputStrings(QString output);
  void parsePacmanProcessOutput(QString output);
  void prepareTextToPrint(QString str, TreatString ts = ectn_TREAT_STRING, TreatURLLinks tl = ectn_TREAT_URL_LINK);

private slots:
  //UnixCommand slots:
  void onStarted();
  void onReadOutput();
  void onReadOutputError();
  void onFinished(int exitCode, QProcess::ExitStatus);

public:
  explicit PacmanExec(QObject *parent = 0);
  virtual ~PacmanExec();

  void setDebugMode(bool value);
  void runLastestCommandInTerminal();
  void removeTemporaryFile();

  static bool isDatabaseLocked();
  static void removeDatabaseLock();

  //MIRROR-CHECK
  void doMirrorCheck();

  //PACMAN
  void doInstall(const QString &listOfPackages);
  void doInstallInTerminal(const QString &listOfPackages);

  void doInstallLocal(const QString &listOfPackages);
  void doInstallLocalInTerminal(const QString &listOfPackages);

  void doRemove(const QString &listOfPackages);
  void doRemoveInTerminal(const QString &listOfPackages);

  void doRemoveAndInstall(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall);
  void doRemoveAndInstallInTerminal(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall);

  void doSystemUpgrade();
  void doSystemUpgradeInTerminal(CommandExecuting additionalCommand = ectn_NONE);

  void doSyncDatabase();

  //AUR
  void doAURUpgrade(const QString &listOfPackages);
  void doAURInstall(const QString &listOfPackages);
  void doAURRemove(const QString &listOfPackages);

signals:
  void percentage(int);
  void started();
  void readOutput();
  void readOutputError();
  void finished(int exitCode, QProcess::ExitStatus);
  void textToPrintExt(QString m_textToPrint);

};

#endif // PACMANEXEC_H
