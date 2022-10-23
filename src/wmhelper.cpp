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
//#include "settingsmanager.h"
//#include "terminal.h"
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
bool WMHelper::isKDERunning()
{
  return (qgetenv("XDG_CURRENT_DESKTOP").toLower() == QByteArray("kde"));
}

/*
 * Checks if TDE is running
 */
bool WMHelper::isTDERunning()
{
  QStringList slParam;
  QProcess proc;
  slParam << QStringLiteral("-aux");
  //slParam << ctn_TDE_DESKTOP;

  proc.start(QStringLiteral("ps"), slParam);
  proc.waitForStarted();
  proc.waitForFinished();

  QString out = QString::fromUtf8(proc.readAll());
  proc.close();

  return out.count(ctn_TDE_DESKTOP)>0;
}

/*
 * Checks if GNOME is running
 */
bool WMHelper::isGNOMERunning()
{
  QStringList slParam;
  QProcess proc;
  slParam << QStringLiteral("-aux");
  //slParam << ctn_GNOME_DESKTOP;

  proc.start(QStringLiteral("ps"), slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = QString::fromUtf8(proc.readAll());
  proc.close();

  return out.count(ctn_GNOME_DESKTOP)>0;
}

/*
 * Checks if XFCE is running
 */
bool WMHelper::isXFCERunning()
{
  QStringList slParam;
  QProcess proc;
  slParam << QStringLiteral("-aux");
  //slParam << ctn_XFCE_DESKTOP;

  proc.start(QStringLiteral("ps"), slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = QString::fromUtf8(proc.readAll());
  proc.close();

  return out.count(ctn_XFCE_DESKTOP)>0;
}

/*
 * Checks if LXDE is running
 */
bool WMHelper::isLXDERunning()
{
  QStringList slParam;
  QProcess proc;
  slParam << QStringLiteral("-aux");
  //slParam << ctn_LXDE_DESKTOP;

  proc.start(QStringLiteral("ps"), slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = QString::fromUtf8(proc.readAll());
  proc.close();

  return out.count(ctn_LXDE_DESKTOP)>0;
}

/*
 * Checks if LXQT is running
 */
bool WMHelper::isLXQTRunning()
{
  return (qgetenv("XDG_CURRENT_DESKTOP").toLower() == QByteArray("lxqt"));
}

/*
 * Checks if OpenBox is running
 */
bool WMHelper::isOPENBOXRunning()
{
  QStringList slParam;
  QProcess proc;
  slParam << QStringLiteral("-aux");
  //slParam << ctn_OPENBOX_DESKTOP;

  proc.start(QStringLiteral("ps"), slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = QString::fromUtf8(proc.readAll());
  proc.close();

  return out.count(ctn_OPENBOX_DESKTOP)>0;
}

/*
 * Checks if MATE is running
 */
bool WMHelper::isMATERunning()
{
  QStringList slParam;
  QProcess proc;
  slParam << QStringLiteral("-aux");
  //slParam << ctn_MATE_DESKTOP;

  proc.start(QStringLiteral("ps"), slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = QString::fromUtf8(proc.readAll());
  proc.close();

  return out.count(ctn_MATE_DESKTOP)>0;
}

/*
 * Checks if Cinnamon is running
 */
bool WMHelper::isCinnamonRunning()
{
  QStringList slParam;
  QProcess proc;
  slParam << QStringLiteral("-aux");
  //slParam << ctn_CINNAMON_DESKTOP;

  proc.start(QStringLiteral("ps"), slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = QString::fromUtf8(proc.readAll());
  proc.close();

  return out.count(ctn_CINNAMON_DESKTOP)>0;
}

/*
 * Checks if Lumina is running
 */
bool WMHelper::isLuminaRunning()
{
  QStringList slParam;
  QProcess proc;
  slParam << QStringLiteral("-aux");
  //slParam << ctn_LUMINA_DESKTOP;

  proc.start(QStringLiteral("ps"), slParam);
  proc.waitForStarted();
  proc.waitForFinished();
  QString out = QString::fromUtf8(proc.readAll());
  proc.close();

  return out.count(ctn_LUMINA_DESKTOP)>0;
}

/*
 * Retrieves the XFCE editor...
 */
QString WMHelper::getXFCEEditor()
{
  if (UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR))
    return ctn_XFCE_EDITOR;
  else
    return ctn_XFCE_EDITOR_ALT;
}

/*
 * Retrieves the OctopiSudo command...
 */
QString WMHelper::getOctopiSudoCommand()
{
  QString result = ctn_OCTOPISUDO;
  result += ctn_OCTOPISUDO_PARAMS;

  return result;
}

/*
 * The generic SU get method. It retrieves the SU you have installed in your system!
 */
QString WMHelper::getSUCommand()
{
  QString result(ctn_NO_SU_COMMAND);

  if (QFile::exists(ctn_OCTOPISUDO)){
    result = ctn_OCTOPISUDO;
  }

  return result;
}

/*
 * The generic SU get method. It retrieves the SU tool name you have installed in your system!
 */
QString WMHelper::getSUTool()
{
  QString result(QLatin1String(""));

  if (QFile::exists(ctn_OCTOPISUDO)){
    return ctn_OCTOPISUDO;
  }

  return result;
}

/*
 * Chooses whether to use kde-open or kde5-open
 */
QString WMHelper::getKDEOpenHelper()
{
  if (UnixCommand::hasTheExecutable(ctn_KDE4_OPEN))
    return ctn_KDE4_OPEN;
  else if (UnixCommand::hasTheExecutable(ctn_KDE5_OPEN))
    return ctn_KDE5_OPEN;
  else
    return QStringLiteral("NONE");
}

/*
 * Opens a file based on your DE
 */
void WMHelper::openFile(const QString& fileName)
{
  const QString& fileToOpen(fileName);
  bool isTextFile = UnixCommand::isTextFile(fileToOpen);

  if (!isTextFile){
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
  else if (isXFCERunning() && UnixCommand::hasTheExecutable(ctn_XFCE_FILE_MANAGER))
  {
    if (!isTextFile)
    {
      s << fileToOpen;
      p->startDetached( ctn_XFCE_FILE_MANAGER, s );
    }
    else
    {
      editFile(fileName, ectn_EDIT_AS_NORMAL_USER);
    }
  }
  else if (isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE_FILE_MANAGER)){
    s << QStringLiteral("exec");
    s << QLatin1String("file:") + fileToOpen;
    p->startDetached( ctn_KDE_FILE_MANAGER, s );
  }
  else if ((isKDERunning()) && UnixCommand::hasTheExecutable(ctn_KDE4_FILE_MANAGER)){
    s << fileToOpen;
    p->startDetached( getKDEOpenHelper(), s );
  }
  else if (isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDE_FILE_MANAGER)){
    s << QStringLiteral("exec");
    s << QLatin1String("file:") + fileToOpen;
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
  else if (isLXQTRunning() && UnixCommand::hasTheExecutable(ctn_LXQT_FILE_MANAGER)){
    s << fileToOpen;
    p->startDetached( ctn_LXQT_FILE_MANAGER, s );
  }
  else if (isLuminaRunning() && UnixCommand::hasTheExecutable(ctn_LUMINA_OPEN)){
    s << fileToOpen;
    p->startDetached( ctn_LUMINA_OPEN, s );
  }
  else if (UnixCommand::hasTheExecutable(ctn_ARCHBANG_FILE_MANAGER)){
    s << fileToOpen;
    p->startDetached( ctn_ARCHBANG_FILE_MANAGER, s );
  }  
  else if (UnixCommand::hasTheExecutable(ctn_LXDE_FILE_MANAGER)){
    s << fileToOpen;
    p->startDetached( ctn_LXDE_FILE_MANAGER, s );
  }
  else if (UnixCommand::hasTheExecutable(ctn_LXQT_FILE_MANAGER)){
    s << fileToOpen;
    p->startDetached( ctn_LXQT_FILE_MANAGER, s );
  }
  else if (UnixCommand::hasTheExecutable(ctn_XFCE_FILE_MANAGER))
  {
    if (!isTextFile)
    {
      s << fileToOpen;
      p->startDetached( ctn_XFCE_FILE_MANAGER, s );
    }
    else
    {
      editFile(fileName, ectn_EDIT_AS_NORMAL_USER);
    }
  }
  else if (UnixCommand::hasTheExecutable(ctn_GNOME_FILE_MANAGER)){
    s << fileToOpen;
    if (isTextFile)
      p->startDetached( ctn_GNOME_EDITOR, s );
    else
      p->startDetached( ctn_GNOME_FILE_MANAGER, s );
  }
  else if (UnixCommand::hasTheExecutable(ctn_XDG_OPEN)){
    s << fileToOpen;
    p->startDetached( ctn_XDG_OPEN, s );
  }
}

/*
 * Edits a file based on your DE.
 */
void WMHelper::editFile(const QString& fileName, EditOptions opt)
{
  QProcess *process = new QProcess(qApp->activeWindow());
  QString p;

  LinuxDistro distro = UnixCommand::getLinuxDistro();
  if (distro == ectn_ARCHBANGLINUX && UnixCommand::hasTheExecutable(ctn_ARCHBANG_EDITOR))
  {
    p = ctn_ARCHBANG_EDITOR + QLatin1Char(' ') + fileName;
  }
  else if (isGNOMERunning() && UnixCommand::hasTheExecutable(ctn_GNOME_EDITOR))
  {
    p = ctn_GNOME_EDITOR + QLatin1Char(' ') + fileName;
  }
  else if (isXFCERunning() && (UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR) ||
                               UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR_ALT))){
    p = getXFCEEditor() + QLatin1Char(' ') + fileName;
  }
  else if (isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE_EDITOR)){
    p += ctn_KDE_EDITOR + QLatin1Char(' ') + fileName;
  }
  else if (isKDERunning() && UnixCommand::hasTheExecutable(ctn_KDE4_EDITOR)){
    p += ctn_KDE4_EDITOR + QLatin1Char(' ') + fileName;
  }
  else if (isTDERunning() && UnixCommand::hasTheExecutable(ctn_TDE_EDITOR)){
    p += ctn_TDE_EDITOR + QLatin1Char(' ') + fileName;
  }
  else if (isMATERunning() && UnixCommand::hasTheExecutable(ctn_MATE_EDITOR)){
    p = ctn_MATE_EDITOR + QLatin1Char(' ') + fileName;
  }
  else if (isLXQTRunning() && UnixCommand::hasTheExecutable(ctn_LXQT_EDITOR)){
    p = ctn_LXQT_EDITOR + QLatin1Char(' ') + fileName;
  }
  else if (isLuminaRunning() && UnixCommand::hasTheExecutable(ctn_LUMINA_EDITOR)){
    p += ctn_LUMINA_EDITOR + QLatin1Char(' ') + fileName;
  }
  if (UnixCommand::hasTheExecutable(ctn_ARCHBANG_EDITOR))
  {
    p = ctn_ARCHBANG_EDITOR + QLatin1Char(' ') + fileName;
  }
  else if (UnixCommand::hasTheExecutable(ctn_CINNAMON_EDITOR)){
    p = ctn_CINNAMON_EDITOR + QLatin1Char(' ') + fileName;
  }
  else if (UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR) || UnixCommand::hasTheExecutable(ctn_XFCE_EDITOR_ALT)){
    p = getXFCEEditor() + QLatin1Char(' ') + fileName;
  }
  else if (UnixCommand::hasTheExecutable(ctn_XDG_OPEN))
  {
    p = ctn_XDG_OPEN + QLatin1Char(' ') + fileName;
  }

  if (opt == ectn_EDIT_AS_NORMAL_USER && !p.isEmpty())
  {
    QStringList params = p.split(QStringLiteral(" "), Qt::SkipEmptyParts);
    QStringList fn;
    process->startDetached(params.at(0), fn << fileName);
  }
  else if (!p.isEmpty())
  {
    QStringList params = p.split(QStringLiteral(" "), Qt::SkipEmptyParts);
    process->startDetached(getSUCommand(), params);
  }
}

/*
 * Opens a directory based on your DE.
 */
void WMHelper::openDirectory(const QString& dirName)
{
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
    else if (isGNOMERunning() && UnixCommand::hasTheExecutable(ctn_GNOME_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_GNOME_FILE_MANAGER, s );
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
        s << QStringLiteral("newTab");
        s << dir;
        p->startDetached( ctn_KDE_FILE_MANAGER, s );
      }
    }
    else if (isTDERunning())
    {
      if (UnixCommand::hasTheExecutable(ctn_TDE_FILE_MANAGER))
      {
        s << QStringLiteral("newTab");
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
    else if (isLXQTRunning() && UnixCommand::hasTheExecutable(ctn_LXQT_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_LXQT_FILE_MANAGER, s );
    }
    else if (isLuminaRunning() && UnixCommand::hasTheExecutable(ctn_LUMINA_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_LUMINA_FILE_MANAGER, s );
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
    else if (UnixCommand::hasTheExecutable(ctn_GNOME_FILE_MANAGER))
    {
      s << dir;
      p->startDetached( ctn_GNOME_FILE_MANAGER, s );
    }
  }
}
