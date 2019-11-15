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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
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
  env.remove("LANG");
  env.remove("LC_MESSAGES");
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  proc.setProcessEnvironment(env);
  proc.start(commandToRun);
  proc.waitForStarted();
  proc.waitForFinished(-1);

  QString res("");

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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
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
  QString commandStr = "\"yes | pacman -Scc\"";
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  env.insert("LC_ALL", "C");
  pacman.setProcessEnvironment(env);

  pacman.start("pacman", args);
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  env.insert("LC_ALL", "C");
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");

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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  env.insert("LANGUAGE", "C");

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
    res.remove("[1m");
    res.remove("[m");
    res.remove("[1;32m");
    res.remove("[1;34m");
    res.remove("[1;36m");

    return res.toLatin1();
  }
  else if (UnixCommand::getLinuxDistro() == ectn_CHAKRA)
  {
    QString res = result;
    res.remove("\033");
    res.remove("[0m");
    res.remove("[1m");
    res.remove("[m");
    res.remove("[32m");
    res.remove("[35m");

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
  QString shell = env.value("SHELL", "/bin/bash");

  QFileInfo fi(shell);

  if (fi.fileName() == "fish")
    return "bash";
  else
    return fi.fileName();
}

/*
 * Returns a string containing all packages no one depends on
 */
QByteArray UnixCommand::getUnrequiredPackageList()
{
  QByteArray result = performQuery(QStringList("-Qt"));
  return result;
}

/*
 * Returns a string containing all packages that are outdated since last DB sync
 */
QByteArray UnixCommand::getOutdatedPackageList()
{
  return performQuery(QStringList("-Qu"));
}

/*
 * Returns a string containing all AUR outdated packages
 */
