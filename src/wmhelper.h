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
const QString ctn_ROOT_SH("/bin/sh -c ");

const QString ctn_KDESU("kdesu");

const QString ctn_KDE_DESKTOP("kwin");
const QString ctn_KDE_X11_DESKTOP("kwin_x11");
const QString ctn_KDE_WAYLAND_DESKTOP("kwin_wayland");
const QString ctn_KDE_EDITOR("kwrite");
const QString ctn_KDE_FILE_MANAGER("kfmclient");
const QString ctn_KDE_TERMINAL("konsole");
const QString ctn_KDE4_OPEN("kde-open");
const QString ctn_KDE5_OPEN("kde-open5");
const QString ctn_KDE4_FILE_MANAGER("dolphin");
const QString ctn_KDE4_EDITOR("kate");

const QString ctn_TDESU("tdesu");
const QString ctn_TDE_DESKTOP("twin");
const QString ctn_TDE_EDITOR("kwrite");
const QString ctn_TDE_FILE_MANAGER("kfmclient");
const QString ctn_TDE_TERMINAL("konsole");

const QString ctn_GKSU_1("/usr/libexec/gksu");
const QString ctn_GKSU_2("gksu");

const QString ctn_ANTERGOS_FILE_MANAGER("nautilus");

const QString ctn_ARCHBANG_EDITOR("medit");
const QString ctn_ARCHBANG_FILE_MANAGER("spacefm");

const QString ctn_MOOOS_EDITOR("subl3");
const QString ctn_RXVT_TERMINAL("urxvt");

const QString ctn_XFCE_DESKTOP("xfdesktop");
const QString ctn_XFCE_EDITOR("mousepad");
const QString ctn_XFCE_EDITOR_ALT("leafpad");
const QString ctn_XFCE_FILE_MANAGER("thunar");
const QString ctn_XFCE_TERMINAL("xfce4-terminal");

const QString ctn_OPENBOX_DESKTOP("openbox");
const QString ctn_LXDE_DESKTOP("lxsession");
const QString ctn_LXDE_TERMINAL("lxterminal");
const QString ctn_LXDE_FILE_MANAGER("pcmanfm");

const QString ctn_LXQT_DESKTOP("lxqt-session");
const QString ctn_LXQT_TERMINAL("qterminal");
const QString ctn_LXQT_FILE_MANAGER("pcmanfm-qt");
const QString ctn_LXQT_EDITOR("juffed");

const QString ctn_MATE_DESKTOP("mate-session");
const QString ctn_MATE_EDITOR("mate-open");
const QString ctn_MATE_FILE_MANAGER("caja");
const QString ctn_MATE_TERMINAL("mate-terminal");

const QString ctn_CINNAMON_DESKTOP("gnome-session");
const QString ctn_CINNAMON_EDITOR("gedit");
const QString ctn_CINNAMON_FILE_MANAGER("nemo");
const QString ctn_CINNAMON_TERMINAL("gnome-terminal");

const QString ctn_RAZORQT_DESKTOP("razor-session");
const QString ctn_PEK_TERMINAL("sakura");
const QString ctn_XTERM("xterm");

enum EditOptions { ectn_EDIT_AS_ROOT, ectn_EDIT_AS_NORMAL_USER };

/*
 * This class exposes some services of the underlying Window Manager being used
 */

class WMHelper
{
public:
  static bool isKDERunning();
  static bool isTDERunning();
  static bool isXFCERunning();
  static bool isOPENBOXRunning();
  static bool isLXDERunning();
  static bool isLXQTRunning();
  static bool isMATERunning();
  static bool isCinnamonRunning();
  static bool isRazorQtRunning();

  static QString getKDEOpenHelper();

  static QString getKDESUCommand();
  static QString getTDESUCommand();
  static QString getGKSUCommand();
  static QString getXFCEEditor();
  static QString getSUCommand();
  static void openFile(const QString& fileName); //fileName is Path + Name
  static void editFile(const QString& fileName , EditOptions opt = ectn_EDIT_AS_ROOT); //fileName is Path + Name
  static void openDirectory( const QString& dirName );
  static void openTerminal( const QString& dirName );
};

#endif // WMHELPER_H
