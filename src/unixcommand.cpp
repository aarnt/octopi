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
#include <iostream>

#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QTextStream>
#include <QtNetwork/QNetworkInterface>

QFile *UnixCommand::m_temporaryFile = 0;

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
    res = proc.readAllStandardError();

  proc.close();
  return res;
}

/*
 * Returns the path of given executable
 */
QString UnixCommand::discoverBinaryPath(const QString& binary){
  QProcess *proc = new QProcess;

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  proc->setProcessEnvironment(env);

  proc->start("/bin/sh -c \"which " + binary + "\"");
  proc->waitForFinished();
  QString res = proc->readAllStandardOutput();

  proc->close();
  delete proc;
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
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  pacman.setProcessEnvironment(env);

  QString commandStr = "\"yes | pacman -Scc\"";

  QString command = WMHelper::getSUCommand() + " " + commandStr;
  pacman.start(command);
  pacman.waitForFinished();

  return (pacman.exitCode() == 0);
}

/*
 * Performs a pacman query
 */
QByteArray UnixCommand::performQuery(const QStringList args)
{
  QByteArray result("");
  QProcess pacman;

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  pacman.setProcessEnvironment(env);

  pacman.start("pacman", args);
  pacman.waitForFinished(-1);
  result = pacman.readAllStandardOutput();

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
  pacman.setProcessEnvironment(env);

  pacman.start("pacman " + args);
  pacman.waitForFinished(-1);
  result = pacman.readAllStandardOutput();

  return result;
}

/*
 * Performs a yourt command
 */
QByteArray UnixCommand::performYaourtCommand(const QString &args)
{
  QByteArray result("");
  QProcess yaourt;

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  yaourt.setProcessEnvironment(env);

  yaourt.start("yaourt " + args);
  yaourt.waitForFinished(-1);
  result = yaourt.readAllStandardOutput();

  return result;
}

/*
 * Returns a string containing all yaourt packages given a searchString parameter
 */
QByteArray UnixCommand::getYaourtPackageList(const QString &searchString)
{
  QByteArray result("");
  QProcess yaourt;
  QProcess kill;

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  yaourt.setProcessEnvironment(env);

  kill.setStandardOutputProcess(&yaourt);
  kill.start("kill $$");
  yaourt.start("yaourt " + searchString);
  yaourt.waitForFinished(-1);

  result = yaourt.readAll();

  return result;
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
  QByteArray result = performQuery(QStringList("-Qu"));
  return result;
}

/*
 * Returns a string containing all packages that are not contained in any repository
 * (probably the ones installed by a tool such as yaourt)
 */
QByteArray UnixCommand::getForeignPackageList()
{
  QByteArray result = performQuery(QStringList("-Qm"));
  return result;
}

/*
 * Returns a string with the list of all packages available in all repositories
 * (installed + not installed)
 */
QByteArray UnixCommand::getPackageList()
{
  QByteArray result = performQuery(QStringList("-Ss"));
  return result;
}

/*
 * Given a package name and if it is default to the official repositories,
 * returns a string containing all the fields of its information
 * (ex: name, description, version, dependsOn...)
 */
QByteArray UnixCommand::getPackageInformation(const QString &pkgName, bool foreignPackage = false)
{
  QStringList args;

  if(foreignPackage)
    args << "-Qi";
  else
    args << "-Si";

  args << pkgName;

  QByteArray result = performQuery(args);
  return result;
}

/*
 * Given a package name, returns a string containing all the files inside it
 */
QByteArray UnixCommand::getPackageContents(const QString& pkgName)
{
  QStringList args;
  args << "-Ql";
  args << pkgName;

  QByteArray res = performQuery(args);
  return res;
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
  QProcess ping;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  ping.setProcessEnvironment(env);

  ping.start("ping -c 1 -W 3 www.google.com");
  ping.waitForFinished();

  int res = ping.exitCode();
  ping.close();

  return (res == 0);
}

/*
 * We must check if KTSUSS version is 1.3 or 1.4.
 */
