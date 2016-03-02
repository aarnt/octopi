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

#include "constants.h"

#include <QString>

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
  static QString getLXQTSUCommand();
  static QString getXFCEEditor();
  static QString getSUCommand();
  static void openFile(const QString& fileName); //fileName is Path + Name
  static void editFile(const QString& fileName , EditOptions opt = ectn_EDIT_AS_ROOT); //fileName is Path + Name
  static void openDirectory( const QString& dirName );
  static void openTerminal( const QString& dirName );
};

#endif // WMHELPER_H
