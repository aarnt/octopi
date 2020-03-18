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

#include "unixcommand.h"
#include "strconstants.h"
#include "wmhelper.h"
#include "terminal.h"
#include <iostream>

#include <QProcess>
#include <QRegularExpression>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QTextStream>
#include <QtNetwork>
#include <QFileInfo>

/*
 * Collection of methods to execute many Unix commands
 */

QFile *UnixCommand::m_temporaryFile = nullptr;

/*
 * UnixCommand's constructor: the relevant environment english setting and the connectors
 */
UnixCommand::UnixCommand(QObject *parent): QObject()
{
  m_process = new QProcess(parent);
  m_terminal = new Terminal(parent);

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  m_process->setProcessEnvironment(env);

  QObject::connect(m_process, SIGNAL( started() ), this,
                   SIGNAL( started() ));
  QObject::connect(this, SIGNAL( started() ), this,
                   SLOT( processReadyReadStandardOutput() ));

  QObject::connect(m_process, SIGNAL( readyReadStandardOutput() ), this,
                   SIGNAL( readyReadStandardOutput() ));
  QObject::connect(this, SIGNAL( readyReadStandardOutput() ), this,
                   SLOT( processReadyReadStandardOutput() ));

  QObject::connect(m_process, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SIGNAL( finished ( int, QProcess::ExitStatus )) );
  QObject::connect(this, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SLOT( processReadyReadStandardOutput() ));

  QObject::connect(m_process, SIGNAL( readyReadStandardError() ), this,
                   SIGNAL( readyReadStandardError() ));
  QObject::connect(this, SIGNAL( readyReadStandardError() ), this,
                   SLOT( processReadyReadStandardError() ));

  //Terminal signals
  QObject::connect(m_terminal, SIGNAL( started()), this,
                   SIGNAL( started()));
  QObject::connect(m_terminal, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SIGNAL( finished ( int, QProcess::ExitStatus )) );

  QObject::connect(m_terminal, SIGNAL(commandToExecInQTermWidget(QString)), this,
                   SIGNAL(commandToExecInQTermWidget(QString)));
}

/*
 * Executes given command and returns the StandardError Output.
 */
QString UnixCommand::runCommand(const QString& commandToRun)
{
  QProcess proc;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove(QStringLiteral("LANG"));
  env.remove(QStringLiteral("LC_MESSAGES"));
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  proc.setProcessEnvironment(env);
  proc.start(commandToRun);
  proc.waitForStarted();
  proc.waitForFinished(-1);

  QString res = proc.readAllStandardError();
  proc.close();

  return res;
}

/*
 * Executes the CURL command and returns the StandardError Output, if result code <> 0.
 */
QString UnixCommand::runCurlCommand(const QString& commandToRun){
  QProcess proc;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  proc.setProcessEnvironment(env);
  proc.start(commandToRun);
  proc.waitForStarted();
  proc.waitForFinished(-1);

  QString res(QLatin1String(""));

  if (proc.exitCode() != 0)
  {
    res = proc.readAllStandardError();
  }

  proc.close();
  return res;
}

/*
 * Returns the path of given executable
 */
QString UnixCommand::discoverBinaryPath(const QString& binary){
  QProcess proc;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  proc.setProcessEnvironment(env);

  proc.start("/bin/sh -c \"which " + binary + "\"");
  proc.waitForFinished();
  QString res = proc.readAllStandardOutput();

  proc.close();
  res = res.remove('\n');

  //If it still didn't find it, try "/sbin" dir...
  if (res.isEmpty()){
    QFile fbin("/sbin/" + binary);
    if (fbin.exists()){
      res = "/sbin/" + binary;
    }
  }

  return res;
}

/*
 * Cleans Pacman's package cache.
 * Returns true if finished OK
 */
bool UnixCommand::cleanPacmanCache()
{
  QProcess pacman;
  QString commandStr = QStringLiteral("\"yes | pacman -Scc\"");
  QString command = WMHelper::getSUCommand() + " " + commandStr;
  pacman.start(command);
  pacman.waitForFinished();

  return (pacman.exitCode() == 0);
}

/*
 * Performs a pacman query
 */
QByteArray UnixCommand::performQuery(const QStringList &args)
{
  QByteArray result("");
  QProcess pacman;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
  pacman.setProcessEnvironment(env);

  pacman.start(QStringLiteral("pacman"), args);
  pacman.waitForFinished();
  result = pacman.readAllStandardOutput();
  pacman.close();

  return result;
}

/*
 * Performs a pacman query
 * Overloaded with QString parameter
 */
QByteArray UnixCommand::performQuery(const QString &args)
{
  QByteArray result("");
  QProcess pacman;

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
  pacman.setProcessEnvironment(env);

  pacman.start("pacman " + args);
  pacman.waitForFinished();
  result = pacman.readAllStandardOutput();
  pacman.close();
  return result;
}

/*
 * Performs a yourt command
 */
QByteArray UnixCommand::performAURCommand(const QString &args)
{
  QByteArray result("");
  QProcess aur;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  aur.setProcessEnvironment(env);

  aur.start(Package::getForeignRepositoryToolNameParam() + " " + args);
  aur.waitForFinished(-1);
  result = aur.readAllStandardOutput();

  aur.close();
  return result;
}

/*
 * Searches for a given package in the official repos
 */
bool UnixCommand::hasPackage(const QString &pkgName)
{
  QString res = performQuery("-Ss " + pkgName);
  return !res.isEmpty();
}

