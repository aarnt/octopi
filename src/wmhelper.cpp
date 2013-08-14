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

#include "wmhelper.h"
#include "unixcommand.h"
#include "strconstants.h"
#include "settingsmanager.h"
#include <iostream>

#include <QApplication>
#include <QProcess>
#include <QMessageBox>

/*
 * This class is a helper to abstract some Desktop Environments services for Octopi.
 * These include: open and edit a file, open a directory and a terminal.
 *
 * There's also a method to retrieve the available tool to obtain root privileges.
 */

/*
 * Checks if KDE is running
 */
bool WMHelper::isKDERunning(){
  QStringList slParam;
  QProcess proc;
  slParam << "-C";
  slParam << ctn_KDE_DESKTOP;

  proc.start("ps", slParam);
  proc.waitForStarted();
  proc.waitForFinished();

  QString out = proc.readAll();
  proc.close();

  if (out.count(ctn_KDE_DESKTOP)>0)
    return true;
  else
    return false;
}

/*
 * Checks if TDE is running
 */
bool WMHelper::isTDERunning(){
  QStringList slParam;
  QProcess proc;
  slParam << "-C";
  slParam << ctn_TDE_DESKTOP;

  proc.start("ps", slParam);
  proc.waitForStarted();
  proc.waitForFinished();

  QString out = proc.readAll();
  proc.close();

  if (out.count(ctn_TDE_DESKTOP)>0)
    return true;
  else
    return false;
}

/*
 * Checks if XFCE is running
 */
