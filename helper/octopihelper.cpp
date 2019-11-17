/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2019 Alexandre Albuquerque Arnt
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

#include "../src/constants.h"
#include "octopihelper.h"
#include <QProcess>
#include <QDir>
#include <QObject>
#include <QTextStream>
#include <QSharedMemory>
#include <QRegularExpression>
#include <QStringLiteral>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>

QFile *OctopiHelper::m_temporaryFile = nullptr;

void removeTemporaryFiles()
{
  QDir tempDir(QDir::tempPath());
  QStringList nameFilters;
  nameFilters //<< "qtsingleapp-Octopi*" << "qtsingleapp-CacheC*" << "qtsingleapp-Reposi*"
              //<< "qipc_sharedmemory_orgarntoctopi*"
              //<< "qipc_systemsem_orgarntoctopi*"
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
 * Returns the SHELL environment variable, if not set defaults to bash.
 */
QString getShell()
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
 * If justOneInstance = false (default), returns TRUE if one instance of the app is ALREADY running
 * Otherwise, it returns TRUE if the given app is running.
 */
bool isAppRunning(const QString &appName, bool justOneInstance)
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

OctopiHelper::OctopiHelper()
{  
  m_exitCode = -9999;
  m_process = new QProcess();
  //m_suspiciousChars = QStringLiteral("(\\s|[][!#$&'()*,;<=+>?\\^`{}|~])");
  m_suspiciousChars = QStringLiteral("[!#$&'()*,;<=+>?\\^`{}|~\\[\\]]");

  //This is the setting which enables all outputs from "pacman" go thru QProcess output methods
  m_process->setProcessChannelMode(QProcess::ForwardedChannels);
  m_process->setInputChannelMode(QProcess::ForwardedInputChannel);
}

OctopiHelper::~OctopiHelper()
{
  m_process->close();
  removeTemporaryFiles();
}

/*
 * A bit of settings to better run "pacman" commands using QProcess
 */
QProcessEnvironment OctopiHelper::getProcessEnvironment()
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.remove("LANG");
  env.remove("LC_MESSAGES");
  env.insert("LANG", "C");
  env.insert("LC_MESSAGES", "C");
  env.remove("COLUMNS");
  env.insert("COLUMNS", "132");
  return env;
}

/*
 * Let's find the name of the Octopi's transaction tempfile in "/tmp/.qt_temp_octopi_"
 */
QString OctopiHelper::getTransactionTempFileName()
{
  QString res("");
  QDir dir(QDir::tempPath());
  dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
  dir.setSorting(QDir::Time);
  QStringList nf;
  QFileInfoList list = dir.entryInfoList(nf << ".qt_temp_octopi*");

  /*if (list.count() > 1) //If we find 2 or more files we ABORT!
  {
    return "";
  }*/
  if (list.count()>=1)
  {
    res = list.first().filePath();
  }

  return res;
}

/*
 * Checks if Octopi/Octopi-notifier, cache-cleaner, etc is being executed
 */
bool OctopiHelper::isOctoToolRunning(const QString &octoToolName)
{
  bool res=false;

  QProcess proc;
  proc.setProcessEnvironment(getProcessEnvironment());
  QString cmd = "ps -C %1 -o command";
  cmd = cmd.arg(octoToolName);
  proc.start(cmd);
  proc.waitForFinished();
  QString out = proc.readAll().trimmed();
  if (out.contains("|")) return false;
  out=out.remove("\n");
  out=out.remove("COMMAND");
  if ((out == "/usr/bin/" + octoToolName) || out.contains("/usr/bin/" + octoToolName + " ")) res=true;

  return res;
}

/*
 * Executes all commands inside Octopi's SharedMemory
 * octopi-helper -ts
 */