/*
 * Retrieves URL information for remote AUR package
 */
QByteArray UnixCommand::getAURUrl(const QString &pkgName)
{
  QByteArray result("");
  QProcess aur;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));

  aur.setProcessEnvironment(env);

  if (Package::getForeignRepositoryToolName() == ctn_CHASER_TOOL)
    aur.start(Package::getForeignRepositoryToolNameParam() + " info " + pkgName);
  else
    aur.start(Package::getForeignRepositoryToolNameParam() + " -Sia " + pkgName);

  aur.waitForFinished(-1);

  return (aur.readAll());
}

/*
 * Returns a string containing all AUR packages given a searchString parameter
 */
QByteArray UnixCommand::getAURPackageList(const QString &searchString)
{
  QByteArray result("");
  QProcess aur;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  env.insert(QStringLiteral("LANGUAGE"), QStringLiteral("C"));

  aur.setProcessEnvironment(env);

  if (UnixCommand::getLinuxDistro() == ectn_KAOS)
    aur.start(Package::getForeignRepositoryToolNameParam() + " -l ");
  else
  {
    if (Package::getForeignRepositoryToolName() == ctn_YAOURT_TOOL)
      aur.start(Package::getForeignRepositoryToolNameParam() + " --nocolor -Ss " + searchString);
    else if (Package::getForeignRepositoryToolName() == ctn_TRIZEN_TOOL)
        aur.start(Package::getForeignRepositoryToolNameParam() + " --nocolors -Ssa " + searchString);
    else if (Package::getForeignRepositoryToolName() == ctn_PIKAUR_TOOL || Package::getForeignRepositoryToolName() == ctn_YAY_TOOL)
        aur.start(Package::getForeignRepositoryToolNameParam() + " --color=never -Ss --aur " + searchString);
    else if (Package::getForeignRepositoryToolName() == ctn_CHASER_TOOL)
      aur.start(Package::getForeignRepositoryToolNameParam() + " search " + searchString);
    else
      aur.start(Package::getForeignRepositoryToolNameParam() + " -Ss " + searchString);
  }

  aur.waitForFinished(-1);
  result = aur.readAll();

  if (UnixCommand::getLinuxDistro() == ectn_KAOS)
  {
    QString res = result;
    res.remove("\033");
    res.remove(QStringLiteral("[1m"));
    res.remove(QStringLiteral("[m"));
    res.remove(QStringLiteral("[1;32m"));
    res.remove(QStringLiteral("[1;34m"));
    res.remove(QStringLiteral("[1;36m"));

    return res.toLatin1();
  }
  else if (UnixCommand::getLinuxDistro() == ectn_CHAKRA)
  {
    QString res = result;
    res.remove("\033");
    res.remove(QStringLiteral("[0m"));
    res.remove(QStringLiteral("[1m"));
    res.remove(QStringLiteral("[m"));
    res.remove(QStringLiteral("[32m"));
    res.remove(QStringLiteral("[35m"));

    return res.toLatin1();
  }

  return result;
}

/*
 * Returns the SHELL environment variable, if not set defaults to bash.
 */
QString UnixCommand::getShell()
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QString shell = env.value(QStringLiteral("SHELL"), QStringLiteral("/bin/bash"));

  QFileInfo fi(shell);

  if (fi.fileName() == QLatin1String("fish"))
    return QStringLiteral("bash");
  else
    return fi.fileName();
}

/*
 * Returns a string containing all packages no one depends on
 */
QByteArray UnixCommand::getUnrequiredPackageList()
{
  QByteArray result = performQuery(QStringList(QStringLiteral("-Qt")));
  return result;
}

/*
 * Returns a string containing all packages that are outdated since last DB sync
 */
QByteArray UnixCommand::getOutdatedPackageList()
{
  return performQuery(QStringList(QStringLiteral("-Qu")));
}

/*
 * Returns a string containing all AUR outdated packages
 */
QByteArray UnixCommand::getOutdatedAURPackageList()
{
  QByteArray result;

  if (Package::getForeignRepositoryToolName() == ctn_KCP_TOOL)
  {
    result = performAURCommand(QStringLiteral("-lO"));
  }
  else if (Package::getForeignRepositoryToolName() != ctn_KCP_TOOL)
  {
    result = performAURCommand(QStringLiteral("-Qua"));
  }

  //return ":: aur  micro-git  v1.3.3.d6ccaf0-1  ->  v1.3.4\n";
  return result;
}

/*
 * Returns a string containing all packages that are not contained in any repository
 * (probably the ones installed by a tool such as yaourt)
 */
QByteArray UnixCommand::getForeignPackageList()
{
  QByteArray result = performQuery(QStringList(QStringLiteral("-Qem")));
  return result;
}

/*
 * Returns a string with the list of all packages available in all repositories
 * (installed + not installed)
 *
 * @param pkgName Used while the user is searching for the pkg that provides a certain file
 */
QByteArray UnixCommand::getPackageList(const QString &pkgName)
{
  QByteArray result;

  if (pkgName.isEmpty())
    result = performQuery(QStringList(QStringLiteral("-Ss")));
  else
  {
    QStringList sl;
    sl << QStringLiteral("-Ss");
    sl << pkgName;
    result = performQuery(sl);
  }

  return result;
}

/*
 * Given a KCP package name,
 * returns a string containing all of its information fields
 * (ex: name, description, version, dependsOn...)
 */