bool UnixCommand::isKtsussVersionOK()
{
  QProcess proc;

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  proc.setProcessEnvironment(env);

  QStringList slParam("-v");
  proc.start("ktsuss", slParam );
  proc.waitForFinished();

  QString out = proc.readAllStandardOutput();
  proc.close();

  if (out.indexOf("ktsuss 1.3") != -1 || out.indexOf("ktsuss 1.4") != -1)
    return true;
  else
    return false;
}

/*
 * Checks if the given executable is available somewhere in the system
 */
bool UnixCommand::hasTheExecutable( const QString& exeName )
{
  QProcess* proc = new QProcess();
  proc->setProcessChannelMode(QProcess::MergedChannels);
  QString sParam = "\"which " + exeName + "\"";
  proc->start("/bin/sh -c " + sParam);
  proc->waitForFinished();

  QString out = proc->readAllStandardOutput();
  proc->close();

  delete proc;
  if (out.count("which") > 0) return false;
  else return true;
}

/*
 * Does some garbage collection, removing uneeded files
 */
void UnixCommand::removeTemporaryFiles()
{
  QDir tempDir(QDir::tempPath());
  QStringList nameFilters;
  nameFilters << "qtsingleapp*" << "gpg*" << ".qt_temp_*";
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
 * Given a filename, checks if it is a text file
 */
bool UnixCommand::isTextFile(const QString& fileName)
{
  QProcess *p = new QProcess();
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  p->setProcessEnvironment(env);

  QStringList s(fileName);
  p->start( "file", s );
  p->waitForFinished();

  QByteArray output = p->readAllStandardOutput();
  p->close();
  delete p;

  int from = output.indexOf(":", 0)+1;

  return (((output.indexOf( "ASCII", from ) != -1) ||
          (output.indexOf( "text", from ) != -1) ||
          (output.indexOf( "empty", from ) != -1)) &&
          (output.indexOf( "executable", from) == -1));
}

/*
 * Opens a root terminal
 */
void UnixCommand::openRootTerminal(){
  if(WMHelper::isXFCERunning() && UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
    QString cmd = WMHelper::getSUCommand() + " \"" + ctn_XFCE_TERMINAL + "\"";
    m_process->startDetached(cmd);
  }
  else if (WMHelper::isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE_TERMINAL)){
    QString cmd = WMHelper::getSUCommand() + " \"" + ctn_KDE_TERMINAL + "\"";
    m_process->startDetached(cmd);
  }
  else if (WMHelper::isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDE_TERMINAL)){
    QString cmd = WMHelper::getSUCommand() + " \"" + ctn_TDE_TERMINAL + "\"";
    m_process->startDetached(cmd);
  }
  else if (WMHelper::isLXDERunning() && UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
    QString cmd = WMHelper::getSUCommand() + " \"" + ctn_LXDE_TERMINAL + "\"";
    m_process->startDetached(cmd);
  }
  else if (WMHelper::isMATERunning() && UnixCommand::hasTheExecutable(ctn_MATE_TERMINAL)){
    QString cmd = WMHelper::getSUCommand() + " \"" + ctn_MATE_TERMINAL + "\"";
    m_process->startDetached(cmd);
  }
  else if (UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
    QString cmd = WMHelper::getSUCommand() + " \"" + ctn_XFCE_TERMINAL + "\"";
    m_process->startDetached(cmd);
  }
  else if (UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
    QString cmd = WMHelper::getSUCommand() + " \"" + ctn_LXDE_TERMINAL + "\"";
    m_process->startDetached(cmd);
  }
  else if (UnixCommand::hasTheExecutable(ctn_XTERM)){
    QString cmd = WMHelper::getSUCommand() + " \"" + ctn_XTERM +
        " -fn \"*-fixed-*-*-*-18-*\" -fg White -bg Black -title xterm \"";
    m_process->startDetached(cmd);
  }
}

/*
 * Executes given commandToRun inside a terminal, so the user can interact
 */
