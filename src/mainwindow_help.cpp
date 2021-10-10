/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2013  Alexandre Albuquerque Arnt
*               2013  Manuel Tortosa
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

/*
 * This is MainWindow's Help related code
 */

#include "strconstants.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "searchbar.h"

#include <QString>
#include <QTextBrowser>
#include <QMessageBox>
#include <QDesktopServices>

/*
 * Initialize the Help tab with basic information about using Octopi
 */
void MainWindow::initTabHelpUsage()
{
  ui->twProperties->setHelpUsageText(generateHelpUsageHtmlText());
  ui->twProperties->initTabHelpUsage();

  connect(ui->twProperties->getSearchBarHelpUsage(), SIGNAL(textChanged(QString)),
          this, SLOT(searchBarTextChangedInTextBrowser(QString)));
  connect(ui->twProperties->getSearchBarHelpUsage(), SIGNAL(closed()),
          this, SLOT(searchBarClosedInTextBrowser()));
  connect(ui->twProperties->getSearchBarHelpUsage(), SIGNAL(findNext()),
          this, SLOT(searchBarFindNextInTextBrowser()));
  connect(ui->twProperties->getSearchBarHelpUsage(), SIGNAL(findPrevious()),
          this, SLOT(searchBarFindPreviousInTextBrowser()));
}

/*
 * Generates help string using HTML markup language
 */
QString MainWindow::generateHelpUsageHtmlText()
{
  /*QString strNoPassword;
  strNoPassword=tr("To use Octopi without a password your user need to be member of the \"wheel\" group") +
      "<br>" + tr("and you must run the following command as root:") +
      "<ul><li>/usr/lib/octopi/octopi-sudo -setnopasswd</li></ul><br>";*/

  QString strOutdatedAur;
  if(m_hasForeignTool && UnixCommand::getLinuxDistro() != ectn_CHAKRA /*&& !SettingsManager::getSearchOutdatedAURPackages()*/)
  {
    strOutdatedAur=QLatin1String("<li>") +
        tr("Ctrl+Shift+O to display outdated %1 packages").arg(StrConstants::getForeignRepositoryName()) + QLatin1String("</li>");
  }
  else strOutdatedAur=QStringLiteral("<li></li>");

  QString strVote;
  if(m_aurVote != nullptr)
  {
    strVote=QLatin1String("<li>") +
       tr("Ctrl+Shift+A to display AUR voted package list") + QLatin1String("</li>");
  }
  else strVote=QStringLiteral("<li></li>");

  QString iconPath = QStringLiteral("<img height=\"16\" width=\"16\" src=\":/resources/images/");
  QString strForMoreInfo = tr("For more information, visit:");
  QString html =
    QStringLiteral("<h2>Octopi</h2>") +
    QStringLiteral("<h3><p>") + tr("A Qt-based Pacman frontend,") + QLatin1Char(' ') +
    tr("licensed under the terms of") + QLatin1Char(' ');

    html +=
      QStringLiteral("<a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GPL v2</a>.</p></h3>") +
      QStringLiteral("<h4><p>") + strForMoreInfo + QLatin1Char(' ') +
      QStringLiteral("<a href=\"https://tintaescura.com/projects/octopi/\">https://tintaescura.com/projects/octopi</a><br>");
    html += QStringLiteral("<br>") +
      tr("Package classification:") +

  QStringLiteral("<ul type=\"square\"><li>") + iconPath + QLatin1String("installed.png\"/> ") +
     tr("An installed package") + QStringLiteral("</li>") +
  QStringLiteral("<li>") + iconPath + QLatin1String("unrequired.png\"/> ") +
     tr("An installed package (not required by others)") +
  QStringLiteral("</li>") +
  QString(QLatin1String("<li>") + iconPath + QLatin1String("foreign_green.png\"/> ")) +
     tr("A foreign package, installed from") + QLatin1Char(' ') + StrConstants::getForeignRepositoryName() +
  QStringLiteral("</li>") +
  QStringLiteral("<li>") + iconPath + QLatin1String("noninstalled.png\"/> ") +
     tr("A non installed package") +
  QStringLiteral("</li>") +
  QStringLiteral("<li>") + iconPath + QLatin1String("outdated.png\"/> ") +
     tr("An outdated package") +
  QStringLiteral("</li>") +
  QStringLiteral("<li>") + iconPath + QLatin1String("foreign_red.png\"/> ") +
     tr("An outdated foreign package") +
  QStringLiteral("</li>") +
  QStringLiteral("<li>") + iconPath + QLatin1String("newer.png\"/> ") +
           tr("A newer than repository package") +
  QStringLiteral("</li></ul>") +
     tr("Basic usage help:") +
  QStringLiteral("<ul><li>") +
     tr("Position the mouse over a package to see its description") +
  QStringLiteral("</li><li>") +
     tr("Double click an installed package to see its contents") +
  QStringLiteral("</li><li>") +
     tr("Right click package to install/reinstall or remove it") +
  QStringLiteral("</li></ul>") +

     tr("Alt+key sequences:") +
  QStringLiteral("<ul><li>") +
     tr("Alt+1 to switch to 'Info' tab") +
  QStringLiteral("</li><li>") +
     tr("Alt+2 to switch to 'Files' tab") +
  QStringLiteral("</li><li>") +
     tr("Alt+3 to switch to 'Actions' tab") +
  QStringLiteral("</li><li>") +
     tr("Alt+4 to switch to 'Output' tab") +
  QStringLiteral("</li><li>") +
     tr("Alt+5 to switch to 'News' tab") +
  QStringLiteral("</li><li>") +
     tr("Alt+6 or 'F1' to show this help page") +
  QStringLiteral("</li><li>") +
     tr("Alt+7 to switch to 'Terminal' tab") +
  QStringLiteral("</li></ul>") +

     tr("Control+key sequences:") +
  QStringLiteral("<ul><li>") +
     tr("Ctrl+E or 'Actions/Cancel' to clear the selection of to be removed/installed packages") +
  QStringLiteral("</li><li>") +
     tr("Ctrl+F to search for text inside tab Files, News and Usage") +
  QStringLiteral("</li><li>") +
     tr("Ctrl+G or 'File/Get latest distro news' to retrieve the latest RSS based distro news") +
  QStringLiteral("</li><li>") +
     tr("Ctrl+K or 'File/Check updates' to check mirror for latest updates (checkupdates)") +
  QStringLiteral("</li><li>") +
     tr("Ctrl+L to find a package in the package list") +
  QStringLiteral("</li><li>") +
     tr("Ctrl+P to go to package list") +
  QStringLiteral("</li><li>") +
     tr("Ctrl+Q or 'File/Exit' to exit the application") +
  QStringLiteral("</li><li>") +
     tr("Ctrl+U or 'File/System upgrade' to make a full system upgrade (pacman -Su)") +
  QStringLiteral("</li><li>") +
     tr("Ctrl+Y or 'Actions/Apply' to start installation/removal of selected packages") +
  QStringLiteral("</li></ul>") +

     tr("Control+shift+key sequences:") +
  QStringLiteral("<ul>") +
  strVote +
  QStringLiteral("<li>") +
     tr("Ctrl+Shift+G to display all package groups") +
  QStringLiteral("</li>") +
     strOutdatedAur + QLatin1String("<li>") +
     //tr("Ctrl+Shift+R to remove Pacman's transaction lock file") +
  //QString("</li><li>") +
     tr("Ctrl+Shift+Y to display %1 group").arg(StrConstants::getForeignRepositoryGroupName()) +
  QStringLiteral("</li></ul>") +

     tr("F+key sequences:") +
  QStringLiteral("<ul><li>") +
     tr("F1 to show this help page") +
  QStringLiteral("</li><li>") +
     tr("F4 to open a Terminal whitin the selected directory at Files tab") +
  QStringLiteral("</li><li>") +
     tr("F6 to open a File Manager whitin the selected directory at Files tab") +
  QStringLiteral("</li><li>") +
     tr("F10 to maximize/demaximize package list view") +
  QStringLiteral("</li><li>") +
     tr("F11 to maximize/demaximize Tab's view") +
  QStringLiteral("</li></ul><br>");

  //html += "<br>" + strNoPassword;

  return html;
}