QByteArray UnixCommand::getKCPPackageInformation(const QString &pkgName)
{
  QString args = "--information " + pkgName;
  QByteArray result = performAURCommand(args);
  return result;
}

/*
 * Given a package name, returns its description using expac utility
 */
QByteArray UnixCommand::getExpacInfo(const QString &pkgName, const QString &info)
{
  /*QByteArray result("");
  QProcess grep;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  env.insert("LC_ALL", "C");
  grep.setProcessEnvironment(env);
  grep.start(getShell() + " -c \"pacman -Qi " + pkgName + " | grep --color=never -oP '(?<=Description     : ).*'\"");
  grep.waitForFinished();
  result = grep.readAllStandardOutput();
  grep.close();*/

  QByteArray result("");
  QProcess expac;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
  expac.setProcessEnvironment(env);
  QString expacStr = QStringLiteral("expac -s \"%%1\" %2");
  expac.start(expacStr.arg(info, pkgName));
  expac.waitForFinished();
  result = expac.readAllStandardOutput();
  expac.close();

  return result;
}

/*
 * Given a package name and if it is default to the official repositories,
 * returns a string containing all of its information fields
 * (ex: name, description, version, dependsOn...)
 */
QByteArray UnixCommand::getPackageInformation(const QString &pkgName, bool foreignPackage = false)
{
  QStringList args;

  if(foreignPackage)
    args << QStringLiteral("-Qi");
  else
    args << QStringLiteral("-Si");

  if (pkgName.isEmpty() == false) // enables get for all ("")
    args << pkgName;

  QByteArray result = performQuery(args);
  return result;
}

/*
 * Given a package name, returns a string containing all the files inside it
 */
QByteArray UnixCommand::getPackageContentsUsingPacman(const QString& pkgName)
{
  QStringList args;
  args << QStringLiteral("-Ql");
  args << pkgName;

  QByteArray res = performQuery(args);
  return res;
}

/*
 * Check if pkgfile is installed on the system
 */
bool UnixCommand::isPkgfileInstalled()
{
  QProcess pkgfile;

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  pkgfile.setProcessEnvironment(env);

  pkgfile.start(QStringLiteral("pkgfile -V"));
  pkgfile.waitForFinished();

  return pkgfile.exitStatus() == QProcess::NormalExit;
}

/*
 * Given a package name, which can be installed or uninstalled on system
 * returns a string containing all the files inside it, the file list is
 * obtained using pkgfile
 */
QByteArray UnixCommand::getPackageContentsUsingPkgfile(const QString &pkgName)
{
  QByteArray result("");
  QProcess pkgfile;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  pkgfile.setProcessEnvironment(env);

  pkgfile.start("pkgfile -l " + pkgName);
  pkgfile.waitForFinished();
  result = pkgfile.readAllStandardOutput();

  return result;
}

/*
 * Given a complete file path, returns the package that provides that file
 */
QString UnixCommand::getPackageByFilePath(const QString &filePath)
{
  QStringList sl;
  sl << QStringLiteral("-Qo");
  sl << filePath;

  QString out = performQuery(sl);
  QStringList s = out.split(QStringLiteral("\n"), QString::SkipEmptyParts);

  if (s.count() >= 1)
  {
    QStringList res = s.at(0).split(QStringLiteral(" "), QString::SkipEmptyParts);
    return res.at(res.count()-2);
  }
  else return QLatin1String("");
}

/*
 * Based on the given file, we use 'slocate' to suggest complete paths
 */
QStringList UnixCommand::getFilePathSuggestions(const QString &file)
{
  QProcess slocate;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  slocate.setProcessEnvironment(env);
  slocate.start("slocate -l 8 " + file);
  slocate.waitForFinished();

  QString ba = slocate.readAllStandardOutput();
  return ba.split(QStringLiteral("\n"), QString::SkipEmptyParts);
}

/*
 * Retrives the list of package groups
 */
QByteArray UnixCommand::getPackageGroups()
{
  QByteArray res = performQuery(QStringList(QStringLiteral("-Sg")));
  return res;
}

/*
 * Given a group name, returns a string containing all packages from it
 */
QByteArray UnixCommand::getPackagesFromGroup(const QString &groupName)
{
  QByteArray res =
      performQuery(QStringLiteral("--print-format \"%r %n\" -Spg " ) + groupName);

  return res;
}

/*
 * Retrieves the list of targets needed to update the entire system or a given package
 */
QByteArray UnixCommand::getTargetUpgradeList(const QString &pkgName)
{
  QString args;

  if(!pkgName.isEmpty())
  {
    args = "--print-format \"%n %v %s\" -Sp " + pkgName;
  }
  else
  {
    args = QStringLiteral("--print-format \"%n %v %s\" -Spu");
  }

  QByteArray res = performQuery(args);
  return res;
}

/*
 * Given a package name, retrieves the list of all targets needed for its removal
 */
QByteArray UnixCommand::getTargetRemovalList(const QString &pkgName, const QString &removeCommand)
{
  QString args;
  args = "-" + removeCommand + "p " + pkgName;

  QByteArray res = performQuery(args);
  return res;
}

/*
 * Retrieves the system arch
 */
QString UnixCommand::getSystemArchitecture()
{
  QStringList slParam;
  QProcess proc;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  proc.setProcessEnvironment(env);

  slParam << QStringLiteral("-m");
  proc.start(QStringLiteral("uname"), slParam);
  proc.waitForFinished();

  QString out = proc.readAllStandardOutput();
  proc.close();

  return out;
}

