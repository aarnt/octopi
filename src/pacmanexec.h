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

#include "constants.h"
#include "unixcommand.h"

#include <QObject>

class QSharedMemory;

class PacmanExec : public QObject
{
  Q_OBJECT

private:
  bool m_iLoveCandy;
  bool m_debugMode;
  //This variable holds total number of packages to be downloaded/removed
  int m_numberOfPackages;
  //This variable counts the package number being downloaded/removed
  int m_packageCounter;
  //This variable counts the number of "error: failed retrieving file" strings
  int m_errorRetrievingFileCounter;
  //This flag holds TRUE if the parser is parsing a valid package removal operation
  bool m_parsingAPackageChange;

  UnixCommand *m_unixCommand;
  CommandExecuting m_commandExecuting;
  QStringList m_lastCommandList; //run in terminal commands
  QStringList m_textPrinted;
  QStringList m_listOfOutdatedPackages;
  QStringList m_listOfDotPacnewFiles; //contains the list of "blahblah installed as blahblah.pacnew" occurencies (if any)

  bool m_processWasCanceled;
  bool m_retrievingPackages;

  QSharedMemory *m_sharedMemory;

  bool searchForKeyVerbs(QString output);
  bool splitOutputStrings(QString output);
  void parsePacmanProcessOutput(const QString &output);
  bool criticalPhaseInTransaction(const QString &str);
  void prepareTextToPrint(QString str, TreatString ts = ectn_TREAT_STRING, TreatURLLinks tl = ectn_TREAT_URL_LINK);
  void parseCheckUpdates(QString &output);

private slots:
  //UnixCommand slots:
  void onStarted();
  void onReadOutput();
  void onReadOutputError();
  void onFinished(int exitCode, QProcess::ExitStatus);

public:
  explicit PacmanExec(QObject *parent = nullptr);
  virtual ~PacmanExec();

  void setSharedMemory(QSharedMemory *sm) { m_sharedMemory=sm; };

  void setDebugMode(bool value);
  void runLatestCommandInTerminal();
  void runLatestCommandWithOctopiHelper();

  static bool isDatabaseLocked();
  int cancelProcess();
  void doCheckUpdates();

  QStringList getOutdatedPackages();
  QStringList getDotPacnewFileList();

  //MIRROR-CHECK
  void doMirrorCheck();

  //PACMAN
  void doInstall(const QString &listOfPackages, const QString &listOfPackageDeps);
  void doInstallInTerminal(const QString &listOfPackages, const QString &listOfPackageDeps);

  void doInstallLocal(const QString &listOfPackages);
  void doInstallLocalInTerminal(const QString &listOfPackages);

  void doRemove(const QString &listOfPackages);
  void doRemoveInTerminal(const QString &listOfPackages);

  void doRemoveAndInstall(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall, const QString &listOfPackagesToInstallDeps);
  void doRemoveAndInstallInTerminal(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall, const QString &listOfPackagesToInstallDeps);

  void doSystemUpgrade();
  void doSystemUpgradeInTerminal(CommandExecuting additionalCommand = ectn_NONE);

  void doChangeInstallReason(const QHash<QString, QString> &listOfPackages);

  void doRefreshPacmanKeys();

  //AUR  
  void doAURUpgrade(const QString &listOfPackages);
  void doAURInstall(const QString &listOfPackages);
  void doInstallYayUsingTempYay();
  void doAURRemove(const QString &listOfPackages);
  void doAURRemoveAndInstallInTerminal(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall);

signals:
  void percentage(int);
  void started();
  void readOutput();
  void readOutputError();
  void finished(int exitCode, QProcess::ExitStatus);
  void textToPrintExt(QString m_textToPrint);
  void canStopTransaction(bool yesNo);
  void commandToExecInQTermWidget(QString);
};

#endif // PACMANEXEC_H
