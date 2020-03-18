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

#ifndef ICONHELPER_H
#define ICONHELPER_H

#include "wmhelper.h"
#include "unixcommand.h"

#include <QDir>
#include <QIcon>
#include <QApplication>
#include <QWidget>

/*
 * IconHelper provides some very used icons to the interface
 */

enum AppIcon { ectn_OCTOPI_BUSY, ectn_OCTOPI_RED, ectn_OCTOPI_YELLOW, ectn_OCTOPI_GREEN };

class IconHelper{
public:

  //App icon code
  static QString getOctopiIconPath(AppIcon ic)
  {
    bool useDefaultIcons = SettingsManager::getUseDefaultAppIcon();

    if (!useDefaultIcons)
    {
      //Let's test every choosen icon to see if the file is still there
      QString path;
      path = SettingsManager::getOctopiBusyIconPath();
      if (path.isEmpty() || !QFile::exists(path))
      {
        useDefaultIcons=true;
        goto switch_clause;
      }

      path = SettingsManager::getOctopiRedIconPath();
      if (path.isEmpty() || !QFile::exists(path))
      {
        useDefaultIcons=true;
        goto switch_clause;
      }

      path = SettingsManager::getOctopiYellowIconPath();
      if (path.isEmpty() || !QFile::exists(path))
      {
        useDefaultIcons=true;
        goto switch_clause;
      }

      path = SettingsManager::getOctopiGreenIconPath();
      if (path.isEmpty() || !QFile::exists(path))
      {
        useDefaultIcons=true;
        goto switch_clause;
      }
    }

    switch_clause:
    QString res;
    switch (ic)
    {
      case ectn_OCTOPI_BUSY:
        if (useDefaultIcons)
          res = QStringLiteral(":/resources/images/octopi_transparent.png");
        else
          res = SettingsManager::getOctopiBusyIconPath();
        break;
      case ectn_OCTOPI_RED:
        if (useDefaultIcons)
          res = QStringLiteral(":/resources/images/octopi_red.png");
        else
          res = SettingsManager::getOctopiRedIconPath();
        break;
      case ectn_OCTOPI_YELLOW:
        if (useDefaultIcons)
          res = QStringLiteral(":/resources/images/octopi_yellow.png");
        else
          res = SettingsManager::getOctopiYellowIconPath();
        break;
      case ectn_OCTOPI_GREEN:
        if (useDefaultIcons)
          res = QStringLiteral(":/resources/images/octopi_green.png");
        else
          res = SettingsManager::getOctopiGreenIconPath();
    }

    return res;
  }

  static QIcon getIconOctopi(){
    if (WMHelper::isKDERunning())
      return QIcon::fromTheme("octopi", QIcon());
    else return QIcon();
  }

  static QIcon getIconOctopiBusy(){
    return QIcon(getOctopiIconPath(ectn_OCTOPI_BUSY));
  }

  static QIcon getIconOctopiRed(){ return QIcon(getOctopiIconPath(ectn_OCTOPI_RED)); }
  static QIcon getIconOctopiYellow(){ return QIcon(getOctopiIconPath(ectn_OCTOPI_YELLOW)); }
  static QIcon getIconOctopiGreen(){ return QIcon(getOctopiIconPath(ectn_OCTOPI_GREEN)); }
  //App icon code

  static QIcon getIconFrozen(){ return QIcon(":/resources/images/tgz_frozen_flat.png"); }
  static QIcon getIconUnFrozen(){ return QIcon(":/resources/images/tgz4_flat.png"); }
  static QIcon getIconRPM(){ return QIcon(":/resources/images/rpm.png"); }
  static QIcon getIconInferior(){ return QIcon(":/resources/images/inferiorversion_red.png"); }
  static QIcon getIconSuperior(){ return QIcon(":/resources/images/superiorversion.png"); }
  static QIcon getIconOtherVersion(){ return QIcon(":/resources/images/agent.png"); }
  static QIcon getIconOtherArch(){ return QIcon(":/resources/images/cpu.png"); }
  static QIcon getIconInstalled(){ return QIcon(":/resources/images/installed.png"); }
  static QIcon getIconNonInstalled(){ return QIcon(":/resources/images/noninstalled.png"); }
  static QIcon getIconOutdated(){ return QIcon(":/resources/images/outdated.png"); }
  static QIcon getIconNewer(){ return QIcon(":/resources/images/newer.png"); }
  static QIcon getIconUnrequired(){ return QIcon(":/resources/images/unrequired.png"); }
  static QIcon getIconForeignGreen(){ return QIcon(":/resources/images/foreign_green.png"); }
  static QIcon getIconForeignRed(){ return QIcon(":/resources/images/foreign_red.png"); }
  static QIcon getIconForeignWhite(){ return QIcon(":/resources/images/foreign_white.png"); }
  static QIcon getIconStop(){ return QIcon(":/resources/images/stop_small_red.png"); }
  static QIcon getIconVote(){ return QIcon(":/resources/images/vote.png"); }
  static QIcon getIconUnvote(){ return QIcon(":/resources/images/un_vote.png"); }

  static QIcon getIconBinary(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("application-x-object",QIcon(":/resources/images/binary.png"));
    else
      return QIcon(":/resources/images/binary.png");
  }

  static QIcon getIconToRemove(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("dialog-cancel", QIcon(":/resources/images/toremove.png"));
    else
      return QIcon(":/resources/images/toremove.png");
  }