/*
 * Checks if we have internet access!
 */
bool UnixCommand::hasInternetConnection()
{
  QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
  bool result = false;

  for (int i = 0; i < ifaces.count(); i++){
    QNetworkInterface iface = ifaces.at(i);

    if ( iface.flags().testFlag(QNetworkInterface::IsUp)
         && !iface.flags().testFlag(QNetworkInterface::IsLoopBack) ){
      for (int j=0; j<iface.addressEntries().count(); j++){
        /*
         We have an interface that is up, and has an ip address
         therefore the link is present.

         We will only enable this check on first positive,
         all later results are incorrect
        */
        if (result == false)
          result = true;
      }
    }
  }

  //It seems to be alright, but let's make a ping to see the result
  if (result == true)
  {
    result = UnixCommand::doInternetPingTest();
  }

  return result;
}

/*
 * Pings google site, to make sure internet is OK
 */
bool UnixCommand::doInternetPingTest()
{
  QTcpSocket socket;
  QString hostname = QStringLiteral("www.google.com");

  socket.connectToHost(hostname, 80);
  if (socket.waitForConnected(5000))
    return true;
  else
  {
    hostname = QStringLiteral("www.baidu.com");
    socket.connectToHost(hostname, 80);
    if (socket.waitForConnected(5000))
      return true;
    else
      return false;
  }
}

/*
 * Checks if the given executable is available somewhere in the system
 */
bool UnixCommand::hasTheExecutable( const QString& exeName )
{
  QProcess proc;
  proc.setProcessChannelMode(QProcess::MergedChannels);
  QString sParam = "\"which " + exeName + "\"";
  proc.start("/bin/sh -c " + sParam);
  proc.waitForFinished();

  QString out = proc.readAllStandardOutput();
  proc.close();

  if (out.isEmpty() || out.count(QStringLiteral("which")) > 0) return false;
  else return true;
}

/*
 * Does some garbage collection, removing uneeded shared memory files
 */
void UnixCommand::removeSharedMemFiles()
{
  QDir tempDir(QDir::tempPath());
  QStringList nameFilters;
  nameFilters << QStringLiteral("qipc_sharedmemory_orgarntoctopi*")
              << QStringLiteral("qipc_systemsem_orgarntoctopi*");

  QFileInfoList list = tempDir.entryInfoList(nameFilters, QDir::Dirs | QDir::Files | QDir::System | QDir::Hidden);

  foreach(QFileInfo file, list){
    QFile fileAux(file.filePath());

    if (!file.isDir()){
      fileAux.remove();
    }
    else{
      QDir dir(file.filePath());
      QFileInfoList listd = dir.entryInfoList(QDir::Files | QDir::System);

      foreach(QFileInfo filed, listd){
        QFile fileAuxd(filed.filePath());
        fileAuxd.remove();
      }

      dir.rmdir(file.filePath());
    }
  }
}

/*
 * Does some garbage collection, removing uneeded files
 */
void UnixCommand::removeTemporaryFiles()
{
  QDir tempDir(QDir::tempPath());
  QStringList nameFilters;
  nameFilters << QStringLiteral("qtsingleapp-Octopi*") << QStringLiteral("qtsingleapp-CacheC*") << QStringLiteral("qtsingleapp-Reposi*")
              //<< "qipc_sharedmemory_orgarntoctopi*"
              //<< "qipc_systemsem_orgarntoctopi*"
              << QStringLiteral(".qt_temp_octopi*");
  QFileInfoList list = tempDir.entryInfoList(nameFilters, QDir::Dirs | QDir::Files | QDir::System | QDir::Hidden);

  foreach(QFileInfo file, list){
    QFile fileAux(file.filePath());

    if (!file.isDir()){
      fileAux.remove();
    }
    else{
      QDir dir(file.filePath());
      QFileInfoList listd = dir.entryInfoList(QDir::Files | QDir::System);

      foreach(QFileInfo filed, listd){
        QFile fileAuxd(filed.filePath());
        fileAuxd.remove();
      }

      dir.rmdir(file.filePath());
    }
  }
}

/*
 * Runs a command AS NORMAL USER externaly with QProcess!
 */
void UnixCommand::execCommandAsNormalUser(const QString &pCommand)
{
  QProcess::startDetached(pCommand);
}

/*
 * Execs a command as normal user and returns its output
 */
QByteArray UnixCommand::execCommandAsNormalUserExt(const QString &pCommand)
{
  QProcess p;
  QByteArray res;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  p.setProcessEnvironment(env);

  p.start(pCommand);
  p.waitForFinished(-1);
  res = p.readAllStandardOutput();
  p.close();
  return res;
}

/*
 * Runs a command with a QProcess blocking object!
 */
void UnixCommand::execCommand(const QString &pCommand)
{
  QProcess p;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  p.setProcessEnvironment(env);

  p.start(WMHelper::getSUCommand() + getShell() + " -c \"" + pCommand + "\"");
  p.waitForFinished(-1);
  p.close();
}

/*
 * Runs a command with a QProcess blocking object and returns its output!
 */
QByteArray UnixCommand::getCommandOutput(const QString &pCommand)
{
  QProcess p;
  /*QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  p.setProcessEnvironment(env);*/

  p.start(pCommand);
  p.waitForFinished(-1);
  return p.readAllStandardOutput();
}

/*
 * Runs a command with a QProcess blocking object and returns its output!
 */