QByteArray UnixCommand::getOutdatedAURPackageList()
{
  QByteArray result;

  if (Package::getForeignRepositoryToolName() == ctn_KCP_TOOL)
  {
    result = performAURCommand("-lO");
  }
  else if (Package::getForeignRepositoryToolName() != ctn_KCP_TOOL)
  {
    result = performAURCommand("-Qua");
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
  QByteArray result = performQuery(QStringList("-Qem"));
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
    result = performQuery(QStringList("-Ss"));
  else
  {
    QStringList sl;
    sl << "-Ss";
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  env.insert("LC_ALL", "C");
  expac.setProcessEnvironment(env);
  QString expacStr = "expac -s \"%%1\" %2";
  expac.start(expacStr.arg(info).arg(pkgName));
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
    args << "-Qi";
  else
    args << "-Si";

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
  args << "-Ql";
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

  pkgfile.start("pkgfile -V");
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
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
  sl << "-Qo";
  sl << filePath;

  QString out = performQuery(sl);
  QStringList s = out.split("\n", QString::SkipEmptyParts);

  if (s.count() >= 1)
  {
    QStringList res = s.at(0).split(" ", QString::SkipEmptyParts);
    return res.at(res.count()-2);
  }
  else return "";
}

/*
 * Based on the given file, we use 'slocate' to suggest complete paths
 */
QStringList UnixCommand::getFilePathSuggestions(const QString &file)
{
  QProcess slocate;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  slocate.setProcessEnvironment(env);
  slocate.start("slocate -l 8 " + file);
  slocate.waitForFinished();

  QString ba = slocate.readAllStandardOutput();
  return ba.split("\n", QString::SkipEmptyParts);
}

/*
 * Retrives the list of package groups
 */
QByteArray UnixCommand::getPackageGroups()
{
  QByteArray res = performQuery(QStringList("-Sg"));
  return res;
}

/*
 * Given a group name, returns a string containing all packages from it
 */
QByteArray UnixCommand::getPackagesFromGroup(const QString &groupName)
{
  QByteArray res =
      performQuery(QString("--print-format \"%r %n\" -Spg " ) + groupName);

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
    args = "--print-format \"%n %v %s\" -Spu";
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  proc.setProcessEnvironment(env);

  slParam << "-m";
  proc.start("uname", slParam);
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
  QString hostname = "www.google.com";

  socket.connectToHost(hostname, 80);
  if (socket.waitForConnected(5000))
    return true;
  else
  {
    hostname = "www.baidu.com";
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

  if (out.isEmpty() || out.count("which") > 0) return false;
  else return true;
}

/*
 * Does some garbage collection, removing uneeded files
 */
void UnixCommand::removeTemporaryFiles()
{
  QDir tempDir(QDir::tempPath());
  QStringList nameFilters;
  nameFilters << "qtsingleapp-Octopi*" << "qtsingleapp-CacheC*" << "qtsingleapp-Reposi*"
              << "qipc_sharedmemory_orgarntoctopi*"
              << "qipc_systemsem_orgarntoctopi*"
              << ".qt_temp_octopi*";
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

  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  p.setProcessEnvironment(env);

  p.start(WMHelper::getSUCommand() + "\"" + pCommand + "\"");
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
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  p->setProcessEnvironment(env);

  QStringList s(fileName);
  p->start( "file", s );
  p->waitForFinished();

  QString output = p->readAllStandardOutput();
  p->close();
  delete p;

  //If it's a symbolic link, let's discover what is the real target
  if (output.contains(": symbolic link to"))
  {
    QFileInfo fi(fileName);
    fileName = fi.symLinkTarget();
    goto init;
  }
  else
  {
    int from = output.indexOf(":", 0)+1;

    return (((output.indexOf( "ASCII", from ) != -1) ||
            (output.indexOf( "text", from ) != -1) ||
            (output.indexOf( "empty", from ) != -1)) &&
            (output.indexOf( "executable", from) == -1));
  }
}

/*
 * Executes given commandList as root inside a terminal, so the user can interact
 */
void UnixCommand::runCommandInTerminal(const QStringList& commandList){
  m_terminal->runCommandInTerminal(commandList);
}

/*
 * Executes given commandList as root inside a terminal using "octopi-helper -t" (passing cmds thru memory)
 */
void UnixCommand::runOctopiHelperInTerminalWithSharedMem(const QStringList &commandList, QSharedMemory *sharedMem)
{
  m_terminal->runOctopiHelperInTerminalWithSharedMem(commandList, sharedMem);
}

/*
 * Executes given commandList as root inside a terminal using "octopi-helper -t", so the user can interact
 */
void UnixCommand::runOctopiHelperInTerminal(const QStringList &commandList)
{
  m_terminal->runOctopiHelperInTerminal(commandList);
}

/*
 * Executes given commandList inside a terminal, as the current user!
 */
void UnixCommand::runCommandInTerminalAsNormalUser(const QStringList &commandList)
{
  m_terminal->runCommandInTerminalAsNormalUser(commandList);
}

/*
 * Executes the given command using QProcess async technology with ROOT credentials
 */
void UnixCommand::executeCommandWithSharedMem(const QString &pCommand, QSharedMemory *sharedMem)
{
  QString command;

  //COLUMNS variable code!
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove("LANG");
  env.remove("LC_MESSAGES");
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  env.remove("COLUMNS");
  env.insert("COLUMNS", "132");
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
void UnixCommand::executeCommand(const QString &pCommand, Language lang)
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
  else //We are not using "octopi-sudo" nor "lxqt-sudo" utility...*/
  {
    command = suCommand + "\"" + pCommand + "\"";
  }

  m_process->start(command);
}

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
  octopiHelperCommandParameter = " -ts";
  suCommand += ctn_OCTOPI_HELPER + octopiHelperCommandParameter;

  QStringList commandList;
  QString commands;
  QByteArray sharedData;

  //If this is a multiple command string, let's break it
  if (pCommand.contains(";"))
  {
    commandList = pCommand.split(";", QString::SkipEmptyParts);
    foreach(QString line, commandList)
    {
      commands += line.trimmed() + "\n";
    }
  }
  else //We have just one command here
  {
    commandList << pCommand.trimmed();
    commands += commandList.first();
  }

  sharedData=commands.toLatin1();

  if (sharedMem != nullptr)
  {
    if (sharedMem->isAttached())
      sharedMem->detach();
    delete sharedMem;
    sharedMem=nullptr;
  }

  sharedMem=new QSharedMemory("org.arnt.octopi", this);
  sharedMem->create(sharedData.size());
  sharedMem->lock();
  memcpy(sharedMem->data(), sharedData.data(), sharedData.size());
  sharedMem->unlock();

  return suCommand;
}

/*
 * Constructs the octopi-sudo related command to execute given pCommand using "octopi-helper" utility
 */
QString UnixCommand::buildOctopiHelperCommand(const QString &pCommand)
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
}

/*
 * Cancels the running process
 */
void UnixCommand::cancelProcess(QSharedMemory *sharedMem)
{
  //Q_UNUSED(sharedMem)
  QProcess pacman;
  QString suCommand = WMHelper::getSUCommand();
  QString pCommand = "killall pacman; rm " + ctn_PACMAN_DATABASE_LOCK_FILE;
  QString result;

  if (suCommand == WMHelper::getOctopiSudoCommand())
  {
    removeTemporaryFiles();
    result = buildOctopiHelperCommandWithSharedMem(pCommand, sharedMem);
  }
  else {
    result = suCommand + "\"" + pCommand + "\"";
  }

  pacman.start(result);
  pacman.waitForFinished(-1);
}

/*
 * If justOneInstance = false (default), returns TRUE if one instance of the app is ALREADY running
 * Otherwise, it returns TRUE if the given app is running.
 */
bool UnixCommand::isAppRunning(const QString &appName, bool justOneInstance)
{
  QStringList slParam;
  QProcess proc;

  slParam << "-C";
  slParam << appName;
  proc.start("ps", slParam);
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
 * Checks if current user is in "wheel" group
 */
bool UnixCommand::isUserInWheelGroup()
{
  bool result=false;
  QProcess groups;
  groups.start("groups");
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
  QFile file("/etc/pacman.conf");

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QString contents = file.readAll();
  int end = contents.indexOf("ILoveCandy");
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
  QFile file("/etc/pacman.conf");
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
        int equal = ignorePkg.indexOf("=");
        int newLine = ignorePkg.indexOf("\n");

        ignorePkg = ignorePkg.mid(equal+1, newLine-(equal+1)).trimmed();
        result += ignorePkg.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
        from = end + newLine;
      }
      else if (str != "#")
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

  resPkgs = getFieldFromPacmanConf("IgnorePkg");
  resGroups = getFieldFromPacmanConf("IgnoreGroup");

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
    if (QFile::exists("/etc/os-release"))
    {
      QFile file("/etc/os-release");

      if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        ret = ectn_UNKNOWN;

      QString contents = file.readAll();

      if (contents.contains(QRegularExpression("Antergos")))
      {
        ret = ectn_ANTERGOS;
      }
      else if (contents.contains(QRegularExpression("ArchBang")))
      {
        ret = ectn_ARCHBANGLINUX;
      }
      else if (contents.contains(QRegularExpression("Arch Linux")) ||
               (contents.contains(QRegularExpression("arcolinux"))))
      {
        ret = ectn_ARCHLINUX;
      }
      else if (contents.contains(QRegularExpression("Chakra")))
      {
        ret = ectn_CHAKRA;
      }
      else if (contents.contains(QRegularExpression("Condres OS")))
      {
        ret = ectn_CONDRESOS;
      }
      else if (contents.contains(QRegularExpression("EndeavourOS")))
      {
        ret = ectn_ENDEAVOUROS;
      }
      else if (contents.contains(QRegularExpression("KaOS")))
      {
        ret = ectn_KAOS;
      }
      else if (contents.contains(QRegularExpression("Manjaro")))
      {
        ret = ectn_MANJAROLINUX;
      }
      else if (contents.contains(QRegularExpression("Netrunner")))
      {
        ret = ectn_NETRUNNER;
      }
      else if (contents.contains(QRegularExpression("Parabola GNU/Linux-libre")))
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
  static QString ret("");
  static bool firstTime = true;

  if (firstTime)
  {
    if (QFile::exists("/etc/os-release"))
    {
      QFile file("/etc/os-release");
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        ret = "";

      QString contents = file.readAll();
      int ind = contents.indexOf("PRETTY_NAME", Qt::CaseInsensitive);
      int end = contents.indexOf("\n", ind);

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
  QString v = performQuery("--version");
  QString res = "???";
  int p = v.indexOf("Pacman");
  int q = v.indexOf("- libalpm");

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