int OctopiHelper::executePkgTransactionWithSharedMem()
{
  if(!isOctoToolRunning("octopi") &&
     !isOctoToolRunning("octopi-notifier") &&
     !isOctoToolRunning("octopi-cacheclener"))
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Suspicious execution method" << endl;
    return -1;
  }

  QSharedMemory *sharedMem = new QSharedMemory("org.arnt.octopi", this);
  if (!sharedMem->attach(QSharedMemory::ReadOnly))
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Couldn't attach to parent" << endl;
    return -1;
  }

  QByteArray sharedData(sharedMem->size(), '\0');
  sharedMem->lock();
  memcpy(sharedData.data(), sharedMem->data(), sharedMem->size());
  sharedMem->unlock();

  QString contents=QString::fromLatin1(sharedData);
  //QTextStream qout(stdout);
  //qout << endl << "octopi-helper[info]: Reading from memory" << endl;

  sharedMem->detach();
  delete sharedMem;

  bool suspicious = false;

  /*if (contents.contains(";") || contents.contains(",") || contents.contains("|") ||
      contents.contains(">") || contents.contains("<") || contents.contains("&") ||
      contents.contains("'") || contents.contains("'") || contents.contains("`") ||
      contents.contains("^") || contents.contains("~") || contents.contains("@") ||
      contents.contains("#") || contents.contains("$") || contents.contains("%") ||
      contents.contains("*") || contents.contains("?") || contents.contains(":") ||
      contents.contains("!") || contents.contains("+") || contents.contains("=") ||
      contents.contains("(") || contents.contains(")"))*/
  if (contents.isEmpty() || contents.contains(QRegularExpression(m_suspiciousChars)))
      suspicious = true;

  if (suspicious)
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Suspicious transaction detected -> \"" << contents << "\"" << endl;
    return ctn_SUSPICIOUS_ACTIONS_FILE;
  }

  QStringList lines = contents.split("\n", QString::SkipEmptyParts);

  bool testCommandFromOctopi=false;
  bool testCommandFromNotifier=false;
  foreach (QString line, lines)
  {
    line = line.trimmed();

    if ((line == "killall pacman") ||
      (line == "rm " + ctn_PACMAN_DATABASE_LOCK_FILE) ||
      (line == "echo -e") ||
      (line == "echo \"Press any key to continue...\"") ||
      (line == "read -n 1 -p \"Press any key to continue...\"") ||
      (line == "pkgfile -u") ||
      (line == "paccache -r -k 0") ||
      (line == "paccache -r -k 1") ||
      (line == "paccache -r -k 2") ||
      (line == "paccache -r -k 3") ||
      (line == "pacman -Syu") ||
      (line == "pacman -Syu --noconfirm") ||
      (line.startsWith("pacman -U ")) ||
      (line.startsWith("pacman -S ")) ||
      (line.startsWith("pacman -R ")))
    {
      if (line.startsWith("pacman -U ") ||
          line.startsWith("pacman -S ") ||
          line.startsWith("pacman -R "))
      {
        testCommandFromOctopi=true;
        testCommandFromNotifier=false;
      }
      else if (line.startsWith("pacman -Sy"))
      {
        testCommandFromOctopi=true;
        testCommandFromNotifier=true;
      }
    }
    else
      suspicious = true;

    if (suspicious)
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: Suspicious transaction detected -> \"" << line << "\"" << endl;
      return ctn_SUSPICIOUS_ACTIONS_FILE;
    }
  }

  //If there is a "pacman" process executing elsewhere, let's abort octopi-helper!
  if (contents != "killall pacman\nrm " + ctn_PACMAN_DATABASE_LOCK_FILE +"\n" && isAppRunning("pacman", true))
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Pacman process already running" << endl;
    return(ctn_PACMAN_PROCESS_EXECUTING);
  }

  if (testCommandFromOctopi)
  {
    //Let's make a connection to Octopi server to ensure it sent this command.
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 12701);

    if (!socket.waitForConnected(5000))
    {
      if (!testCommandFromNotifier)
      {
        QTextStream qout(stdout);
        qout << endl << "octopi-helper[aborted]: Timeout connecting to Octopi" << endl;
        return -1;
      }
      else goto testNotifierConnection;
    }

    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_5_10);
    QString octopiResponse;

    do
    {
      if (!socket.waitForReadyRead() && !testCommandFromNotifier)
      {
        QTextStream qout(stdout);
        qout << endl << "octopi-helper[aborted]: Timeout contacting Octopi" << endl;
        return -1;
      }

      in.startTransaction();
      in >> octopiResponse;
    } while (!in.commitTransaction());

    if (octopiResponse == "Octopi est occupatus")
      testCommandFromNotifier=false;

    if (octopiResponse != "Octopi est occupatus" && !testCommandFromNotifier)
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: No transaction being executed in Octopi -> " << octopiResponse << endl;
      return -1;
    }
  }

  testNotifierConnection:
  if (testCommandFromNotifier)
  {
    //Let's make a connection to Octopi server to ensure it sent this command.
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 12702);

    if (!socket.waitForConnected(5000))
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: Timeout connecting to Octopi-Notifier" << endl;
      return -1;
    }

    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_5_10);
    QString octopiResponse;

    do
    {
      if (!socket.waitForReadyRead())
      {
        QTextStream qout(stdout);
        qout << endl << "octopi-helper[aborted]: Timeout contacting Octopi-Notifier" << endl;
        return -1;
      }

      in.startTransaction();
      in >> octopiResponse;
    } while (!in.commitTransaction());

    if (octopiResponse != "Octopi est occupatus")
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: No transaction being executed in Octopi-Notifier -> " << octopiResponse << endl;
      return -1;
    }
  }

  QFile *ftemp = generateTemporaryFile();
  QTextStream out(ftemp);
  out << contents;
  out.flush();
  ftemp->close();

  QString command;
  m_process->setProcessEnvironment(getProcessEnvironment());
  command = getShell() + " " + m_temporaryFile->fileName();
  m_process->start(command);
  m_process->waitForStarted(-1);
  m_process->waitForFinished(-1);

  if (m_temporaryFile != nullptr)
    QFile::remove(m_temporaryFile->fileName());

  return m_process->exitCode();
}