/*
 * Refreshes html text displayed on Help/Usage
 */
void MainWindow::refreshHelpUsageText()
{
  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_HELPUSAGE)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));
  if (!text) return;

  text->setText(generateHelpUsageHtmlText());
}

/*
 * Slot to position twProperties at Help tab
 */
void MainWindow::onHelpUsage()
{
  changeTabWidgetPropertiesIndex(ctn_TABINDEX_HELPUSAGE);
}

/*
 * Slot which opens Donate url
 */
void MainWindow::onHelpDonate()
{
  const QString url=QStringLiteral("http://sourceforge.net/donate/index.php?group_id=186459");
  QDesktopServices::openUrl(QUrl(url));
}

/*
 * Slot which opens the About dialog
 */
void MainWindow::onHelpAbout()
{
  QString aboutText =
      QLatin1String("<b>") + StrConstants::getApplicationName() + QLatin1String("</b><br>");

  aboutText += StrConstants::getVersion() + QLatin1String(": ") + ctn_APPLICATION_VERSION /*StrConstants::getApplicationVersion()*/ +
      QLatin1String(" - ") + StrConstants::getQtVersion() + QLatin1String("<br>");
  aboutText += StrConstants::getURL() + QLatin1String(": ") +
      QStringLiteral("<a href=\"https://tintaescura.com/projects/octopi/\">https://tintaescura.com/projects/octopi</a><br>");
  aboutText += StrConstants::getLicenses() + QLatin1String(": ") + QStringLiteral("<a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GPL v2</a><br>");
  aboutText += QLatin1String("&copy; Alexandre Albuquerque Arnt<br><br>");

  aboutText += QLatin1String("<b>Pacman</b><br>");
  QString pacmanV = UnixCommand::getPacmanVersion();
  if (pacmanV.at(0) == QLatin1Char('v')) pacmanV.remove(0, 1);
  aboutText += StrConstants::getVersion() + QLatin1String(": ") + pacmanV + QLatin1String("<br>");
  aboutText += StrConstants::getURL() + QLatin1String(": ") +
      QLatin1String("<a href=\"https://www.archlinux.org/pacman/\">https://www.archlinux.org/pacman</a><br>");
  QDate d = QDate::currentDate();
  aboutText += QLatin1String("&copy; 2006-%1 Pacman Development Team<br>");
  aboutText += QLatin1String("&copy; 2002-2006 Judd Vinet");
  aboutText = aboutText.arg(d.year());

  QMessageBox::about(this, StrConstants::getHelpAbout(), aboutText);
}
