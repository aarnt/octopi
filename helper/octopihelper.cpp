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
#include <QTcpSocket>
#include <QDataStream>
#include <QFile>

QFile *OctopiHelper::m_temporaryFile = nullptr;

/*
 * Removes temporary transaction files
 */
void removeTemporaryFiles()
{
  QDir tempDir(QDir::tempPath());
  QStringList nameFilters;
  nameFilters << ".qt_temp_octopi*";
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
  QString shell = env.value("SHELL", "/usr/bin/bash");
  QFileInfo fi(shell);

  if (fi.fileName() == "fish")
    return "/usr/bin/bash";
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
  proc.start("/usr/bin/ps", slParam);
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
  //If old helper still exists, let's remove it
  if (QFileInfo::exists("/usr/lib/octopi/octopi-helper"))
    QFile::remove("/usr/lib/octopi/octopi-helper");

  m_exitCode = -9999;
  m_process = new QProcess();
  //m_suspiciousChars = QStringLiteral("(\\s|[][!#$&'()*,;<=+>?\\^`{}|~])");
  m_suspiciousChars = QStringLiteral("[!#$&'()*,;<=+>?\\^`{}|~\\[\\]]");

  //These settings enable all "pacman" output go thru QProcess output methods
  m_process->setProcessChannelMode(QProcess::ForwardedChannels);
  m_process->setInputChannelMode(QProcess::ForwardedInputChannel);
}

