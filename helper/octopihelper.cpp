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
#include <QSettings>
#include <QDateTime>

QFile *OctopiHelper::m_temporaryFile = nullptr;

/*
 * Removes temporary transaction files
 */
void removeTemporaryFiles()
{
  QDir tempDir(QDir::tempPath());
  QStringList nameFilters;
  nameFilters << QStringLiteral(".qt_temp_octopi*");
  const QFileInfoList list = tempDir.entryInfoList(nameFilters, QDir::Dirs | QDir::Files | QDir::System | QDir::Hidden);

  for(const auto &file : list){
    QFile fileAux(file.filePath());

    if (!file.isDir()){
      fileAux.remove();
    }
    else{
      QDir dir(file.filePath());
      const QFileInfoList listd = dir.entryInfoList(QDir::Files | QDir::System);

      for(const auto &filed : listd){
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
  QString shell = env.value(QStringLiteral("SHELL"), QStringLiteral("/usr/bin/bash"));
  QFileInfo fi(shell);

  if (fi.fileName() == QLatin1String("fish"))
    return QStringLiteral("/usr/bin/bash");
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
  slParam << QStringLiteral("-C");
  slParam << appName;
  proc.start(QStringLiteral("/usr/bin/ps"), slParam);
  proc.waitForFinished();
  QString out = QString::fromUtf8(proc.readAll());
  proc.close();

  if (justOneInstance)
  {
    return out.count(appName)>0;
  }
  else
  {
    return out.count(appName)>1;
  }
}

OctopiHelper::OctopiHelper()
{  
  //If old helper still exists, let's remove it
  if (QFileInfo::exists(QStringLiteral("/usr/lib/octopi/octopi-helper")))
    QFile::remove(QStringLiteral("/usr/lib/octopi/octopi-helper"));

  m_exitCode = -9999;
  m_process = new QProcess();
  //m_suspiciousChars = QStringLiteral("(\\s|[][!#$&'()*,;<=+>?\\^`{}|~])");
  m_suspiciousChars = QStringLiteral("[!#$&'()*,;<=>?\\^`{}|~\\[\\]]");

  //These settings enable all "pacman" output go thru QProcess output methods
  m_process->setProcessChannelMode(QProcess::ForwardedChannels);
  m_process->setInputChannelMode(QProcess::ForwardedInputChannel);

  QString fname = QStringLiteral("/usr/lib/octopi/octphelper.log");
  m_logFile.setFileName(fname);
  m_logFile.open(QIODevice::WriteOnly | /*QIODevice::Append |*/ QIODevice::Text);
}

OctopiHelper::~OctopiHelper()
{
  m_process->close();
  if (m_temporaryFile != nullptr)
    QFile::remove(m_temporaryFile->fileName());
  removeTemporaryFiles();

  if (m_logFile.isOpen())
    m_logFile.close();
}

/*
 * Logs received str in a file called "octphelper.log" (for debugging purposes)
 */
void OctopiHelper::log(const QString &str)
{
  QString dateTimeFormat = QLocale().dateTimeFormat(QLocale::ShortFormat);
  QDateTime bdt = QDateTime::currentDateTime();

  QTextStream out(&m_logFile);
  out << bdt.toString(dateTimeFormat) << QLatin1String(": ") << str << Qt::endl;

  m_logFile.flush();
}

/*
 * A bit of settings to better run "pacman" commands using QProcess
 */
QProcessEnvironment OctopiHelper::getProcessEnvironment()
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
 * Retrieves Proxy_Settings config from "/root/.config/octopi/octopi.conf"
 */
QString OctopiHelper::getProxySettings()
{
  QSettings settings(QSettings::UserScope, ctn_ORGANIZATION, ctn_APPLICATION);
  return (settings.value(ctn_KEY_PROXY_SETTINGS, QLatin1String("")).toString());
}

/*
 * Checks if Octopi/Octopi-notifier, cache-cleaner, etc is being executed
 */
bool OctopiHelper::isOctoToolRunning(const QString &octoToolName)
{
  bool res=false;

  QProcess proc;
  proc.setProcessEnvironment(getProcessEnvironment());
  QStringList sl;
  sl << QStringLiteral("-C");
  sl << octoToolName;
  sl << QStringLiteral("-o");
  sl << QStringLiteral("command");
  proc.start(QStringLiteral("/usr/bin/ps"), sl);
  proc.waitForFinished();

  QString out = QString::fromUtf8(proc.readAll().trimmed());
  if (out.contains(QLatin1String("|"))) return false;
  out=out.remove(QStringLiteral("\n"));
  out=out.remove(QStringLiteral("COMMAND"));

  if (octoToolName==QLatin1String("octopi-cachecle"))
  {
    if (out == QLatin1String("/usr/bin/octopi-cachecleaner")) res=true;
  }
  else
  {
    QStringList options;
    options << QStringLiteral("/usr/bin/octopi-notifier -d");
    options << QStringLiteral("/usr/bin/octopi -d");
    options << QStringLiteral("/usr/bin/octopi -sysupgrade");
    options << QStringLiteral("/usr/bin/octopi -sysupgrade-noconfirm");

    //QRegularExpression re(QStringLiteral("(/usr/bin/octopi-notifier -session )[a-fA-F0-9_]+"));
    QRegularExpression re(QStringLiteral("(/usr/bin/octopi-notifier.*)"));
    QRegularExpressionMatch match = re.match(out);
    bool hasMatchInSession = match.capturedLength()==out.length();

    re=QRegularExpression(QStringLiteral("(/usr/bin/octopi -style )\\S+"));
    match = re.match(out);
    bool hasMatchInStyle = match.capturedLength()==out.length();

    if (out == QLatin1String("/usr/bin/") + octoToolName ||
        (options.indexOf(out)!=-1) ||
        (hasMatchInSession) ||
        (hasMatchInStyle)) res=true;
  }
  return res;
}

/*
 * Executes all commands inside Octopi's SharedMemory - "octopi-helper -ts"
 */
int OctopiHelper::executePkgTransactionWithSharedMem()
{
  bool isOctopiRunning=isOctoToolRunning(QStringLiteral("octopi"));
  bool isNotifierRunning=isOctoToolRunning(QStringLiteral("octopi-notifier"));
  bool isCacheCleanerRunning=isOctoToolRunning(QStringLiteral("octopi-cachecle"));

  if (!isOctopiRunning && !isNotifierRunning && !isCacheCleanerRunning)
  {
    log(QLatin1String("octopi-helper[aborted]: Suspicious execution method - NO [/usr/bin/octopi-cachecleaner] OR [/usr/bin/octopi-notifier] OR [/usr/bin/octopi] is running..."));
    return ctn_SUSPICIOUS_EXECUTION_METHOD;
  }

  //Let's retrieve commands from sharedmem pool
  QSharedMemory *sharedMem = new QSharedMemory(QStringLiteral("org.arnt.octopi"), this);
  if (!sharedMem->attach(QSharedMemory::ReadOnly))
  {
    log(QLatin1String("octopi-helper[aborted]: Couldn't attach to memory"));
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

  if (contents.isEmpty() || contents.contains(QRegularExpression(m_suspiciousChars)))
    suspicious = true;

  if (suspicious)
  {
    log(QLatin1String("octopi-helper[aborted]: Suspicious transaction detected -> \"") + contents + QLatin1String("\""));
    return ctn_SUSPICIOUS_ACTIONS_FILE;
  }

  const QStringList lines = contents.split(QStringLiteral("\n"), Qt::SkipEmptyParts);
  bool testCommandFromOctopi=false;
  bool testCommandFromNotifier=false;
  bool testCommandFromCacheCleaner=false;

  for(auto line : lines)
  {
    line = line.trimmed();

    if ((line == QLatin1String("killall pacman")) ||
      (line == QLatin1String("rm ") + ctn_PACMAN_DATABASE_LOCK_FILE) ||
      (line == QLatin1String("echo -e")) ||
      (line == QLatin1String("echo \"PAKtC\"")) ||
      (line == QLatin1String("read -n 1 -p \"PAKtC\"")) ||
      (line == QLatin1String("pkgfile -u")) ||
      (line == QLatin1String("paccache -r -k 0")) ||
      (line == QLatin1String("paccache -r -k 1")) ||
      (line == QLatin1String("paccache -r -k 2")) ||
      (line == QLatin1String("paccache -r -k 3")) ||
      (line == QLatin1String("pacman -Fy")) ||
      (line == QLatin1String("pacman -Syu")) ||
      (line == QLatin1String("pacman -Syu --noconfirm")) ||
      (line.startsWith(QLatin1String("pacman -D --asexplicit "))) ||
      (line.startsWith(QLatin1String("pacman -D --asdeps "))) ||
      (line.startsWith(QLatin1String("pacman -S "))) ||
      (line.startsWith(QLatin1String("pacman -R "))))
    {
      if (line.startsWith(QLatin1String("pacman -D --asexplicit ")) ||
          line.startsWith(QLatin1String("pacman -D --asdeps ")) ||
          line.startsWith(QLatin1String("pacman -S ")) ||
          line.startsWith(QLatin1String("pacman -R ")))
      {
        testCommandFromOctopi=true;
      }
      else if (line.startsWith(QLatin1String("pacman -Syu")))
      {
        testCommandFromOctopi=true;
        testCommandFromNotifier=true;
      }
      else if (line.startsWith(QLatin1String("paccache -r -k")))
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
      log(QLatin1String("octopi-helper[aborted]: Suspicious transaction detected -> \"") + line + QLatin1String("\""));
      return ctn_SUSPICIOUS_ACTIONS_FILE;
    }
  }

  //Using full path binaries
  contents = contents.replace(QLatin1String("killall pacman"), QLatin1String("/usr/bin/killall pacman"));
  contents = contents.replace(QLatin1String("rm ") + ctn_PACMAN_DATABASE_LOCK_FILE, QLatin1String("/usr/bin/rm ") + ctn_PACMAN_DATABASE_LOCK_FILE);
  contents = contents.replace(QLatin1String("pkgfile -u"), QLatin1String("/usr/bin/pkgfile -u"));
  contents = contents.replace(QLatin1String("paccache -r"), QLatin1String("/usr/bin/paccache -r"));
  contents = contents.replace(QLatin1String("pacman -Fy"), QLatin1String("/usr/bin/pacman -Fy"));
  contents = contents.replace(QLatin1String("pacman -Syu"), QLatin1String("/usr/bin/pacman -Syu"));
  contents = contents.replace(QLatin1String("pacman -D "), QLatin1String("/usr/bin/pacman -D "));
  contents = contents.replace(QLatin1String("pacman -S "), QLatin1String("/usr/bin/pacman -S "));
  contents = contents.replace(QLatin1String("pacman -R "), QLatin1String("/usr/bin/pacman -R "));

  //If there is a "pacman" process executing elsewhere, let's abort octopi-helper!
  if (contents != QLatin1String("/usr/bin/killall pacman\n/usr/bin/rm ") + ctn_PACMAN_DATABASE_LOCK_FILE +QLatin1Char('\n') && isAppRunning(QStringLiteral("pacman"), true))
  {
    log(QLatin1String("octopi-helper[aborted]: Pacman process already running"));
    return ctn_PACMAN_PROCESS_EXECUTING;
  }

  if (testCommandFromOctopi)
  {
    if (!isOctopiRunning && !testCommandFromNotifier)
    {
      log(QLatin1String("octopi-helper[aborted]: Suspicious execution method -> Octopi not running"));
      return ctn_SUSPICIOUS_EXECUTION_METHOD;
    }

    //Let's make a connection to Octopi server to ensure it sent this command.
    QTcpSocket socket;
    socket.connectToHost(QStringLiteral("127.0.0.1"), 12701);

    if (!socket.waitForConnected(5000))
    {
      if (!testCommandFromNotifier)
      {
        log(QLatin1String("octopi-helper[aborted]: Timeout connecting to Octopi"));
        return ctn_TIMEOUT_CONNECTING;
      }
      else goto testNotifierConnection;
    }

    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_5_15);
    QString octopiResponse;

    do
    {
      if (!socket.waitForReadyRead() && !testCommandFromNotifier)
      {
        log(QLatin1String("octopi-helper[aborted]: Timeout contacting Octopi"));
        return ctn_TIMEOUT_CONNECTING;
      }

      in.startTransaction();
      in >> octopiResponse;
    } while (!in.commitTransaction());

    if (octopiResponse == QLatin1String("Octopi est occupatus"))
    {
      testCommandFromNotifier=false;
    }
    else if (octopiResponse != QLatin1String("Octopi est occupatus") && !testCommandFromNotifier)
    {
      log(QLatin1String("octopi-helper[aborted]: No transaction being executed"));
      return ctn_NO_TRANSACTION_EXECUTING;
    }
  }

  testNotifierConnection:
  if (testCommandFromNotifier)
  {
    if (!isNotifierRunning)
    {
      log(QLatin1String("octopi-helper[aborted]: Suspicious execution method -> Notifier is not running"));
      return ctn_SUSPICIOUS_EXECUTION_METHOD;
    }

    //Let's make a connection to Octopi-Notifier server to ensure it sent this command.
    QTcpSocket socket;
    socket.connectToHost(QStringLiteral("127.0.0.1"), 12702);

    if (!socket.waitForConnected(5000))
    {
      log(QLatin1String("octopi-helper[aborted]: Timeout connecting to Octopi-Notifier"));
      return ctn_TIMEOUT_CONNECTING;
    }

    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_5_15);
    QString octopiResponse;

    do
    {
      if (!socket.waitForReadyRead())
      {
        log(QLatin1String("octopi-helper[aborted]: Timeout contacting Octopi-Notifier"));
        return ctn_TIMEOUT_CONNECTING;
      }

      in.startTransaction();
      in >> octopiResponse;
    } while (!in.commitTransaction());

    if (octopiResponse != QLatin1String("Octopi est occupatus"))
    {
      log(QLatin1String("octopi-helper[aborted]: No transaction being executed"));
      return ctn_NO_TRANSACTION_EXECUTING;
    }
  }

  if (testCommandFromCacheCleaner)
  {
    if (!isCacheCleanerRunning)
    {
      log(QLatin1String("octopi-helper[aborted]: Suspicious execution method -> CacheCleaner is not running"));
      return ctn_SUSPICIOUS_EXECUTION_METHOD;
    }

    //Let's make a connection to Octopi-Notifier server to ensure it sent this command.
    QTcpSocket socket;
    socket.connectToHost(QStringLiteral("127.0.0.1"), 12703);

    if (!socket.waitForConnected(5000))
    {
      log(QLatin1String("octopi-helper[aborted]: Timeout connecting to Octopi-CacheCleaner"));
      return ctn_TIMEOUT_CONNECTING;
    }

    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_5_15);
    QString octopiResponse;

    do
    {
      if (!socket.waitForReadyRead())
      {
        log(QLatin1String("octopi-helper[aborted]: Timeout contacting Octopi-CacheCleaner"));
        return ctn_TIMEOUT_CONNECTING;
      }

      in.startTransaction();
      in >> octopiResponse;
    } while (!in.commitTransaction());

    if (octopiResponse != QLatin1String("Octopi est occupatus"))
    {
      log(QLatin1String("octopi-helper[aborted]: No transaction being executed"));
      return ctn_NO_TRANSACTION_EXECUTING;
    }
  }

  //Let's construct the root owned execution file
  QFile *ftemp = generateTemporaryFile();
  QTextStream out(ftemp);

  QString proxySettings = getProxySettings();
  if (!proxySettings.isEmpty())
  {
    if (proxySettings.contains(QLatin1String("ftp://")))
      out << QLatin1String("export ftp_proxy=") + proxySettings + QLatin1Char('\n');
    else if (proxySettings.contains(QLatin1String("http://")))
      out << QLatin1String("export http_proxy=") + proxySettings + QLatin1Char('\n');
    else if (proxySettings.contains(QLatin1String("https://")))
      out << QLatin1String("export https_proxy=") + proxySettings + QLatin1Char('\n');
  }

  out << QLatin1String("unalias -a\n") << contents;

  log(QLatin1String("Exec as root: ") + contents.trimmed());

  out.flush();
  ftemp->close();

  m_process->setProcessEnvironment(getProcessEnvironment());
  m_process->start(getShell(), QStringList() << m_temporaryFile->fileName());
  m_process->waitForStarted(-1);
  m_process->waitForFinished(-1);

  return m_process->exitCode();
}