QByteArray UnixCommand::getCommandOutput(const QString &pCommand, const QString &fileName)
{
  QProcess p;

  p.setStandardInputFile(fileName);
  p.start(pCommand);
  p.waitForFinished(-1);
  return p.readAllStandardOutput();
}

/*
 * Returns a list of the available AUR tools installed in the system
 */
QStringList UnixCommand::getAvailableAURTools()
{
  QStringList aurTools;

  if ((UnixCommand::getLinuxDistro() != ectn_KAOS) &&
    (UnixCommand::getLinuxDistro() != ectn_CHAKRA &&
     UnixCommand::getLinuxDistro() != ectn_PARABOLA))
  {
    aurTools << ctn_NO_AUR_TOOL;
    if (UnixCommand::hasTheExecutable(ctn_PACAUR_TOOL))
      aurTools << ctn_PACAUR_TOOL;
    if (UnixCommand::hasTheExecutable(ctn_YAOURT_TOOL))
      aurTools << ctn_YAOURT_TOOL;
    if (UnixCommand::hasTheExecutable(ctn_TRIZEN_TOOL))
      aurTools << ctn_TRIZEN_TOOL;
    if (UnixCommand::hasTheExecutable(ctn_PIKAUR_TOOL))
      aurTools << ctn_PIKAUR_TOOL;
    if (UnixCommand::hasTheExecutable(ctn_YAY_TOOL))
      aurTools << ctn_YAY_TOOL;
  }

  return aurTools;
}

/*
 * Given a filename, checks if it is a text file
 */
bool UnixCommand::isTextFile(QString fileName)
{
  init:
  QProcess *p = new QProcess();
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  p->setProcessEnvironment(env);

  QStringList s(fileName);
  p->start( QStringLiteral("file"), s );
  p->waitForFinished();

  QString output = p->readAllStandardOutput();
  p->close();
  delete p;

  //If it's a symbolic link, let's discover what is the real target
  if (output.contains(QLatin1String(": symbolic link to")))
  {
    QFileInfo fi(fileName);
    fileName = fi.symLinkTarget();
    goto init;
  }
  else
  {
    int from = output.indexOf(QLatin1String(":"), 0)+1;

    return (((output.indexOf( QLatin1String("ASCII"), from ) != -1) ||
            (output.indexOf( QLatin1String("text"), from ) != -1) ||
            (output.indexOf( QLatin1String("empty"), from ) != -1)) &&
            (output.indexOf( QLatin1String("executable"), from) == -1));
  }
}

/*
 * Executes given commandList as root inside a terminal, so the user can interact
 */
void UnixCommand::runCommandInTerminalWithSudo(const QString& command){
  m_terminal->runCommandInTerminalWithSudo(command);
}

/*
 * Executes given commandList as root inside a terminal using "octopi-helper -t" (passing cmds thru memory)
 */
void UnixCommand::runOctopiHelperInTerminalWithSharedMem(const QStringList &commandList, QSharedMemory *sharedMem)
{
  //Checks if octopi-helper is running. If so, we exit!
  if (isOctopiHelperRunning()) return;

  m_terminal->runOctopiHelperInTerminalWithSharedMem(commandList, sharedMem);
}

/*
 * Executes given commandList inside a terminal, as the current user!
 */
void UnixCommand::runCommandInTerminalAsNormalUser(const QStringList &commandList)
{
  m_terminal->runCommandInTerminalAsNormalUser(commandList);
}

/*
 * Executes the given command using QProcess async technology
 */
void UnixCommand::executeCommand(const QString &pCommand)
{
  QString command;

  //COLUMNS variable code!
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove(QStringLiteral("LANG"));
  env.remove(QStringLiteral("LC_MESSAGES"));
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  env.remove(QStringLiteral("COLUMNS"));
  env.insert(QStringLiteral("COLUMNS"), QStringLiteral("132"));
  m_process->setProcessEnvironment(env);

  QString suCommand = WMHelper::getSUCommand();
  command = suCommand + getShell() + " -c " + "\"" + pCommand + "\"";
  m_process->start(command);
}

/*
 * Executes the given command using QProcess async technology with ROOT credentials
 */
void UnixCommand::executeCommandWithSharedMemHelper(const QString &pCommand, QSharedMemory *sharedMem)
{
  //Checks if octopi-helper is running. If so, we exit!
  if (isOctopiHelperRunning()) return;

  QString command;

  //COLUMNS variable code!
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove(QStringLiteral("LANG"));
  env.remove(QStringLiteral("LC_MESSAGES"));
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  env.remove(QStringLiteral("COLUMNS"));
  env.insert(QStringLiteral("COLUMNS"), QStringLiteral("132"));
  m_process->setProcessEnvironment(env);

  QString suCommand = WMHelper::getSUCommand();

  if (suCommand == WMHelper::getOctopiSudoCommand())
  {
    command = buildOctopiHelperCommandWithSharedMem(pCommand, sharedMem);
  }
  else //We are not using "octopi-sudo" nor "lxqt-sudo" utility...*/
  {
    command = suCommand + "\"" + pCommand + "\"";
  }

  m_process->start(command);
}

/*
 * Executes the given command using QProcess async technology with ROOT credentials
 */
