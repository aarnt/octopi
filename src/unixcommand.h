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

#ifndef UNIXCOMMAND_H
#define UNIXCOMMAND_H

#include <QObject>
#include <QProcess>
#include <QTime>
#include <unistd.h>

#include "package.h"
#include "utils.h"

//Forward class declarations.
class QString;
class QStringList;
class Terminal;

class UnixCommand : public QObject{
  Q_OBJECT

private:
  QString m_readAllStandardOutput;
  QString m_readAllStandardError;
  QString m_errorString;
  Terminal *m_terminal;
  QProcess *m_process;
  static QFile *m_temporaryFile;

public:
  UnixCommand(QObject *parent);

  inline QProcess * getProcess(){ return m_process; }

  //Returns true if ILoveCandy is enabled in "/etc/pacman.conf"
  static bool isILoveCandyEnabled();

  //Returns "fieldName" strings from "/etc/pacman.conf"
  static QStringList getFieldFromPacmanConf(const QString &fieldName);
  //Returns the list of ignored packages in "/etc/pacman.conf"
  static QStringList getIgnorePkgsFromPacmanConf();

  //Returns the Linux Distro where Octopi is running on
  static LinuxDistro getLinuxDistro();

  //Returns the PrettyName info from /etc/os-release
  static QString getLinuxDistroPrettyName();

  static QString getPacmanVersion();

  static QString getShell();

  //Delegations from Package class (due to QProcess use)
  static QString runCommand(const QString& commandToRun);
  static QString runCurlCommand(const QString& commandToRun);
  static QString discoverBinaryPath(const QString&);

  static bool cleanPacmanCache();

  static QByteArray performQuery(const QStringList args);
  static QByteArray performQuery(const QString &args);
  static QByteArray performAURCommand(const QString &args);

  static bool hasPackage(const QString &pkgName);

  static QByteArray getAURUrl(const QString &pkgName);
  static QByteArray getAURPackageList(const QString &searchString);
  static QByteArray getUnrequiredPackageList();
  static QByteArray getOutdatedPackageList();
  static QByteArray getOutdatedAURPackageList();
  static QByteArray getForeignPackageList();
  static QByteArray getPackageList(const QString &pkgName = "");

  static QByteArray getKCPPackageInformation(const QString &pkgName);
  static QByteArray getPackageInformation(const QString &pkgName, bool foreignPackage);
  static QByteArray getAURPackageVersionInformation();
  static QByteArray getPackageContentsUsingPacman(const QString &pkgName);
  static bool isPkgfileInstalled();
  static QByteArray getPackageContentsUsingPkgfile(const QString &pkgName);

  static QString getPackageByFilePath(const QString &filePath);
  static QStringList getFilePathSuggestions(const QString &file);

  static QByteArray getPackageGroups();
  static QByteArray getPackagesFromGroup(const QString &groupName);
  static QByteArray getTargetUpgradeList(const QString &pkgName = "");
  static QByteArray getTargetRemovalList(const QString &pkgName, const QString &removeCommand);

  static QString getSystemArchitecture();
  static bool hasInternetConnection();
  static bool doInternetPingTest();
  static bool isTextFile( const QString& fileName ); //fileName is Path + Name

  static bool hasTheExecutable( const QString& exeName );
  static bool isAppRunning(const QString &appName, bool justOneInstance = false);

  static bool isRootRunning(){
    int uid = geteuid();
    return (uid == 0); //Returns TRUE if root is running Octopi
  }

  static QFile* getTemporaryFile(){
    QTime time = QTime::currentTime();
    qsrand(time.minute() + time.second() + time.msec());

    m_temporaryFile = new QFile(ctn_TEMP_ACTIONS_FILE + QString::number(qrand()));
    m_temporaryFile->open(QIODevice::ReadWrite|QIODevice::Text);
    m_temporaryFile->setPermissions(QFile::Permissions(QFile::ExeOwner|QFile::ReadOwner));

    return m_temporaryFile;
  }

  static void removeTemporaryFile(){
    if (m_temporaryFile != 0){
      m_temporaryFile->close();
      m_temporaryFile->remove();
      delete m_temporaryFile;
      m_temporaryFile = 0;
    }
  }

  static bool isPackageInstalled(const QString& pkgName);

  static void removeTemporaryFiles();

  void openRootTerminal();
  void runCommandInTerminal(const QStringList& commandList);
  void runCommandInTerminalAsNormalUser(const QStringList& commandList);

  static void execCommandAsNormalUser(const QString &pCommand);
  static void execCommand(const QString &pCommand);

  static QByteArray getCommandOutput(const QString &pCommand);

  void executeCommand(const QString &pCommand, Language lang=ectn_LANG_ENGLISH);
  void executeCommandAsNormalUser(const QString &pCommand);

  QString readAllStandardOutput();
  QString readAllStandardError();
  QString errorString();

public slots:
  void processReadyReadStandardOutput();
  void processReadyReadStandardError();

signals:
  void started();
  void readyReadStandardOutput();
  void finished ( int, QProcess::ExitStatus );
  void readyReadStandardError();

  //ProcessWrapper signals
  void startedTerminal();
  void finishedTerminal(int, QProcess::ExitStatus);
};

#endif // UNIXCOMMAND_H