bool WMHelper::isXFCERunning(){
  QStringList slParam;
  QProcess proc;
  slParam << "-C";
  slParam << ctn_XFCE_DESKTOP;

  proc.start("ps", slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = proc.readAll();
  proc.close();

  if (out.count(ctn_XFCE_DESKTOP)>0)
    return true;
  else
    return false;
}

/*
 * Checks if LXDE is running
 */
bool WMHelper::isLXDERunning(){
  QStringList slParam;
  QProcess proc;
  slParam << "-C";
  slParam << ctn_LXDE_DESKTOP;

  proc.start("ps", slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = proc.readAll();
  proc.close();

  if (out.count(ctn_LXDE_DESKTOP)>0)
    return true;
  else
      return false;
}

/*
 * Checks if OpenBox is running
 */
bool WMHelper::isOPENBOXRunning(){
  QStringList slParam;
  QProcess proc;
  slParam << "-C";
  slParam << ctn_OPENBOX_DESKTOP;

  proc.start("ps", slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = proc.readAll();
  proc.close();

  if (out.count(ctn_OPENBOX_DESKTOP)>0)
    return true;
  else
      return false;
}

/*
 * Checks if MATE is running
 */
bool WMHelper::isMATERunning(){
  QStringList slParam;
  QProcess proc;
  slParam << "-C";
  slParam << ctn_MATE_DESKTOP;

  proc.start("ps", slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = proc.readAll();
  proc.close();

  if (out.count(ctn_MATE_DESKTOP)>0)
    return true;
  else
    return false;
}

/*
 * Checks if Cinnamon is running
 */
bool WMHelper::isCinnamonRunning(){
  QStringList slParam;
  QProcess proc;
  slParam << "-C";
  slParam << ctn_CINNAMON_DESKTOP;

  proc.start("ps", slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = proc.readAll();
  proc.close();

  if (out.count(ctn_CINNAMON_DESKTOP)>0)
    return true;
  else
    return false;
}

/*
 * Retrieves the XFCE editor...
 */
QString WMHelper::getXFCEEditor(){
  if (UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR))
    return ctn_XFCE_EDITOR;
  else
    return ctn_XFCE_EDITOR_ALT;
}

/*
 * Retrieves the KDESU command...
 */
QString WMHelper::getKDESUCommand(){
  QString result = ctn_KDESU;
  result += " -d ";
  result += " -t ";
  result += " --noignorebutton ";
  result += " -c";

  return result;
}

/*
 * Retrieves the TDESU command...
 */
QString WMHelper::getTDESUCommand(){
  QString result = ctn_TDESU;
  result += " -d ";
  result += " -t ";
  result += " --noignorebutton ";

  return result;
}

/*
 * Retrieves the KTSUSS command...
 */
QString WMHelper::getKTSUSSCommand(){
  QString result = ctn_KTSUSS;

  result += " -m " + QString("\"") + StrConstants::getEnterAdministratorsPassword() + QString("\"");
  result += " -u root ";

  return result;
}

/*
 * Retrieves the GKSU command...
 */
QString WMHelper::getGKSUCommand(){
  QString result;
  result = UnixCommand::discoverBinaryPath(ctn_GKSU_2);
  result += " -u root -m " + QString("\"") + StrConstants::getEnterAdministratorsPassword() + QString("\" ");

  return result;
}

/*
 * The generic SU get method. It retrieves the SU you have installed in your system!
 */
QString WMHelper::getSUCommand(){
  QString result(ctn_NO_SU_COMMAND);

  if (SettingsManager::getPrivilegeEscalationTool() == ctn_AUTOMATIC){
    //User wants the app to automatically choose Privilege escalation tool
    if (isXFCERunning() && (UnixCommand::hasTheExecutable(ctn_GKSU_2))){
      result = getGKSUCommand();
    }
    else if (isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDESU)){
      result = getKDESUCommand();
    }
    else if (isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDESU)){
      result = getTDESUCommand();
    }
    else if (UnixCommand::hasTheExecutable(ctn_GKSU_2)){
      result = getGKSUCommand();
    }
    else if (UnixCommand::isKtsussVersionOK()){
      result = getKTSUSSCommand();
    }
    else if (UnixCommand::hasTheExecutable(ctn_KDESU)){
      result = getKDESUCommand();
    }
    else if (UnixCommand::hasTheExecutable(ctn_TDESU)){
      result = getTDESUCommand();
    }
    //Otherwise, user chose his own privilege escalation tool...
    else if (SettingsManager::getPrivilegeEscalationTool() == ctn_GKSU_2)
      result = getGKSUCommand();
    else if (SettingsManager::getPrivilegeEscalationTool() == ctn_KDESU)
      result = getKDESUCommand();
    else if (SettingsManager::getPrivilegeEscalationTool() == ctn_TDESU)
      result = getTDESUCommand();
    else if (SettingsManager::getPrivilegeEscalationTool() == ctn_KTSUSS){
      result = getKTSUSSCommand();
    }
  }

  return result;
}

/*
 * Opens a file based on your DE
 */
void WMHelper::openFile(const QString& fileName){
  QString fileToOpen(fileName);

  if (!UnixCommand::isTextFile(fileToOpen)){
    int res = QMessageBox::question(qApp->activeWindow(), StrConstants::getConfirmation(),
                                    StrConstants::getThisIsNotATextFile(),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);

    if ( res == QMessageBox::No ) return;
  }

  QProcess *p = new QProcess(qApp->activeWindow());
  QStringList s;

  LinuxDistro distro = UnixCommand::getLinuxDistro();
  if (distro == ectn_ARCHBANGLINUX && UnixCommand::hasTheExecutable(ctn_ARCHBANG_FILE_MANAGER))
  {
    s << fileToOpen;
    p->startDetached( ctn_ARCHBANG_FILE_MANAGER, s );
  }

  else if (isXFCERunning() && UnixCommand::hasTheExecutable(ctn_XFCE_FILE_MANAGER)){
    s << fileToOpen;
    p->startDetached( ctn_XFCE_FILE_MANAGER, s );
  }
  else if (isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE_FILE_MANAGER)){
    s << "exec";
    s << "file:" + fileToOpen;
    p->startDetached( ctn_KDE_FILE_MANAGER, s );
  }
  else if (isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDE_FILE_MANAGER)){
    s << "exec";
    s << "file:" + fileToOpen;
    p->startDetached( ctn_TDE_FILE_MANAGER, s );
  }
  else if (isMATERunning() && UnixCommand::hasTheExecutable(ctn_MATE_EDITOR)){
    s << fileToOpen;
    p->startDetached( ctn_MATE_EDITOR, s );
  }
  else if (isCinnamonRunning() && UnixCommand::hasTheExecutable(ctn_CINNAMON_EDITOR)){
    s << fileToOpen;
    p->startDetached( ctn_CINNAMON_EDITOR, s );
  }
  else if (UnixCommand::hasTheExecutable(ctn_XFCE_FILE_MANAGER)){
    s << fileToOpen;
    p->startDetached( ctn_XFCE_FILE_MANAGER, s );
  }
  else if (UnixCommand::hasTheExecutable(ctn_LXDE_FILE_MANAGER)){
    s << fileToOpen;
    p->startDetached( ctn_LXDE_FILE_MANAGER, s );
  }
}

/*
 * Edits a file based on your DE.
 */
void WMHelper::editFile( const QString& fileName ){
  QProcess *process = new QProcess(qApp->activeWindow());
  QStringList s;

  if (!UnixCommand::isRootRunning()){
    LinuxDistro distro = UnixCommand::getLinuxDistro();
    if (distro == ectn_ARCHBANGLINUX && UnixCommand::hasTheExecutable(ctn_ARCHBANG_EDITOR))
    {
      QString p = ctn_ARCHBANG_EDITOR + " " + fileName;
      process->startDetached(getSUCommand() + p);
    }
    else if (isXFCERunning() && (UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR) ||
                             UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR_ALT))){

      QString p = getXFCEEditor() + " " + fileName;
      process->startDetached(getSUCommand() + p);
    }
    else if (isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE_EDITOR)){
      QString p = " -d -t --noignorebutton -c ";
      p += ctn_KDE_EDITOR + " " + fileName;
      process->startDetached(getSUCommand() + p);
    }
    else if (isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDE_EDITOR)){
      QString p = " -d -t --noignorebutton ";
      p += ctn_TDE_EDITOR + " " + fileName;
      process->startDetached(getSUCommand() + p);
    }
    else if (isMATERunning() && UnixCommand::hasTheExecutable(ctn_MATE_EDITOR)){
      QString p = ctn_MATE_EDITOR + " " + fileName;
      process->startDetached(getSUCommand() + p);
    }
    else if (isCinnamonRunning() && UnixCommand::hasTheExecutable(ctn_CINNAMON_EDITOR)){
      QString p = ctn_CINNAMON_EDITOR + " " + fileName;
      process->startDetached(getSUCommand() + p);
    }
    else if (UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR) || UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR_ALT)){
      QString p = getXFCEEditor() + " " + fileName;
      process->startDetached(getSUCommand() + p);
    }
  }
  //Octopi was started by root account.
  else{
    if (UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR) || UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR_ALT))
      s << getXFCEEditor() + " " + fileName;
    else if (UnixCommand::hasTheExecutable(ctn_KDE_EDITOR))
      s << ctn_KDE_EDITOR + " " + fileName;
    else if (UnixCommand::hasTheExecutable(ctn_TDE_EDITOR))
      s << ctn_TDE_EDITOR + " " + fileName;

    process->startDetached("/bin/sh", s);
  }
}

