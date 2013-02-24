/*
* This file is part of Octopi, an open-source GUI for ArchLinux pacman.
* Copyright (C) 2013  Alexandre Albuquerque Arnt
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
#include "package.h"
#include "strconstants.h"
#include "wmhelper.h"
#include <iostream>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QApplication>
#include <QMessageBox>
#include <QTextStream>
#include <QtNetwork/QNetworkInterface>

QFile *UnixCommand::m_temporaryFile = 0;

//Execute the given command and returns the StandardError Output.
QString UnixCommand::runCommand(const QString& commandToRun){
  QProcess proc;

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  proc.setProcessEnvironment(env);
#endif

  proc.start(commandToRun);
  proc.waitForStarted();
  proc.waitForFinished(-1);
  QString res = proc.readAllStandardError();
  proc.close();
  return res;
}

//Execute the CURL command and returns the StandardError Output, if result code <> 0.
QString UnixCommand::runCurlCommand(const QString& commandToRun){
  QProcess proc;

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  proc.setProcessEnvironment(env);
#endif

  proc.start(commandToRun);
  proc.waitForStarted();
  proc.waitForFinished(-1);

  QString res("");
  if (proc.exitCode() != 0)
    res = proc.readAllStandardError();

  proc.close();
  return res;
}

//Returns the path of a given executable
QString UnixCommand::discoverBinaryPath(const QString& binary){
  QProcess *proc = new QProcess;

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  proc->setProcessEnvironment(env);
#endif

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

//Verifies if SPKG is installed correctly. Returns TRUE or FALSE.
bool UnixCommand::isSpkgInstalled(){
  return (discoverBinaryPath(ctn_SPKG) != "");
}

bool UnixCommand::isSlackPackage(const QString& filePath){
  bool bResult = false;

  QProcess tar;

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  tar.setProcessEnvironment(env);
#endif

  QStringList args;
  args << "-Otf";
  args << filePath;
  args << "install/";
  tar.start("tar", args);

  QString result("");
  tar.waitForFinished(-1);
  result = tar.readAllStandardOutput();

  if (result == ""){
    result = tar.readAllStandardError();
    if (result == "")
      bResult = false;
    else if (result.contains("install/") || result.contains("not found", Qt::CaseInsensitive) >= 0)
      bResult = true;
  }
  else
    bResult = true;

  tar.close();
  return bResult;
}

QByteArray UnixCommand::getPackageInformation( const QString &pkgName, bool installed ){
  QByteArray result("");
  QProcess tar;

  if ( installed == false ) {
    //"tar -Oxzf lives-0.9.8-i486-1gas.tgz install/slack-desc > slack-desc"
    QStringList args;

    if (pkgName.endsWith(ctn_TGZ_PACKAGE_EXTENSION)) args << "-Oxzf";
    else args << "-OxJf";

    args << pkgName;
    args << "install/slack-desc";
    tar.start("tar", args);

    if (!tar.waitForFinished(100)){
      result = tar.readAllStandardOutput();
      if (result == ""){
        tar.waitForFinished();
        result = tar.readAllStandardOutput();
      }
    }

    if (result == "") result = tar.readAllStandardOutput();
  }

  else if ( installed == true ){
    QStringList args;

    args << pkgName;
    tar.start("cat", args);
    if (!(tar.waitForStarted())) return "";
    if (!(tar.waitForFinished())) return "";
    result = tar.readAll();
  }

  tar.close();
  return result;
}

QByteArray UnixCommand::getPackageContents(const QString& pkgName){
  QByteArray res;
  QProcess tar;

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  tar.setProcessEnvironment(env);
#endif

  QStringList args;
  args << "-Otf";
  args << pkgName;

  tar.start ( "tar", args );
  if ( !( tar.waitForStarted() ) ) return ctn_PKG_CONTENT_ERROR.toAscii();
  if ( !( tar.waitForFinished(-1) ) ) return ctn_PKG_CONTENT_ERROR.toAscii();

  res = tar.readAllStandardError();
  tar.close();

  if (res == ctn_XZ_TAR_ERROR || res == ctn_GZ_TAR_ERROR) return ctn_PKG_CONTENT_ERROR.toAscii();

  return res;
}

QString UnixCommand::getSlackArchitecture(){
  QStringList slParam;
  QProcess proc;

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  proc.setProcessEnvironment(env);
#endif

  slParam << "-m";
  proc.start("uname", slParam);
  proc.waitForFinished();

  QString out = proc.readAllStandardOutput();
  proc.close();

  return out;
}

/*bool UnixCommand::isAuthenticPackage(const QString& packageName){
  QProcess gpgProcess;

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  gpgProcess.setProcessEnvironment(env);
#endif

  gpgProcess.start("gpg --status-fd 1 --verify " + packageName + ctn_GPG_SIGNATURE_EXTENSION);
  gpgProcess.waitForFinished(-1);

  int exitCode = gpgProcess.exitCode();
  gpgProcess.close();

  return (exitCode == 0);
}*/

