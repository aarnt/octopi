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

#include "octopitabinfo.h"
#include "src/strconstants.h"
#include "src/package.h"

/*
 * The OctopiTabInfo class provides functionality for the Tab "Info"
 */

/**
 * @brief OctopiTabInfo::anchorBegin for navigation
 */
const QString OctopiTabInfo::anchorBegin("anchorBegin");

OctopiTabInfo::OctopiTabInfo()
{
}

/**
 * This function has been extracted from src/mainwindow_refresh.cpp void MainWindow::refreshTabInfo(QString pkgName)
 */
QString OctopiTabInfo::formatTabInfo(const PackageRepository::PackageData& package,
                                     const QHash<QString, QString>& outdatedAURPackagesNameVersion)
{
  PackageInfoData pid;

  if (package.repository != StrConstants::getForeignRepositoryName() &&
      (package.installed() == false || package.outdated() == true)) {
    pid = Package::getInformation(package.name);
  }
  else
  {
    pid = Package::getInformation(package.name, true); //This is a foreign package!!!
  }

  QString version = StrConstants::getVersion();
  QString url = StrConstants::getURL();
  QString licenses = StrConstants::getLicenses();
  QString groups = StrConstants::getGroups();
  QString provides = StrConstants::getProvides();
  QString dependsOn = StrConstants::getDependsOn();
  QString requiredBy = StrConstants::getRequiredBy();
  QString optionalFor = StrConstants::getOptionalFor();
  QString optionalDeps = StrConstants::getOptionalDeps();
  QString conflictsWith = StrConstants::getConflictsWith();
  QString replaces = StrConstants::getReplaces();
  QString downloadSize = StrConstants::getDownloadSize();
  QString installedSize = StrConstants::getInstalledSize();
  QString packager = StrConstants::getPackager();
  QString architecture = StrConstants::getArchitecture();
  QString buildDate = StrConstants::getBuildDate();

  //Let's put package description in UTF-8 format
  QString pkgDescription = pid.description;

#if QT_VERSION < 0x050000
  pkgDescription = pkgDescription.fromUtf8(pkgDescription.toLatin1().data());
#endif

  QString html;
  html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
  html += "<a id=\"" + anchorBegin + "\"></a>";

  html += "<h2>" + package.name + "</h2>";
  html += "<a style=\"font-size:16px;\">" + pkgDescription + "</a>";

  html += "<table border=\"0\">";

  html += "<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>";
  html += "<tr><td>" + url + "</td><td style=\"font-size:14px;\">" + pid.url + "</td></tr>";

  if (package.outdated())
  {
    if (package.status != ectn_NEWER)
    {
      //if (package.repository != StrConstants::getForeignRepositoryName())
      {
        QString outdatedVersion = package.outdatedVersion;
        html += "<tr><td>" + version + "</td><td>" + package.version + " <b><font color=\"#E55451\">"
            + StrConstants::getOutdatedInstalledVersion().arg(outdatedVersion) +
            "</b></font></td></tr>";
      }
    }
    else
    {
      QString newerVersion = package.outdatedVersion;
      html += "<tr><td>" + version + "</td><td>" + package.version + " <b><font color=\"#FF8040\">"
          + StrConstants::getNewerInstalledVersion().arg(newerVersion) +
          "</b></font></td></tr>";
    }
  }
  else
  {
    if (package.repository != StrConstants::getForeignRepositoryName())
    {
      html += "<tr><td>" + version + "</td><td>" + package.version + "</td></tr>";
    }
    else
    {
      if (package.status == ectn_FOREIGN_OUTDATED &&
          outdatedAURPackagesNameVersion.count() > 0)
      {
        QString availableVersion = outdatedAURPackagesNameVersion.value(package.name);
        html += "<tr><td>" + version + "</td><td>" + availableVersion + " <b><font color=\"#E55451\">"
            + StrConstants::getOutdatedInstalledVersion().arg(package.version) +
            "</b></font></td></tr>";
      }
      else
      {
        html += "<tr><td>" + version + "</td><td>" + package.version + "</td></tr>";
      }
    }
  }

  //This is needed as packager names could be encoded in different charsets, resulting in an error
  QString packagerName = pid.packager;
  packagerName = packagerName.replace("<", "&lt;");
  packagerName = packagerName.replace(">", "&gt;");

#if QT_VERSION < 0x050000
  packagerName = packagerName.fromUtf8(packagerName.toLatin1().data());
#endif

  QString strConflictsWith = pid.conflictsWith;
  strConflictsWith = strConflictsWith.replace("<", "&lt;");
  strConflictsWith = strConflictsWith.replace(">", "&gt;");
  strConflictsWith = strConflictsWith.replace("&lt;br&gt;", "<br>");

  html += "<tr><td>" + licenses + "</td><td>" + pid.license + "</td></tr>";

  //Show this info only if there's something to show
  if(! pid.group.contains("None"))
    html += "<tr><td>" + groups + "</td><td>" + pid.group + "</td></tr>";
  if(! pid.provides.contains("None"))
    html += "<tr><td>" + provides + "</td><td>" + pid.provides + "</td></tr>";
  if(! pid.dependsOn.contains("None"))
    html += "<tr><td>" + dependsOn + "</td><td>" + pid.dependsOn + "</td></tr>";
  if(! pid.optDepends.contains("None"))
    html += "<tr><td>" + optionalDeps + "</td><td>" + pid.optDepends + "</td></tr>";
  if(!pid.requiredBy.isEmpty() && !pid.requiredBy.contains("None"))
    html += "<tr><td>" + requiredBy + "</td><td>" + pid.requiredBy + "</td></tr>";
  if(!pid.optionalFor.isEmpty() && !pid.optionalFor.contains("None"))
    html += "<tr><td>" + optionalFor + "</td><td>" + pid.optionalFor + "</td></tr>";
  if(! pid.conflictsWith.contains("None"))
    html += "<tr><td><b>" + conflictsWith + "</b></td><td><b>" + strConflictsWith +
        "</b></font></td></tr>";

  if(! pid.replaces.contains("None"))
    html += "<tr><td>" + replaces + "</td><td>" + pid.replaces + "</td></tr>";

  html += "<tr><td>" + downloadSize + "</td><td>" + Package::kbytesToSize(pid.downloadSize) + "</td></tr>";
  html += "<tr><td>" + installedSize + "</td><td>" + Package::kbytesToSize(pid.installedSize) + "</td></tr>";
  html += "<tr><td>" + packager + "</td><td>" + packagerName + "</td></tr>";
  html += "<tr><td>" + architecture + "</td><td>" + pid.arch + "</td></tr>";
  html += "<tr><td>" + buildDate + "</td><td>" +
      pid.buildDate.toString("ddd - dd/MM/yyyy hh:mm:ss") + "</td></tr>";

  html += "</table>";
  return html;
}