void UnixCommand::runCommandInTerminal(const QStringList& commandList){
  QFile *ftemp = getTemporaryFile();
  QTextStream out(ftemp);

  foreach(QString line, commandList)
    out << line;

  out.flush();
  ftemp->close();

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove("LANG");
  env.remove("LC_MESSAGES");
  env.insert("LANG", QLocale::system().name() + ".UTF-8");
  env.insert("LC_MESSAGES", QLocale::system().name() + ".UTF-8");
  m_process->setProcessEnvironment(env);

  QString suCommand = WMHelper::getSUCommand();

  if(WMHelper::isXFCERunning() && UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
    QString cmd = suCommand + " \"" + ctn_XFCE_TERMINAL + " -e \'bash -c " + ftemp->fileName() + "'\"";
    m_process->start(cmd);
  }
  else if (WMHelper::isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE_TERMINAL)){
    QString cmd = suCommand + " \"" + ctn_KDE_TERMINAL + " --nofork -e bash -c " + ftemp->fileName() + "\"";
    m_process->start(cmd);
  }
  else if (WMHelper::isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDE_TERMINAL)){
    QString cmd = suCommand + " \"" + ctn_TDE_TERMINAL + " --nofork -e bash -c " + ftemp->fileName() + "\"";
    m_process->start(cmd);
  }
  else if (WMHelper::isLXDERunning() && UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
    QString cmd = suCommand + " \"" + ctn_LXDE_TERMINAL + " -e \'bash -c " + ftemp->fileName() + "'\"";
    m_process->start(cmd);
  }
  else if (WMHelper::isMATERunning() && UnixCommand::hasTheExecutable(ctn_MATE_TERMINAL)){
    QString cmd = suCommand + " \"" + ctn_MATE_TERMINAL + " -e \'bash -c " + ftemp->fileName() + "'\"";
    m_process->start(cmd);
  }
  else if (UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
    QString cmd = suCommand + " \"" + ctn_XFCE_TERMINAL + " -e \'bash -c " + ftemp->fileName() + "'\"";
    m_process->start(cmd);
  }
  else if (UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
    QString cmd = suCommand + " \"" + ctn_LXDE_TERMINAL + " -e \'bash -c " + ftemp->fileName() + "'\"";
    m_process->start(cmd);
  }
  else if (UnixCommand::hasTheExecutable(ctn_XTERM)){
    QString cmd = suCommand + " \"" + ctn_XTERM +
        " -fn \"*-fixed-*-*-*-18-*\" -fg White -bg Black -title xterm -e \'bash -c " + ftemp->fileName() + "'\"";
    m_process->start(cmd);
  }
}

/*
 * Executes given commandToRun inside a terminal, as the current user!
 */
void UnixCommand::runCommandInTerminalAsNormalUser(const QStringList &commandList)
{
  QFile *ftemp = getTemporaryFile();
  QTextStream out(ftemp);

  foreach(QString line, commandList)
    out << line;

  out.flush();
  ftemp->close();

  QString suCommand = WMHelper::getSUCommand();
  QString loggedUser = QDir::homePath();
  QFileInfo fi(loggedUser);
  loggedUser = fi.fileName();
  suCommand.replace("root", loggedUser);

  if(WMHelper::isXFCERunning() && UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
    QString cmd = ctn_XFCE_TERMINAL + " -e " + ftemp->fileName();
    m_processWrapper->executeCommand(cmd);
  }
  else if (WMHelper::isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE_TERMINAL)){
    QString cmd = ctn_KDE_TERMINAL + " --nofork -e " + ftemp->fileName();
    m_processWrapper->executeCommand(cmd);
  }
  else if (WMHelper::isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDE_TERMINAL)){
    QString cmd = ctn_TDE_TERMINAL + " --nofork -e " + ftemp->fileName();
    m_processWrapper->executeCommand(cmd);
  }
  if (WMHelper::isLXDERunning() && UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
    QString cmd = ctn_LXDE_TERMINAL + " -e " + ftemp->fileName();
    m_processWrapper->executeCommand(cmd);
  }
  else if (WMHelper::isMATERunning() && UnixCommand::hasTheExecutable(ctn_MATE_TERMINAL)){
    QString cmd = ctn_MATE_TERMINAL + " -e " + ftemp->fileName();
    m_processWrapper->executeCommand(cmd);
  }
  else if (UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
    QString cmd = ctn_XFCE_TERMINAL + " -e " + ftemp->fileName();
    m_processWrapper->executeCommand(cmd);
  }
  else if (UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
    QString cmd = ctn_LXDE_TERMINAL + " -e " + ftemp->fileName();
    m_processWrapper->executeCommand(cmd);
  }
  else if (UnixCommand::hasTheExecutable(ctn_XTERM)){
    QString cmd = ctn_XTERM +
        " -fn \"*-fixed-*-*-*-18-*\" -fg White -bg Black -title xterm -e " + ftemp->fileName();
    m_processWrapper->executeCommand(cmd);
  }
}