void UnixCommand::installSlackGPGKey(const QString& gpgKeyPath){
  QProcess gpgProcess;
  QString gpgCommand = "gpg --import " + gpgKeyPath;

  gpgProcess.start(gpgCommand);
  gpgProcess.waitForFinished(-1);
  gpgProcess.close();
}

bool UnixCommand::hasInternetConnection(){
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
  return result;
}

/*bool UnixCommand::hasSlackGPGKeyInstalled(QString slackVersion){
  QProcess gpgProcess;

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  gpgProcess.setProcessEnvironment(env);
#endif

  QString gpgCommand = "gpg --fingerprint";
  gpgProcess.start(gpgCommand);
  gpgProcess.waitForFinished(-1);

  QString output = gpgProcess.readAllStandardOutput();
  gpgProcess.close();

  //If we have Slackware for ARM archs...
  if (slackVersion.contains("arm", Qt::CaseInsensitive)){
    return (output.indexOf(ctn_LIST_GPG_SLACKWARE_ARM_KEY) != -1);
  }
  else { //Otherwise, we are dealing with 32 or 64 bit intel/AMD
    return (output.indexOf(ctn_LIST_GPG_SLACKWARE_KEY) != -1);
  }
}*/

//We must check if KTSUSS version is 1.3 or 1.4.
bool UnixCommand::isKtsussVersionOK(){
  QProcess proc;

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  proc.setProcessEnvironment(env);
#endif

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

bool UnixCommand::hasTheExecutable( const QString& exeName ){
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

void UnixCommand::removeTemporaryFiles(){
  QDir tempDir(QDir::tempPath());
  QStringList nameFilters;
  nameFilters << "qtsingleapp*" << "gpg*";
  QFileInfoList list = tempDir.entryInfoList(nameFilters, QDir::Dirs | QDir::Files | QDir::System);

  foreach(QFileInfo file, list){
    QFile fileAux(file.filePath());
    //std::cout << "Found: " << file.fileName().toAscii().data() << std::endl;

    if (!file.isDir()){
      //std::cout << "trying to remove " << file.fileName().toAscii().data() << std::endl;
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

bool UnixCommand::isTextFile( const QString& fileName ){
  QProcess *p = new QProcess();

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  p->setProcessEnvironment(env);
#endif

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

QString UnixCommand::executeDiffToEachOther( QString pkg1, QString pkg2 ){
  QStringList sl = Package::getContents(pkg1);
  QCoreApplication::processEvents();
  QStringList sl2 = Package::getContents(pkg2);
  QCoreApplication::processEvents();

  if ( sl.isEmpty() && sl2.isEmpty() ) return 0;

  sl.sort();
  sl2.sort();
  QFile fPkg("/tmp/tempPkg");
  QFile fIPkg("/tmp/tempIPkg");

  fPkg.open(QIODevice::ReadWrite | QIODevice::Text);
  fIPkg.open(QIODevice::ReadWrite | QIODevice::Text);
  QTextStream tsp(&fPkg);
  QTextStream tsip(&fIPkg);

  foreach(QString s, sl) tsp << s << endl;
  foreach(QString s, sl2) tsip << s << endl;

  QStringList slParam;
  QProcess proc;

  slParam << "--suppress-common-lines";
  slParam << fPkg.fileName();
  slParam << fIPkg.fileName();

  proc.start("diff", slParam);
  proc.waitForStarted();
  proc.waitForFinished();

  fPkg.close();
  fIPkg.close();
  fPkg.remove();
  fIPkg.remove();

  QString result = proc.readAllStandardOutput();

  if (result != 0)
    return result;
  else
    return ctn_PACKAGES_WITH_SAME_CONTENT;
}

QString UnixCommand::executeDiffToInstalled( QString pkg, QString installedPackage ){
  QStringList sl = Package::getContents(ctn_PACKAGES_DIR + QDir::separator() + installedPackage, true);
  QCoreApplication::processEvents();
  QStringList sl2 = Package::getContents(pkg);
  QCoreApplication::processEvents();

  if ( sl2.isEmpty() ) return 0;

  sl.sort();
  sl2.sort();

  QFile fPkg("/tmp/tempPkg");
  QFile fIPkg("/tmp/tempIPkg");

  fPkg.open(QIODevice::ReadWrite | QIODevice::Text);
  fIPkg.open(QIODevice::ReadWrite | QIODevice::Text);
  QTextStream tsp(&fPkg);
  QTextStream tsip(&fIPkg);

  foreach(QString s, sl2) tsp << s << endl;
  foreach(QString s, sl) tsip << s << endl;

  //Here, we execute the diff
  QStringList slParam;
  QProcess proc;

  slParam << "--suppress-common-lines";
  slParam << fPkg.fileName();
  slParam << fIPkg.fileName();
  proc.start("diff", slParam);
  proc.waitForStarted();
  proc.waitForFinished();

  fPkg.close();
  fIPkg.close();
  fPkg.remove();
  fIPkg.remove();

  return proc.readAllStandardOutput();
}

void UnixCommand::executePackageActions( const QStringList& commandList ){
  QFile *ftemp = getTemporaryFile();
  QTextStream out(ftemp);

  foreach(QString line, commandList)
    out << line;

  out.flush();
  ftemp->close(); 

  QString command = WMHelper::getSUCommand() + " " + ftemp->fileName();
  m_process->start(command);
}

void UnixCommand::transformTGZinLZM( const QStringList& commandList, LZMCommand commandUsed ){
  QFile *ftemp = getTemporaryFile();
  QTextStream out(ftemp);
  QFileInfo fi(commandList[0]);
  m_process->setWorkingDirectory(fi.absolutePath());

  if (commandUsed == ectn_TXZ2SB) {
    foreach(QString line, commandList){
      out << "echo -e " << StrConstants::getExecutingCommand() <<
             " \\\x27" << "txz2sb " << line << "\\\x27...\n";
      out << "txz2sb " << line << "\n";
    }
  }
  else if (commandUsed == ectn_TGZ2LZM) {
    foreach(QString line, commandList){
      QFileInfo fi(line);
      QString newFile = fi.fileName();
      newFile = newFile.replace(".tgz", ".lzm");
      out << "echo -e " << StrConstants::getExecutingCommand() <<
             " \\\x27" << "tgz2lzm " << line << " " << newFile << "\\\x27...\n";
      out << "tgz2lzm " << line << " " << newFile << "\n";
    }
  }
  else if (commandUsed == ectn_MAKELZM) {
    foreach(QString line, commandList){
      out << "echo -e " << StrConstants::getExecutingCommand() <<
             " \\\x27" << "make-lzm " << line << "\\\x27...\n";
      out << "make-lzm " << line << "\n";
    }
  }

  out.flush();
  ftemp->close();
  m_process->start(WMHelper::getSUCommand() + " " + ftemp->fileName());
}

void UnixCommand::transformRPMinTGZ(const QStringList& commandList){
  QFile *ftemp = getTemporaryFile();
  QTextStream out(ftemp);
  QFileInfo fi(commandList[0]);
  m_process->setWorkingDirectory(fi.absolutePath());

  foreach(QString line, commandList){
    out << "echo -e " << StrConstants::getExecutingCommand() <<
           " \\\x27" << "/usr/bin/rpm2tgz " << line << "\\\x27...\n";
    out << "/usr/bin/rpm2tgz " << line << "\n";
  }

  out.flush();
  ftemp->close();
  m_process->start(WMHelper::getSUCommand() + " " + ftemp->fileName());
}

void UnixCommand::transformRPMinTXZ(const QStringList &commandList){
  QFile *ftemp = getTemporaryFile();
  QTextStream out(ftemp);
  QFileInfo fi(commandList[0]);
  m_process->setWorkingDirectory(fi.absolutePath());

  foreach(QString line, commandList){
    out << "echo -e " << StrConstants::getExecutingCommand() <<
           " \\\x27" << "/usr/bin/rpm2txz " << line << "\\\x27...\n";
    out << "/usr/bin/rpm2txz " << line << "\n";
  }

  out.flush();
  ftemp->close();
  m_process->start(WMHelper::getSUCommand() + " " + ftemp->fileName());
}

void UnixCommand::processReadyReadStandardOutput(){
  m_readAllStandardOutput = m_process->readAllStandardOutput();
}

void UnixCommand::processReadyReadStandardError(){
  m_readAllStandardError = m_process->readAllStandardError();
  m_errorString = m_process->errorString();
}

QString UnixCommand::readAllStandardOutput(){
  return m_readAllStandardOutput;
}

QString UnixCommand::readAllStandardError(){
  return m_readAllStandardError;
}

QString UnixCommand::errorString(){
  return m_errorString;
}

UnixCommand::UnixCommand(QObject *parent): QObject(){
  m_process = new QProcess(parent);

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
}

QString UnixCommand::getPkgRemoveCommand(){
  if (SettingsManager::getUsePkgTools() == true)
    return ctn_PKGTOOLS_REMOVE;
  else
    return discoverBinaryPath(ctn_SPKG) + ctn_SPKG_REMOVE;
}

QString UnixCommand::getPkgInstallCommand(){
  if (SettingsManager::getUsePkgTools() == true)
    return ctn_PKGTOOLS_INSTALL;
  else
    return discoverBinaryPath(ctn_SPKG) + ctn_SPKG_INSTALL;
}

QString UnixCommand::getPkgUpgradeCommand(){
  if (SettingsManager::getUsePkgTools() == true)
    return ctn_PKGTOOLS_UPGRADE;
  else
    return discoverBinaryPath(ctn_SPKG) + ctn_SPKG_UPGRADE;
}

QString UnixCommand::getPkgReinstallCommand(){
  if (SettingsManager::getUsePkgTools() == true)
    return ctn_PKGTOOLS_REINSTALL;
  else
    return discoverBinaryPath(ctn_SPKG) + ctn_SPKG_REINSTALL;
}