/*void UnixCommand::executeCommand(const QString &pCommand, Language lang)
{
  QString command;

  if (lang == ectn_LANG_ENGLISH)
  {
    //COLUMNS variable code!
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("LANG");
    env.remove("LC_MESSAGES");
    env.insert("LANG", "C");
    env.insert("LC_MESSAGES", "C");
    env.remove("COLUMNS");
    env.insert("COLUMNS", "132");
    m_process->setProcessEnvironment(env);
  }
  else if (lang == ectn_LANG_USER_DEFINED)
  {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("LANG");
    env.remove("LC_MESSAGES");
    env.insert("LANG", QLocale::system().name() + ".UTF-8");
    env.insert("LC_MESSAGES", QLocale::system().name() + ".UTF-8");
    env.remove("COLUMNS");
    env.insert("COLUMNS", "132");
    m_process->setProcessEnvironment(env);
  }

  QString suCommand = WMHelper::getSUCommand();

  if (suCommand == WMHelper::getOctopiSudoCommand())
  {
    command = buildOctopiHelperCommand(pCommand);
  }
  //We are not using "octopi-sudo" nor "lxqt-sudo" utility...
  //else
  //{
  //  command = suCommand + "\"" + pCommand + "\"";
  //}

  m_process->start(command);
}*/

/*
 * Executes the given command using QProcess async technology as a normal user
 */
void UnixCommand::executeCommandAsNormalUser(const QString &pCommand)
{
  m_process->start(pCommand);
}

/*
 * Puts all Standard output of the member process into a member string
 */
void UnixCommand::processReadyReadStandardOutput()
{
  if (m_process->isOpen())
    m_readAllStandardOutput = m_process->readAllStandardOutput();
}

/*
 * Puts all StandardError output of the member process into a member string
 */
void UnixCommand::processReadyReadStandardError()
{
  if (m_process->isOpen())
  {
    m_readAllStandardError = m_process->readAllStandardError();
    m_errorString = m_process->errorString();
  }
}

/*
 * Retrieves Standard output of member process
 */
QString UnixCommand::readAllStandardOutput()
{
  return m_readAllStandardOutput;
}

/*
 * Retrieves StandardError output of member process
 */
QString UnixCommand::readAllStandardError()
{
  return m_readAllStandardError;
}

/*
 * Retrieves ErrorString of member process
 */
QString UnixCommand::errorString()
{
  return m_errorString;
}

QString UnixCommand::buildOctopiHelperCommandWithSharedMem(const QString &pCommand, QSharedMemory *sharedMem)
{
  QString octopiHelperCommandParameter;
  QString suCommand = WMHelper::getSUCommand();
  octopiHelperCommandParameter = QStringLiteral(" -ts");
  suCommand += ctn_OCTOPI_HELPER_PATH + octopiHelperCommandParameter;

  QStringList commandList;
  QString commands;
  QByteArray sharedData;

  //If this is a multiple command string, let's break it
  if (pCommand.contains(QLatin1String(";")))
  {
    commandList = pCommand.split(QStringLiteral(";"), QString::SkipEmptyParts);
    foreach(QString line, commandList)
    {
      commands += line.trimmed() + "\n";
    }
  }
  else //We have just one command here
  {
    commandList << pCommand.trimmed();
    commands += commandList.first() + "\n";
  }

  sharedData=commands.toLatin1();

  /*if (sharedMem != nullptr)
  {
    if (sharedMem->isAttached())
      sharedMem->detach();
    delete sharedMem;
    sharedMem=nullptr;
  }*/

  //removeSharedMemFiles();
  //sharedMem=new QSharedMemory("org.arnt.octopi", this);
  sharedMem->create(sharedData.size());
  sharedMem->lock();
  memcpy(sharedMem->data(), sharedData.data(), sharedData.size());
  sharedMem->unlock();

  return suCommand;
}

/*
 * Constructs the octopi-sudo related command to execute given pCommand using "octopi-helper" utility
 */
/*QString UnixCommand::buildOctopiHelperCommand(const QString &pCommand)
{
  QString octopiHelperCommandParameter;
  QStringList commandList;
  QString suCommand = WMHelper::getSUCommand();
  QFile *ftemp = UnixCommand::generateTemporaryFile();
  QTextStream out(ftemp);

  //If this is a multiple command string, let's break it
  if (pCommand.contains(";"))
  {
    commandList = pCommand.split(";", QString::SkipEmptyParts);
    foreach(QString line, commandList)
    {
      out << line.trimmed() << "\n";
    }
  }
  else //We have just one command here
  {
    commandList << pCommand.trimmed();
    out << commandList.first();
  }

  out.flush();
  ftemp->close();

  octopiHelperCommandParameter = " -t";
  return suCommand + ctn_OCTOPI_HELPER + octopiHelperCommandParameter;
}*/

/*
 * Cancels the running process. Returns octopi-sudo exit code.
 */
int UnixCommand::cancelProcess(QSharedMemory *sharedMem)
{
  QProcess pacman;
  QString suCommand = WMHelper::getSUCommand();
  QString pCommand = "killall pacman; rm " + ctn_PACMAN_DATABASE_LOCK_FILE;
  QString result;

  if (suCommand == WMHelper::getOctopiSudoCommand())
  {
    result = buildOctopiHelperCommandWithSharedMem(pCommand, sharedMem);
  }
  else {
    result = suCommand + "\"" + pCommand + "\"";
  }

  pacman.start(result);
  pacman.waitForFinished(-1);
  return pacman.exitCode();
}

/*
 * If justOneInstance = false (default), returns TRUE if one instance of the app is ALREADY running
 * Otherwise, it returns TRUE if the given app is running.
 */