/*
 * Executes the given command using QProcess async technology
 */
void UnixCommand::executeCommand(const QString &pCommand)
{
  QString command = WMHelper::getSUCommand() + "\"" + pCommand + "\"";

  m_process->start(command);
}

/*
 * Executes the given commandlist using QProcess async technology
 */
void UnixCommand::executePackageActions( const QStringList& commandList )
{
  QFile *ftemp = getTemporaryFile();
  QTextStream out(ftemp);

  foreach(QString line, commandList)
    out << line;

  out.flush();
  ftemp->close(); 

  QString command = WMHelper::getSUCommand() + " " + ftemp->fileName();

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  m_process->setProcessEnvironment(env);
  m_process->start(command);
}

/*
 * Puts all Standard output of the member process into a member string
 */
void UnixCommand::processReadyReadStandardOutput()
{
  m_readAllStandardOutput = m_process->readAllStandardOutput();
}

/*
 * Puts all StandardError output of the member process into a member string
 */
void UnixCommand::processReadyReadStandardError()
{
  m_readAllStandardError = m_process->readAllStandardError();
  m_errorString = m_process->errorString();
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

/*
 * UnixCommand's constructor: the relevant environment english setting and the connectors
 */
UnixCommand::UnixCommand(QObject *parent): QObject()
{
  m_process = new QProcess(parent);
  m_processWrapper = new ProcessWrapper(parent);

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

  //ProcessWrapper signals
  QObject::connect(m_processWrapper, SIGNAL( startedTerminal()), this,
                   SIGNAL( startedTerminal()));
  QObject::connect(m_processWrapper, SIGNAL( finishedTerminal(int,QProcess::ExitStatus)), this,
                   SIGNAL( finishedTerminal(int,QProcess::ExitStatus)));
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
  int start=0;

  if (end != -1)
  {
    //Does it contains a # before it???
    start = end;
    do{
      start--;
    }while (contents.at(start) != '\n');

    QString str = contents.mid(start+1, (end-start-1)).trimmed();

    if (str.isEmpty()) res = true;
    else res = false;
  }

  return res;
}

/*
 * Searches "/etc/pacman.conf" to retrive IgnorePkg items (if any)
 */
QStringList UnixCommand::getIgnorePkg()
{
  QStringList res;
  QFile file("/etc/pacman.conf");

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return res;

  QString contents = file.readAll();
  int from = 0;
  do
  {
    int end = contents.indexOf("IgnorePkg", from, Qt::CaseInsensitive);
    int start=0;

    if (end != -1)
    {
      //Does it contains a # before it???
      start = end;
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
        res = ignorePkg.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        break;
      }
      else from += end+9;
    }
    else break;
  }
  while(true);

  return res;
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
    QFile file("/etc/os-release");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      ret = ectn_UNKNOWN;

    QString contents = file.readAll();

    if (contents.contains(QRegExp("ArchBang")))
    {
      ret = ectn_ARCHBANGLINUX;
    }
    else if (contents.contains(QRegExp("Arch Linux")))
    {
      ret = ectn_ARCHLINUX;
    }
    else if (contents.contains(QRegExp("Chakra")))
    {
      ret = ectn_CHAKRA;
    }
    else if (contents.contains(QRegExp("Manjaro")))
    {
      ret = ectn_MANJAROLINUX;
    }
    else
    {
      ret = ectn_UNKNOWN;
    }

    firstTime = false;
  }

  return ret;
}