  static QIcon getIconToInstall(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("download", QIcon(":/resources/images/toinstall.png"));
    else
      return QIcon(":/resources/images/toinstall.png");
  }

  static QIcon getIconTerminal(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("utilities-terminal", QIcon(":/resources/images/terminal.png"));
    else
      return QIcon(":/resources/images/terminal.png");
  }

  static QIcon getIconRemoveItem() {
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("list-remove", QIcon(":/resources/images/toremove.png"));
    else
      return QIcon(":/resources/images/toremove.png");
  }

  static QIcon getIconInstallItem() {
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("list-add", QIcon(":/resources/images/toinstall.png"));
    else
      return QIcon(":/resources/images/toinstall.png");
  }

  static QIcon getIconExit(){    
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("application-exit", QIcon(":/resources/images/exit.png"));
    else      
      return QIcon(":/resources/images/exit.png");
  }

  // Icons for QActions
  static QIcon getIconCheckUpdates(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("view-refresh", QIcon(":/resources/images/refresh.png"));
    else
      return QIcon(":/resources/images/refresh.png");
  }

  static QIcon getIconCommit(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("dialog-ok-apply", QIcon(":/resources/images/commit.png"));
    else
      return QIcon(":/resources/images/commit.png");
  }

  static QIcon getIconRollback(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("edit-undo", QIcon(":/resources/images/rollback.png"));
    else
      return QIcon(":/resources/images/rollback.png");
  }

  static QIcon getIconSystemUpgrade(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("go-up", QIcon(":/resources/images/fast_forward.png"));
    else
      return QIcon(":/resources/images/fast_forward.png");
  }

  static QIcon getIconGetNews(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("application-rss+xml", QIcon(":/resources/images/rss.png"));
    else
      return QIcon(":/resources/images/rss.png");
  }

  static QIcon getIconCollapse(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("zoom-out", QIcon(":/resources/images/collapse.png"));
    else
      return QIcon(":/resources/images/collapse.png");
  }

  static QIcon getIconExpand(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("zoom-in", QIcon(":/resources/images/expand.png"));
    else
      return QIcon(":/resources/images/expand.png");
  }

  static QIcon getIconEditFile(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("document-edit", QIcon(":/resources/images/editfile.png"));
    else
      return QIcon(":/resources/images/editfile.png");
  }

  static QIcon getIconEditCopy(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("edit-copy", QIcon(":/resources/images/edit-copy.png"));
    else
      return QIcon(":/resources/images/edit-copy.png");
  }

  static QIcon getIconFolder(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("document-open-folder", QIcon(":/resources/images/folder.png"));
    else
    {
      if (WMHelper::isKDERunning() || WMHelper::isLuminaRunning() || WMHelper::isLXQTRunning())
      {
        return QIcon(":/resources/images/folder.png");
      }
      else
      {
        return QIcon(":/resources/images/folder_gnome.png");
      }
    }
  }

  static QIcon getIconFindFileInPackage(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("edit-find", QIcon(":/resources/images/find.png"));
    else
      return QIcon(":/resources/images/find.png");
  }

  static QIcon getIconMirrorCheck(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("svn-update", QIcon(":/resources/images/mirror-check.png"));
    else
      return QIcon(":/resources/images/mirror-check.png");
  }

  static QIcon getIconShowGroups(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("view-list-tree", QIcon(":/resources/images/show_groups.png"));
    else
      return QIcon(":/resources/images/show_groups.png");
  }

  static QIcon getIconClose(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("edit-delete", QIcon(":/resources/images/window_close.png"));
    else
      return QIcon(":/resources/images/window_close.png");
  }

  static QIcon getIconSearch(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("edit-find", QIcon(":/resources/images/esf-search.png"));
    else
      return QIcon(":/resources/images/esf-search.png");
  }

  static QIcon getIconClear(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("edit-clear-locationbar-ltr", QIcon(":/resources/images/esf-clear.png"));
    else
      return QIcon(":/resources/images/esf-clear.png");
  }

  static QIcon getIconOptions(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("settings-configure", QIcon());
    else
      return QIcon();
  }

  //QActions without icons in Octopi does this works for gtk, too?
  static QIcon getIconHelpAbout(){
    if (WMHelper::isKDERunning() && (UnixCommand::getLinuxDistro() != ectn_KAOS))
      return QIcon::fromTheme("help-about", QIcon());
    else
      return QIcon();
  }

  static QIcon getIconHelpUsage(){ return QIcon::fromTheme("help-contents"); }
  static QIcon getIconInstallLocalPackage(){ return QIcon::fromTheme("utilities-file-archiver"); }
};

//This is a RAII class used when the GUI is going to face a very CPU intensive action
class CPUIntensiveComputing: public QObject{
private:
  QWidget *m_parent;

public:
  CPUIntensiveComputing(){
    m_parent = 0;
    QApplication::setOverrideCursor(Qt::WaitCursor);
  }

  CPUIntensiveComputing(QWidget *parent){
    if (parent) m_parent = parent;
    if (m_parent) m_parent->setCursor(Qt::WaitCursor);
  }

  virtual ~CPUIntensiveComputing()
  {
    if (m_parent)
      m_parent->setCursor(Qt::ArrowCursor);
    else
      QApplication::restoreOverrideCursor();
  }

  void restoreDefaultCursor(){
    QApplication::restoreOverrideCursor();
  }
};

#endif // ICONHELPER_H