bool UnixCommand::isAppRunning(const QString &appName, bool justOneInstance)
{
  QStringList slParam;
  QProcess proc;

  slParam << QStringLiteral("-C");
  slParam << appName;
  proc.start(QStringLiteral("ps"), slParam);
  proc.waitForFinished();
  QString out = proc.readAll();
  proc.close();

  if (justOneInstance)
  {
    if (out.count(appName)>0)
      return true;
    else
      return false;
  }
  else
  {
    if (out.count(appName)>1)
      return true;
    else
      return false;
  }
}

/*
 * Checks if Octopi/Octopi-notifier, cache-cleaner, etc is being executed
 */
bool UnixCommand::isOctoToolRunning(const QString &octoToolName)
{
  bool res=false;

  QProcess proc;
  proc.setProcessEnvironment(getProcessEnvironment());
  QString cmd = QStringLiteral("/usr/bin/ps -C %1 -o command");
  cmd = cmd.arg(octoToolName);
  proc.start(cmd);
  proc.waitForFinished();
  QString out = proc.readAll().trimmed();
  if (out.contains(QLatin1String("|"))) return false;
  out=out.remove(QStringLiteral("\n"));
  out=out.remove(QStringLiteral("COMMAND"));

  if (octoToolName==QLatin1String("octopi-cachecle"))
  {
    if (out == QLatin1String("/usr/bin/octopi-cachecleaner")) res=true;
  }
  else if ((out == "/usr/bin/" + octoToolName) || out.contains("/usr/bin/" + octoToolName + " ")) res=true;

  return res;
}

/*
 * Checks if current user is in "wheel" group
 */
bool UnixCommand::isUserInWheelGroup()
{
  bool result=false;
  QProcess groups;
  groups.start(QStringLiteral("groups"));
  groups.waitForFinished();
  if (groups.readAllStandardOutput().contains("wheel"))
    result=true;
  return result;
}

/*
 * Given a 'pkgName' package name, checks if that one is installed in the system
 */
bool UnixCommand::isPackageInstalled(const QString &pkgName)
{
  QProcess pacman;
  QString command = "pacman -Q " + pkgName;
  pacman.start(command);
  pacman.waitForFinished();
  return (pacman.exitCode() == 0);
}

/*
 * Searches "/etc/pacman.conf" to see if ILoveCandy is enabled
 */
bool UnixCommand::isILoveCandyEnabled()
{
  bool res=false;
  QFile file(QStringLiteral("/etc/pacman.conf"));

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QString contents = file.readAll();
  int end = contents.indexOf(QLatin1String("ILoveCandy"));
  if (end != -1)
  {
    //Does it contains a # before it???
    int start = end;
    do{
      start--;
    }while (contents.at(start) != '\n');

    QString str = contents.mid(start+1, (end-start-1)).trimmed();

    if (str.isEmpty()) res = true;
    else res = false;
  }

  file.close();

  return res;
}

/*
 * Returns the list of strings after "fieldName" in Pacman.conf;
 */
QStringList UnixCommand::getFieldFromPacmanConf(const QString &fieldName)
{
  QStringList result;
  QFile file(QStringLiteral("/etc/pacman.conf"));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return result;

  QString contents = file.readAll();
  int from = 0;
  const int ctn_FIELD_LENGTH = fieldName.length();

  do
  {
    int end = contents.indexOf(fieldName, from, Qt::CaseInsensitive);
    if (end != -1)
    {
      //Does it contains a # before it???
      int start = end;
      do{
        start--;
      }while (contents.at(start) != '\n');

      QString str = contents.mid(start+1, (end-start-1)).trimmed();

      if (str.isEmpty())
      {
        QString ignorePkg = contents.mid(end);
        int equal = ignorePkg.indexOf(QLatin1String("="));
        int newLine = ignorePkg.indexOf(QLatin1String("\n"));

        ignorePkg = ignorePkg.mid(equal+1, newLine-(equal+1)).trimmed();
        result += ignorePkg.split(QRegularExpression(QStringLiteral("\\s+")), QString::SkipEmptyParts);
        from = end + newLine;
      }
      else if (str != QLatin1String("#"))
        from += end + ctn_FIELD_LENGTH;
      else
        from += ctn_FIELD_LENGTH;
    }
    else break;
  }
  while(true);

  file.close();
  return result;
}

/*
 * Searches "/etc/pacman.conf" to retrive IgnorePkg items (if any)
 */
QStringList UnixCommand::getIgnorePkgsFromPacmanConf()
{
  QStringList resPkgs;
  QStringList resGroups;

  resPkgs = getFieldFromPacmanConf(QStringLiteral("IgnorePkg"));
  resGroups = getFieldFromPacmanConf(QStringLiteral("IgnoreGroup"));

  if (!resGroups.isEmpty())
  {
    //Let's retrieve all pkgs that live inside each group
    foreach (QString group, resGroups)
    {
      QStringList *packagesOfGroup = Package::getPackagesOfGroup(group);
      if (!packagesOfGroup->isEmpty())
      {
        foreach (QString pkg, *packagesOfGroup)
        {
          resPkgs.append(pkg);
        }
      }
    }
  }

  resPkgs.removeDuplicates();
  return resPkgs;
}

/*
 * Retrieves the LinuxDistro where Octopi is running on!
 * Reads file "/etc/os-release" and searchs for compatible Octopi distros
 */
