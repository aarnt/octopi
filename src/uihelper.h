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
  static QIcon getIconOctopiRed(){ return QIcon(":/resources/images/octopi_red.png"); }
  static QIcon getIconOctopiYellow(){ return QIcon(":/resources/images/octopi_yellow.png"); }
  static QIcon getIconOctopiGreen(){ return QIcon(":/resources/images/octopi_green.png"); }
  static QIcon getIconInternalError(){ return QIcon(":/resources/images/internalerror.png"); }

  static QIcon getIconFolder()
  {
    if (WMHelper::isKDERunning())
      return QIcon(":/resources/images/folder.png");
    else
      return QIcon(":/resources/images/folder_gnome.png");
  }

  static QIcon getIconBinary(){ return QIcon(":/resources/images/binary.png"); }
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
  static QIcon getIconToRemove(){ return QIcon(":/resources/images/toremove.png"); }  
  static QIcon getIconToInstall(){ return QIcon(":/resources/images/toinstall.png"); }
  static QIcon getIconRemove(){ return QIcon(":/resources/images/close.png"); }
  static QIcon getIconTerminal(){ return QIcon(":/resources/images/terminal.png"); }
  static QIcon getIconRemoveItem() { return QIcon(":/resources/images/remove_item.png"); }
  static QIcon getIconInstallItem() { return QIcon(":/resources/images/install_item.png"); }
  static QIcon getIconExit(){ return QIcon(":/resources/images/exit.png"); }
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