/*
 * Opens a directory based on your DE.
 */
void WMHelper::openDirectory( const QString& dirName ){
  QProcess *p = new QProcess(qApp->activeWindow());
  QStringList s;
  QString dir(dirName);

  //Is it really a directory?
  QFileInfo f(dirName);
  if (!f.isDir())
  {
    dir = f.absolutePath();
    f = QFileInfo(dir);
  }

  if (f.exists())
  {
    LinuxDistro distro = UnixCommand::getLinuxDistro();
    if (distro == ectn_ARCHBANGLINUX && UnixCommand::hasTheExecutable(ctn_ARCHBANG_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_ARCHBANG_FILE_MANAGER, s );
    }
    else if(isXFCERunning() && UnixCommand::hasTheExecutable(ctn_XFCE_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_XFCE_FILE_MANAGER, s );
    }
    else if (isKDERunning())
    {
      if (UnixCommand::hasTheExecutable(ctn_KDE4_FILE_MANAGER))
      {
        s << dir;
        p->startDetached( ctn_KDE4_FILE_MANAGER, s);
      }
      else if (UnixCommand::hasTheExecutable(ctn_KDE_FILE_MANAGER))
      {
        s << "newTab";
        s << dir;
        p->startDetached( ctn_KDE_FILE_MANAGER, s );
      }
    }
    else if (isTDERunning())
    {
      if (UnixCommand::hasTheExecutable(ctn_TDE_FILE_MANAGER))
      {
        s << "newTab";
        s << dir;
        p->startDetached( ctn_TDE_FILE_MANAGER, s );
      }
    }
    else if (isMATERunning() && UnixCommand::hasTheExecutable(ctn_MATE_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_MATE_FILE_MANAGER, s );
    }
    else if (isCinnamonRunning() && UnixCommand::hasTheExecutable(ctn_CINNAMON_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_CINNAMON_FILE_MANAGER, s );
    }
    else if (UnixCommand::hasTheExecutable(ctn_XFCE_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_XFCE_FILE_MANAGER, s );
    }
    else if (UnixCommand::hasTheExecutable(ctn_LXDE_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_LXDE_FILE_MANAGER, s );
    }
  }
}

/*
 * Opens a terminal based on your DE.
 */
void WMHelper::openTerminal(const QString& dirName){
  QProcess *p = new QProcess(qApp->activeWindow());
  QStringList s;
  QFileInfo f(dirName);

  if (f.exists()){
    if(isXFCERunning() && UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
      s << "--working-directory=" + dirName;
      p->startDetached( ctn_XFCE_TERMINAL, s );
    }
    else if (isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE_TERMINAL)){
      s << "--workdir";
      s << dirName;
      p->startDetached( ctn_KDE_TERMINAL, s );
    }
    else if (isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDE_TERMINAL)){
      s << "--workdir";
      s << dirName;
      p->startDetached( ctn_TDE_TERMINAL, s );
    }
    else if (isLXDERunning() && UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
      s << "--working-directory=" + dirName;
      p->startDetached( ctn_LXDE_TERMINAL, s );
    }
    else if (isMATERunning() && UnixCommand::hasTheExecutable(ctn_MATE_TERMINAL)){
      s << "--working-directory=" + dirName;
      p->startDetached( ctn_MATE_TERMINAL, s );
    }
    else if (isCinnamonRunning() && UnixCommand::hasTheExecutable(ctn_CINNAMON_TERMINAL)){
      s << "--working-directory=" + dirName;
      p->startDetached( ctn_CINNAMON_TERMINAL, s );
    }
    else if (UnixCommand::hasTheExecutable(ctn_XFCE_TERMINAL)){
      s << "--working-directory=" + dirName;
      p->startDetached( ctn_XFCE_TERMINAL, s );
    }
    else if (UnixCommand::hasTheExecutable(ctn_LXDE_TERMINAL)){
      s << "--working-directory=" + dirName;
      p->startDetached( ctn_LXDE_TERMINAL, s );
    }
  }
}