OctopiHelper::~OctopiHelper()
{
  m_process->close();
  if (m_temporaryFile != nullptr)
    QFile::remove(m_temporaryFile->fileName());
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
 * Checks if Octopi/Octopi-notifier, cache-cleaner, etc is being executed
 */
bool OctopiHelper::isOctoToolRunning(const QString &octoToolName)
{
  bool res=false;

  QProcess proc;
  proc.setProcessEnvironment(getProcessEnvironment());
  QString cmd = "/usr/bin/ps -C %1 -o command";
  cmd = cmd.arg(octoToolName);
  proc.start(cmd);
  proc.waitForFinished();
  QString out = proc.readAll().trimmed();
  if (out.contains("|")) return false;
  out=out.remove("\n");
  out=out.remove("COMMAND");

  if (octoToolName=="octopi-cachecle")
  {
    if (out == "/usr/bin/octopi-cachecleaner") res=true;
  }
  else if ((out == "/usr/bin/" + octoToolName) || out.contains("/usr/bin/" + octoToolName + " ")) res=true;

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
     !isOctoToolRunning("octopi-cachecle"))
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Suspicious execution method" << endl;
    return ctn_SUSPICIOUS_EXECUTION_METHOD;
  }

  //Let's retrieve commands from sharedmem pool
  QSharedMemory *sharedMem = new QSharedMemory("org.arnt.octopi", this);
  if (!sharedMem->attach(QSharedMemory::ReadOnly))
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Couldn't attach to memory" << endl;
    return ctn_COULD_NOT_ATTACH_TO_MEM;
  }

  QByteArray sharedData(sharedMem->size(), '\0');
  sharedMem->lock();
  memcpy(sharedData.data(), sharedMem->data(), sharedMem->size());
  sharedMem->unlock();
  QString contents=QString::fromLatin1(sharedData);
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
  bool testCommandFromCacheCleaner=false;

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
      (line.startsWith("pacman -S ")) ||
      (line.startsWith("pacman -R ")))
    {
      if (line.startsWith("pacman -S ") ||
          line.startsWith("pacman -R "))
      {
        testCommandFromOctopi=true;
      }
      else if (line.startsWith("pacman -Syu") /*||
               line == "killall pacman" ||
               line == "rm " + ctn_PACMAN_DATABASE_LOCK_FILE)*/)
      {
        testCommandFromOctopi=true;
        testCommandFromNotifier=true;
      }
      else if (line.startsWith("paccache -r -k"))
      {
        testCommandFromCacheCleaner=true;
      }
    }
    else
    {
      suspicious = true;
    }

    if (suspicious)
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: Suspicious transaction detected -> \"" << line << "\"" << endl;
      return ctn_SUSPICIOUS_ACTIONS_FILE;
    }
  }

  //Using full path binaries
  contents = contents.replace("killall pacman", "/usr/bin/killall pacman");
  contents = contents.replace("rm " + ctn_PACMAN_DATABASE_LOCK_FILE, "/usr/bin/rm " + ctn_PACMAN_DATABASE_LOCK_FILE);
  contents = contents.replace("pkgfile -u", "/usr/bin/pkgfile -u");
  contents = contents.replace("paccache -r", "/usr/bin/paccache -r");
  contents = contents.replace("pacman -Syu", "/usr/bin/pacman -Syu");
  contents = contents.replace("pacman -S ", "/usr/bin/pacman -S ");
  contents = contents.replace("pacman -R ", "/usr/bin/pacman -R ");

  //If there is a "pacman" process executing elsewhere, let's abort octopi-helper!
  if (contents != "/usr/bin/killall pacman\n/usr/bin/rm " + ctn_PACMAN_DATABASE_LOCK_FILE +"\n" && isAppRunning("pacman", true))
  {
    QTextStream qout(stdout);
    qout << endl << "octopi-helper[aborted]: Pacman process already running" << endl;
    return ctn_PACMAN_PROCESS_EXECUTING;
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
        return ctn_TIMEOUT_CONNECTING;
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
        return ctn_TIMEOUT_CONNECTING;
      }

      in.startTransaction();
      in >> octopiResponse;
    } while (!in.commitTransaction());

    if (octopiResponse == "Octopi est occupatus")
    {
      testCommandFromNotifier=false;
    }
    else if (octopiResponse != "Octopi est occupatus" && !testCommandFromNotifier)
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: No transaction being executed" << endl;
      return ctn_NO_TRANSACTION_EXECUTING;
    }
  }

  testNotifierConnection:
  if (testCommandFromNotifier)
  {
    //Let's make a connection to Octopi-Notifier server to ensure it sent this command.
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 12702);

    if (!socket.waitForConnected(5000))
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: Timeout connecting to Octopi-Notifier" << endl;
      return ctn_TIMEOUT_CONNECTING;
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
        return ctn_TIMEOUT_CONNECTING;
      }

      in.startTransaction();
      in >> octopiResponse;
    } while (!in.commitTransaction());

    if (octopiResponse != "Octopi est occupatus")
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: No transaction being executed" << endl;
      return ctn_NO_TRANSACTION_EXECUTING;
    }
  }

  if (testCommandFromCacheCleaner)
  {
    //Let's make a connection to Octopi-Notifier server to ensure it sent this command.
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 12703);

    if (!socket.waitForConnected(5000))
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: Timeout connecting to Octopi-CacheCleaner" << endl;
      return ctn_TIMEOUT_CONNECTING;
    }

    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_5_10);
    QString octopiResponse;

    do
    {
      if (!socket.waitForReadyRead())
      {
        QTextStream qout(stdout);
        qout << endl << "octopi-helper[aborted]: Timeout contacting Octopi-CacheCleaner" << endl;
        return ctn_TIMEOUT_CONNECTING;
      }

      in.startTransaction();
      in >> octopiResponse;
    } while (!in.commitTransaction());

    if (octopiResponse != "Octopi est occupatus")
    {
      QTextStream qout(stdout);
      qout << endl << "octopi-helper[aborted]: No transaction being executed" << endl;
      return ctn_NO_TRANSACTION_EXECUTING;
    }
  }

  //Let's construct the root owned execution file
  QFile *ftemp = generateTemporaryFile();
  QTextStream out(ftemp);
  out << "unalias -a\n" << contents;
  out.flush();
  ftemp->close();

  QString command;
  m_process->setProcessEnvironment(getProcessEnvironment());
  command = getShell() + " " + m_temporaryFile->fileName();
  m_process->start(command);
  m_process->waitForStarted(-1);
  m_process->waitForFinished(-1);

  return m_process->exitCode();
}
