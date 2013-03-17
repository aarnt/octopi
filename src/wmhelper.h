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

#ifndef WMHELPER_H
#define WMHELPER_H

#include <QString>

const QString ctn_NO_SU_COMMAND("none");

const QString ctn_ROOT_SH("sh");  //"/bin/sh -c");

const QString ctn_KDESU("kdesu");
const QString ctn_KDE_DESKTOP("kwin");
const QString ctn_KDE_EDITOR("kwrite");
const QString ctn_KDE_FILE_MANAGER("kfmclient");
const QString ctn_KDE_TERMINAL("konsole");
const QString ctn_KDE4_FILE_MANAGER("dolphin");

const QString ctn_TDESU("tdesu");
const QString ctn_TDE_DESKTOP("twin");
const QString ctn_TDE_EDITOR("kwrite");
const QString ctn_TDE_FILE_MANAGER("kfmclient");
const QString ctn_TDE_TERMINAL("konsole");

const QString ctn_KTSUSS("ktsuss");
const QString ctn_GKSU_1("/usr/libexec/gksu");
const QString ctn_GKSU_2("gksu");
const QString ctn_XFCE_DESKTOP("xfdesktop");
const QString ctn_XFCE_EDITOR("mousepad");
const QString ctn_XFCE_EDITOR_ALT("leafpad");
const QString ctn_XFCE_FILE_MANAGER("thunar");
const QString ctn_XFCE_TERMINAL("terminal");

const QString ctn_LXDE_DESKTOP("lxsession");
const QString ctn_LXDE_TERMINAL("lxterminal");
const QString ctn_LXDE_FILE_MANAGER("pcmanfm");

const QString ctn_MATE_DESKTOP("mate-session");
const QString ctn_MATE_EDITOR("mate-open");
const QString ctn_MATE_FILE_MANAGER("caja");
const QString ctn_MATE_TERMINAL("mate-terminal");

/* This class exposes some services of the underlying Window Manager being used */
class WMHelper
{
public:
  static bool isKDERunning();
  static bool isTDERunning();
  static bool isXFCERunning();
  static bool isLXDERunning();
  static bool isMATERunning();

  static QString getKDESUCommand();
  static QString getTDESUCommand();
  static QString getKTSUSSCommand();
  static QString getGKSUCommand();
  static QString getXFCEEditor();
  static QString getSUCommand();

  static void openFile(const QString& fileName); //fileName is Path + Name
  static void editFile( const QString& fileName ); //fileName is Path + Name
  static void openDirectory( const QString& dirName );
  static void openTerminal( const QString& dirName );
};

#endif // WMHELPER_H