/*
 * Execs a tcpServer to test Octopi
 */
int OctopiHelper::dummyServer()
{
  QTcpServer server;
  server.listen(QHostAddress::LocalHost, 12701);

  while(true){}
}

/*
 * Executes all commands inside Octopi's transaction tempfile
 * octopi-helper -t
 */
/*int OctopiHelper::executePkgTransaction()
{
  if(!isOctoToolRunning("octopi") &&
     !isOctoToolRunning("octopi-notifier") &&
     !isOctoToolRunning("octopi-cacheclener"))
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Suspicious execution method" << endl;
    return -1;
  }

  QString tempFile = getTransactionTempFileName();
  if (tempFile.isEmpty()) return ctn_NO_TEMP_ACTIONS_FILE;

  //Let's view the contents of the tempFile...
  QFile f(tempFile);
  if (!f.open(QFile::ReadOnly | QFile::Text)) return 1;

  QTextStream in(&f);
  QString contents = in.readAll();
  f.close();
  bool suspicious = false;

  if (contents.isEmpty() || contents.contains(QRegularExpression(m_suspiciousChars)))
    suspicious = true;

  if (suspicious)
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Suspicious transaction detected -> \"" << contents << "\"" << endl;
    return ctn_SUSPICIOUS_ACTIONS_FILE;
  }

  QStringList lines = contents.split("\n", QString::SkipEmptyParts);

  foreach (QString line, lines){
    line = line.trimmed();

    if ((line == "killall pacman") ||
      (line == "rm " + ctn_PACMAN_DATABASE_LOCK_FILE) ||
      (line == "echo -e") ||
      (line == "echo \"Press any key to continue...\"") ||
      (line == "read -n 1 -p \"Press any key to continue...\"") ||
      (line == "pkgfile -u") ||
      (line == "paccache -r -k 0") ||
      (line == "paccache -r -k 1") ||
      (line == "paccache -r -k 2") ||
      (line == "paccache -r -k 3") ||
      (line == "pacman -Syu") ||
      (line == "pacman -Syu --noconfirm") ||
      (line.startsWith("pacman -U ")) ||
      (line.startsWith("pacman -S ")) ||
      (line.startsWith("pacman -R "))) { }
    else
      suspicious = true;

    if (suspicious)
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: Suspicious transaction detected -> \"" << line << "\"" << endl;
      return ctn_SUSPICIOUS_ACTIONS_FILE;
    }
  }

  //If there is a "pacman" process executing elsewhere, let's abort octopi-helper!
  if (contents != "killall pacman\nrm " + ctn_PACMAN_DATABASE_LOCK_FILE +"\n" && isAppRunning("pacman", true))
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Pacman process already running" << endl;
    return(ctn_PACMAN_PROCESS_EXECUTING);
  }

  QString command;
  m_process->setProcessEnvironment(getProcessEnvironment());
  command = getShell() + " " + getTransactionTempFileName();
  m_process->start(command);
  m_process->waitForStarted(-1);
  m_process->waitForFinished(-1);

  return m_process->exitCode();
}*/
