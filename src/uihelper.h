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

#ifndef ICONHELPER_H
#define ICONHELPER_H

#include <QDir>
#include <QIcon>
#include <QApplication>
#include <QWidget>

//IconHelper provides some very used icons to the interface
class IconHelper{
public:
  static QIcon getIconInternalError(){ return QIcon(":/resources/images/internalerror.png"); }
  static QIcon getIconFolder(){ return QIcon(":/resources/images/folder.png"); }
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
  static QIcon getIconRemove(){ return QIcon(":/resources/images/close.png"); }

  static QIcon getIconExit(){ return QIcon(":/resources/images/exit.png"); }
};

//SelectedPackage abstracts a package that is currently selected at the "Packages in a directory" view
class SelectedPackage{
private:
  QString path;
  QString fileName;
  QIcon icon;

public:
  SelectedPackage(QString path, QString fileName, QIcon icon){
    this->path = path;
    this->fileName = fileName;
    this->icon = icon;
  }

  SelectedPackage(QString path, QString fileName){
    this->path = path;
    this->fileName = fileName;
  }

  SelectedPackage(){}

  QString getCompleteFileName(){
    return path + QDir::separator() + fileName;
  }

  QString getFileName() {
    return fileName;
  }

  void setFileName(QString fileName) {
    this->fileName = fileName;
  }

  QString getPath() {
    return path;
  }

  QIcon getIcon(){
    return icon;
  }

  void setPath(QString path) {
    this->path = path;
  }

  void setIcon(QIcon icon) {
    this->icon = icon;
  }
};

//PackagesClipBoard implements a ClipBoard feature for "Packages in a directory" view
struct PackagesClipBoard{
private:
  QStringList m_packageList;
  QString m_sourceDir;
  bool m_cutOperation;

public:
  inline void setCutOperation(bool param){ m_cutOperation = param; }
  inline bool getCutOperation(){ return m_cutOperation; }
  inline QStringList getPackageList(){ return m_packageList; }
  inline void setSourceDir(QString param){ m_sourceDir = param; }
  inline QString getSourceDir(){ return m_sourceDir; }
  inline void add(QString pkg){ m_packageList.append(pkg); }
  inline int count(){ return m_packageList.count(); }
  inline void clear(){ m_packageList.clear(); }
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
