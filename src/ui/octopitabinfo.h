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

#ifndef OCTOPITABINFO_H
#define OCTOPITABINFO_H

#include "src/packagerepository.h"


/**
 * @brief The OctopiTabInfo class provides functionality for the Tab "Info"
 * 
 * <b>PackageName</b>
 * 
 * packagedescription
 * 
 * site/URL  www.example.org
 * Version   1.2.3.4
 * Licence   GPL
 * Depends  abcde fg
 * Opt.Dep  abcde: why\n
 *          fg: why
 * Download 5kb
 * Install  10kb
 * Packager notme@any.where
 * Arch     any
 * Build    1.1.2000
 */

class OctopiTabInfo
{
public:
  OctopiTabInfo();
  
  /**
   * @brief formats TabInfo as HTML
   * @param package (the package to show details for)
   * @param outdatedAURPackagesNameVersion
   * @return html
   *
   * This function has been extracted from src/mainwindow_refresh.cpp void MainWindow::refreshTabInfo(QString pkgName)
   */
  static QString formatTabInfo(const PackageRepository::PackageData& package, const QHash<QString, QString>& outdatedAURPackagesNameVersion);

  static const QString anchorBegin;
};

#endif // OCTOPITABINFO_H