LinuxDistro UnixCommand::getLinuxDistro()
{
  static LinuxDistro ret;
  static bool firstTime = true;

  if (firstTime)
  {
    if (QFile::exists(QStringLiteral("/etc/os-release")))
    {
      QFile file(QStringLiteral("/etc/os-release"));

      if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        ret = ectn_UNKNOWN;

      QString contents = file.readAll();

      /*if (contents.contains(QRegularExpression("Antergos")))
      {
        ret = ectn_ANTERGOS;
      }*/
      if (contents.contains(QRegularExpression(QStringLiteral("ArchBang"))))
      {
        ret = ectn_ARCHBANGLINUX;
      }
      else if (contents.contains(QRegularExpression(QStringLiteral("Arch Linux"))) ||
               (contents.contains(QRegularExpression(QStringLiteral("Arcolinux")))))
      {
        ret = ectn_ARCHLINUX;
      }
      else if (contents.contains(QRegularExpression(QStringLiteral("Chakra"))))
      {
        ret = ectn_CHAKRA;
      }
      else if (contents.contains(QRegularExpression(QStringLiteral("Condres OS"))))
      {
        ret = ectn_CONDRESOS;
      }
      else if (contents.contains(QRegularExpression(QStringLiteral("EndeavourOS"))))
      {
        ret = ectn_ENDEAVOUROS;
      }
      else if (contents.contains(QRegularExpression(QStringLiteral("KaOS"))))
      {
        ret = ectn_KAOS;
      }
      else if (contents.contains(QRegularExpression(QStringLiteral("Manjaro"))))
      {
        ret = ectn_MANJAROLINUX;
      }
      /*else if (contents.contains(QRegularExpression("Netrunner")))
      {
        ret = ectn_NETRUNNER;
      }*/
      else if (contents.contains(QRegularExpression(QStringLiteral("Parabola GNU/Linux-libre"))))
      {
        ret = ectn_PARABOLA;
      }
      /*else if (contents.contains(QRegularExpression("SwagArch")))
      {
        ret = ectn_SWAGARCH;
      }*/
      else
      {
        ret = ectn_UNKNOWN;
      }

      firstTime = false;

      file.close();
    }
  }

  return ret;
}

/*
 * Returns the PrettyName info from /etc/os-release
 */
QString UnixCommand::getLinuxDistroPrettyName()
{
  static QString ret(QLatin1String(""));
  static bool firstTime = true;

  if (firstTime)
  {
    if (QFile::exists(QStringLiteral("/etc/os-release")))
    {
      QFile file(QStringLiteral("/etc/os-release"));
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        ret = QLatin1String("");

      QString contents = file.readAll();
      int ind = contents.indexOf(QLatin1String("PRETTY_NAME"), Qt::CaseInsensitive);
      int end = contents.indexOf(QLatin1String("\n"), ind);

      if (ind != -1)
      {
        ret = contents.mid(ind+13, end-(ind+13)-1);
      }
    }

    firstTime = false;
  }

  return ret;
}

/*
 * Retrieves pacman version.
 */
QString UnixCommand::getPacmanVersion()
{
  QString v = performQuery(QStringLiteral("--version"));
  QString res = QStringLiteral("???");
  int p = v.indexOf(QLatin1String("Pacman"));
  int q = v.indexOf(QLatin1String("- libalpm"));

  if (p >=0 && q >= 0)
  {
    res = v.mid(p+6, q-(p+6)).trimmed();
  }

  return res;
}

/*
 * Tests if the installed pacman version is >= 5.1
 */
bool UnixCommand::isPacmanFiveDotOneOrHigher()
{
  bool res = false;
  QString major, minor;

  //v5.1.0
  QString pacmanVersion = UnixCommand::getPacmanVersion();
  if (pacmanVersion.length() == 6)
  {
    major = pacmanVersion.at(1);
    minor = pacmanVersion.at(3);
    int ma = major.toInt();
    int mi = minor.toInt();

    if (ma >=5)
    {
      if (mi >=1) res = true;
    }
  }

  return res;
}

/*
 * Check if /var/lib/pacman/db.lck exists
 */
bool UnixCommand::isPacmanDbLocked()
{
  return QFile::exists(ctn_PACMAN_DATABASE_LOCK_FILE);
}

/*
 * A bit of settings to better run "pacman" commands using QProcess
 */
QProcessEnvironment UnixCommand::getProcessEnvironment()
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove(QStringLiteral("LANG"));
  env.remove(QStringLiteral("LC_MESSAGES"));
  env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
  env.insert(QStringLiteral("LC_MESSAGES"), QStringLiteral("C"));
  env.remove(QStringLiteral("COLUMNS"));
  env.insert(QStringLiteral("COLUMNS"), QStringLiteral("132"));
  return env;
}

/*
 * Checks if Octopi/Octopi-notifier, cache-cleaner, etc is being executed
 */
bool UnixCommand::isOctopiHelperRunning()
{
  bool res=false;

  QProcess proc;
  proc.setProcessEnvironment(getProcessEnvironment());
  QString cmd = QStringLiteral("ps -C %1 -o command");
  QString octoToolName = ctn_OCTOPI_HELPER_NAME;
  cmd = cmd.arg(octoToolName);
  proc.start(cmd);
  proc.waitForFinished();
  QString out = proc.readAll().trimmed();
  if (out.contains(QLatin1String("|"))) return false;
  out=out.remove(QStringLiteral("\n"));
  out=out.remove(QStringLiteral("COMMAND"));
  if ((out == "/usr/lib/octopi/" + octoToolName) || out.contains("/usr/lib/octopi/" + octoToolName + " ")) res=true;

  return res;
}
