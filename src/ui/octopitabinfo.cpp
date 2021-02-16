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
#include "src/unixcommand.h"

#include <QLocale>

/*
 * The OctopiTabInfo class provides functionality for the Tab "Info"
 */

/**
 * @brief OctopiTabInfo::anchorBegin for navigation
 */
const QString OctopiTabInfo::anchorBegin(QStringLiteral("anchorBegin"));

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
      (!package.installed() || package.outdated())) {
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
  QString installDate = StrConstants::getInstallDate();
  QString installReason = StrConstants::getInstallReason();

  //Let's put package description in UTF-8 format
  QString pkgDescription = pid.description;
  QString html;
  html += QLatin1String("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
  html += QLatin1String("<a id=\"") + anchorBegin + QLatin1String("\"></a>");

  if (package.repository != StrConstants::getForeignRepositoryName())
  {
    html += QLatin1String("<h2>") + package.name + QLatin1String("</h2>");
  }
  else if (UnixCommand::getLinuxDistro() == ectn_KAOS || UnixCommand::getLinuxDistro() == ectn_CHAKRA)
    html += QLatin1String("<h2>") + package.name + QLatin1String("</h2>");
  else
    html += QLatin1String("<h2><a href=\"https://aur.archlinux.org/packages/") + package.name + QLatin1String("\">") + package.name + QLatin1String("</a></h2>");

  html += pkgDescription;

  html += QLatin1String("<table border=\"0\">");

  html += QLatin1String("<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>");
  html += QLatin1String("<tr><td>") + url + QLatin1String("</td><td>") + pid.url + QLatin1String("</td>");

  if (package.outdated())
  {
    if (package.status != ectn_NEWER)
    {
      if (package.status == ectn_OUTDATED)
      {
        QString outdatedVersion = package.outdatedVersion;
        html += QLatin1String("<tr><td>") + version + QLatin1String("</td><td>") + package.version + QLatin1String(" <b><font color=\"#E55451\">")
            + StrConstants::getOutdatedInstalledVersion().arg(outdatedVersion) +
            QLatin1String("</b></font></td></tr>");
      }
      else if (package.status == ectn_FOREIGN_OUTDATED && //NEW ELSE
          outdatedAURPackagesNameVersion.count() > 0)
      {
        QString outdatedVersion = package.outdatedVersion;
        if (outdatedVersion.isEmpty()) outdatedVersion = package.version;
        QString availableVersion = outdatedAURPackagesNameVersion.value(package.name);
        if (availableVersion.isEmpty()) availableVersion = package.version;

        html += QLatin1String("<tr><td>") + version + QLatin1String("</td><td>") + availableVersion + QLatin1String(" <b><font color=\"#E55451\">")
            + StrConstants::getOutdatedInstalledVersion().arg(outdatedVersion) +
            QLatin1String("</b></font></td></tr>");
      }
    }
    else
    {
      QString newerVersion = package.outdatedVersion;
      html += QLatin1String("<tr><td>") + version + QLatin1String("</td><td>") + package.version + QLatin1String(" <b><font color=\"#FF8040\">")
          + StrConstants::getNewerInstalledVersion().arg(newerVersion) +
          QLatin1String("</b></font></td></tr>");
    }
  }
  else //Is this Else needed?
  {
    if (package.repository != StrConstants::getForeignRepositoryName())
    {
      html += QLatin1String("<tr><td>") + version + QLatin1String("</td><td>") + package.version + QLatin1String("</td></tr>");
    }
    else
    {
      if (package.status == ectn_FOREIGN_OUTDATED &&
          outdatedAURPackagesNameVersion.count() > 0)
      {
        QString outdatedVersion = package.outdatedVersion;
        if (outdatedVersion.isEmpty()) outdatedVersion = package.version;
        QString availableVersion = outdatedAURPackagesNameVersion.value(package.name);
        if (availableVersion.isEmpty()) availableVersion = package.version;

        html += QLatin1String("<tr><td>") + version + QLatin1String("</td><td>") + availableVersion + QLatin1String(" <b><font color=\"#E55451\">")
            + StrConstants::getOutdatedInstalledVersion().arg(outdatedVersion) +
            QLatin1String("</b></font></td></tr>");
      }
      else
      {
        html += QLatin1String("<tr><td>") + version + QLatin1String("</td><td>") + package.version + QLatin1String("</td></tr>");
      }
    }
  }

  //This is needed as packager names could be encoded in different charsets, resulting in an error
  QString packagerName = pid.packager;
  packagerName = packagerName.replace(QLatin1String("<"), QLatin1String("&lt;"));
  packagerName = packagerName.replace(QLatin1String(">"), QLatin1String("&gt;"));

  QString strConflictsWith = pid.conflictsWith;
  strConflictsWith = strConflictsWith.replace(QLatin1String("<"), QLatin1String("&lt;"));
  strConflictsWith = strConflictsWith.replace(QLatin1String(">"), QLatin1String("&gt;"));
  strConflictsWith = strConflictsWith.replace(QLatin1String("&lt;br&gt;"), QLatin1String("<br>"));

  html += QLatin1String("<tr><td>") + licenses + QLatin1String("</td><td>") + pid.license + QLatin1String("</td></tr>");

  //Show this info only if there's something to show
  if(! pid.group.contains(QLatin1String("None")))
    html += QLatin1String("<tr><td>") + groups + QLatin1String("</td><td>") + pid.group + QLatin1String("</td></tr>");

  if(! pid.provides.contains(QLatin1String("None")))
    html += QLatin1String("<tr><td>") + provides + QLatin1String("</td><td>") + Package::makeAnchorOfPackage(pid.provides) + QLatin1String("</td></tr>");

  if(! pid.dependsOn.contains(QLatin1String("None")))
    html += QLatin1String("<tr><td>") + dependsOn + QLatin1String("</td><td>") + Package::makeAnchorOfPackage(pid.dependsOn) + QLatin1String("</td></tr>");

  if(! pid.optDepends.contains(QLatin1String("None")))
    html += QLatin1String("<tr><td>") + optionalDeps + QLatin1String("</td><td>") + Package::makeAnchorOfOptionalDep(pid.optDepends) + QLatin1String("</td></tr>");

  if(!pid.requiredBy.isEmpty() && !pid.requiredBy.contains(QLatin1String("None")))
    html += QLatin1String("<tr><td>") + requiredBy + QLatin1String("</td><td>") + Package::makeAnchorOfPackage(pid.requiredBy) + QLatin1String("</td></tr>");

  if(!pid.optionalFor.isEmpty() && !pid.optionalFor.contains(QLatin1String("None")))
    html += QLatin1String("<tr><td>") + optionalFor + QLatin1String("</td><td>") + Package::makeAnchorOfPackage(pid.optionalFor) + QLatin1String("</td></tr>");

  if(! pid.conflictsWith.contains(QLatin1String("None")))
    html += QLatin1String("<tr><td><b>") + conflictsWith + QLatin1String("</b></td><td><b>") + Package::makeAnchorOfPackage(strConflictsWith) +
        QLatin1String("</b></font></td></tr>");

  if(! pid.replaces.contains(QLatin1String("None")))
    html += QLatin1String("<tr><td>") + replaces + QLatin1String("</td><td>") + Package::makeAnchorOfPackage(pid.replaces) + QLatin1String("</td></tr>");

  if (!pid.downloadSize.isNull())
    html += QLatin1String("<tr><td>") + downloadSize + QLatin1String("</td><td>") + /*Package::kbytesToSize*/ (pid.downloadSize) + QLatin1String("</td></tr>");

  if (!pid.installedSize.isNull())
    html += QLatin1String("<tr><td>") + installedSize + QLatin1String("</td><td>") + /*Package::kbytesToSize*/ (pid.installedSize) + QLatin1String("</td></tr>");

  html += QLatin1String("<tr><td>") + packager + QLatin1String("</td><td>") + packagerName + QLatin1String("</td></tr>");
  html += QLatin1String("<tr><td>") + architecture + QLatin1String("</td><td>") + pid.arch + QLatin1String("</td></tr>");

  QString dateTimeFormat = QLocale().dateTimeFormat();

  html += QLatin1String("<tr><td>") + buildDate + QLatin1String("</td><td>") +
      pid.buildDate.toString(dateTimeFormat) + QLatin1String("</td></tr>");

  if (!pid.installDate.isNull())
    html += QLatin1String("<tr><td>") + installDate + QLatin1String("</td><td>") +
        pid.installDate.toString(dateTimeFormat) + QLatin1String("</td></tr>");

  if(!pid.installReason.isEmpty())
    html += QLatin1String("<tr><td>") + installReason + QLatin1String("</td><td>") + pid.installReason + QLatin1String("</td></tr>");

  html += QLatin1String("</table><br>");

  return html;
}
