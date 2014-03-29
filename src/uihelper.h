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

#include <QDir>
#include <QIcon>
#include <QApplication>
#include <QWidget>

//IconHelper provides some very used icons to the interface
class IconHelper{
public:
  static QIcon getIconOctopiTransparent(){ return QIcon(":/resources/images/octopi_transparent.png"); }
  static QIcon getIconOctopiRed(){ return QIcon(":/resources/images/octopi_red.png"); }
  static QIcon getIconOctopiYellow(){ return QIcon(":/resources/images/octopi_yellow.png"); }
  static QIcon getIconOctopiGreen(){ return QIcon(":/resources/images/octopi_green.png"); }

  static QIcon getIconFolder()
  {
    if (WMHelper::isKDERunning())
      return QIcon::fromTheme("folder");
    else
      return QIcon(":/resources/images/folder_gnome.png");
  }

  static QIcon getIconBinary(){ return QIcon::fromTheme("application-x-object",QIcon(":/resources/images/binary.png")); }
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
  static QIcon getIconToRemove(){ return QIcon::fromTheme("list-remove", QIcon(":/resources/images/toremove.png")); }
  static QIcon getIconToInstall(){ return QIcon::fromTheme("list-add", QIcon(":/resources/images/toinstall.png")); }
  static QIcon getIconRemove(){ return QIcon::fromTheme("list-remove", QIcon(":/resources/images/close.png")); }
  static QIcon getIconTerminal(){ return QIcon::fromTheme("utilities-terminal", QIcon(":/resources/images/terminal.png")); }
  static QIcon getIconRemoveItem() { return QIcon::fromTheme("list-remove", QIcon(":/resources/images/remove_item.png")); }
  static QIcon getIconInstallItem() { return QIcon::fromTheme("list-add", QIcon(":/resources/images/install_item.png")); }
  static QIcon getIconExit(){ return QIcon::fromTheme("application-exit", QIcon(":/resources/images/exit.png")); }

  // Icons for QActions
  static QIcon getIconSyncPackages(){ return QIcon::fromTheme("view-refresh", QIcon(":/resources/images/refresh.png")); }
  static QIcon getIconHelpAbout(){ return QIcon::fromTheme("help-about"); }
  static QIcon getIconHelpUsage(){ return QIcon::fromTheme("help-contents"); }
  static QIcon getIconNonInstalledPackages(){ return QIcon::fromTheme("view-refresh", QIcon(":/resources/images/refresh.png")); }
  static QIcon getIconCommit(){ return QIcon::fromTheme("dialog-ok-apply", QIcon(":/resources/images/commit.png")); }
  static QIcon getIconRollback(){ return QIcon::fromTheme("edit-undo", QIcon(":/resources/images/rollback.png")); }
  static QIcon getIconSystemUpgrade(){ return QIcon::fromTheme("go-up", QIcon(":/resources/images/fast_forward.png")); }
  static QIcon getIconInstall(){ return QIcon::fromTheme("list-add", QIcon(":/resources/images/toinstall.png")); }
  static QIcon getIconGetNews(){ return QIcon::fromTheme("application-rss+xml", QIcon(":/resources/images/rss.png")); }
  static QIcon getIconRemoveGroup(){ return QIcon::fromTheme("list-remove", QIcon(":/resources/images/toremove.png")); }
  static QIcon getIconInstallGroup(){ return QIcon::fromTheme("list-add", QIcon(":/resources/images/toinstall.png")); }
  static QIcon getIconCollapseItem(){ return QIcon::fromTheme("zoom-out", QIcon(":/resources/images/collapse.png")); }
  static QIcon getIconExpandItem(){ return QIcon::fromTheme("zoom-in", QIcon(":/resources/images/expand.png")); }
  static QIcon getIconCollapseAllItems(){ return QIcon::fromTheme("zoom-out", QIcon(":/resources/images/collapse.png")); }
  static QIcon getIconExpandAllItems(){ return QIcon::fromTheme("zoom-in", QIcon(":/resources/images/expand.png")); }
  static QIcon getIconOpenFile(){ return QIcon::fromTheme("document-open", QIcon(":/resources/images/binary.png")); }
  static QIcon getIconEditFile(){ return QIcon::fromTheme("document-edit", QIcon(":/resources/images/editfile.png")); }
  static QIcon getIconOpenDirectory(){ return QIcon::fromTheme("document-open-folder", QIcon(":/resources/images/folder.png")); }
  static QIcon getIconOpenTerminal(){ return QIcon::fromTheme("utilities-terminal", QIcon(":/resources/images/terminal.png")); }
  static QIcon getIconRemoveTransactionItem(){ return QIcon::fromTheme("edit-delete", QIcon(":/resources/images/window_close.png")); }
  static QIcon getIconRemoveTransactionItems(){ return QIcon::fromTheme("edit-delete", QIcon(":/resources/images/window_close.png")); }
  static QIcon getIconFindFileInPackage(){ return QIcon::fromTheme("edit-find", QIcon(":/resources/images/find.png")); }
  static QIcon getIconOpenRootTerminal(){ return QIcon::fromTheme("utilities-terminal", QIcon(":/resources/images/terminal.png")); }
  static QIcon getIconInstallYaourt(){ return QIcon::fromTheme("list-add", QIcon(":/resources/images/toinstall.png")); }
  static QIcon getIconInstallLocalPackage(){ return QIcon::fromTheme("utilities-file-archiver"); }
  static QIcon getIconRunInTerminal(){ return QIcon::fromTheme("utilities-terminal", QIcon(":/resources/images/terminal.png")); }
  static QIcon getIconMirrorCheck(){ return QIcon::fromTheme("svn-update", QIcon(":/resources/images/mirror-check.png")); }
  static QIcon getIconShowGroups(){ return QIcon::fromTheme("view-list-tree", QIcon(":/resources/images/show_groups.png")); }
  static QIcon getIconClose(){ return QIcon::fromTheme("edit-delete", QIcon(":/resources/images/window_close.png")); }
  static QIcon getIconSearch(){ return QIcon::fromTheme("edit-find", QIcon(":/resources/images/esf-search.png")); }
  static QIcon getIconClear(){ return QIcon::fromTheme("edit-clear-locationbar-ltr", QIcon(":/resources/images/esf-clear.png")); }
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

  ~CPUIntensiveComputing(){
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
