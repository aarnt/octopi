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

/*
 * Initialize the Help tab with basic information about using Octopi
 */
void MainWindow::initTabHelpUsage()
{
  QWidget *tabHelpUsage = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabHelpUsage);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setMargin(0);

  QTextBrowser *text = new QTextBrowser(tabHelpUsage);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget(text, 0, 0, 1, 1);

  QString iconPath = "<img height=\"16\" width=\"16\" src=\":/resources/images/";

  QString strForMoreInfo = tr("For more information, visit:");
  QString html =
    QString("<h2>Octopi</h2>") +
    QString("<h3><p>") + tr("A Qt4-based Pacman frontend,") + " " +
    tr("licensed under the terms of") + " ";

  if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX &&
      (!WMHelper::isKDERunning() && (!WMHelper::isRazorQtRunning())))
  {
    html +=
        QString("<a style=\"color:'#4BC413'\" href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GPL v2</a>.</p></h3>") +
      QString("<h4><p>") + strForMoreInfo + " " +
      QString("<a style=\"color:'#4BC413'\" href=\"http://octopiproject.wordpress.com\">http://octopiproject.wordpress.com</a>.</p></h4><br>");
  }
  else
  {
    html +=
        QString("<a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GPL v2</a>.</p></h3>") +
      QString("<h4><p>") + strForMoreInfo + " " +
      QString("<a href=\"http://octopiproject.wordpress.com\">http://octopiproject.wordpress.com</a>.</p></h4><br>");
  }

    html += tr("Package classification:") +

  QString("<ul type=\"square\"><li>") + iconPath + "installed.png\"/> " +
     tr("An installed package") + QString("</li>") +
  QString("<li>") + iconPath + "unrequired.png\"/> " +
     tr("An installed package (not required by others)") +
  QString("</li>") +
  QString("<li>" + iconPath + "foreign_green.png\"/> ") +
     tr("A foreign package, installed from") + " " + StrConstants::getForeignRepositoryName() +
  QString("</li>") +
  QString("<li>") + iconPath + "noninstalled.png\"/> " +
     tr("A non installed package") +
  QString("</li>") +
  QString("<li>") + iconPath + "outdated.png\"/> " +
     tr("An outdated package") +
  QString("</li>") +
  QString("<li>") + iconPath + "foreign_red.png\"/> " +
     tr("An outdated foreign package") +
  QString("</li>") +
  QString("<li>") + iconPath + "newer.png\"/> " +
           tr("A newer than repository package") +
  QString("</li></ul>") +

     tr("Basic usage help:") +
  QString("<ul><li>") +
     tr("Position the mouse over a package to see its description") +
  QString("</li><li>") +
     tr("Double click an installed package to see its contents") +
  QString("</li><li>") +
     tr("Right click package to install/reinstall or remove it") +
  QString("</li></ul>") +

     tr("Alt+key sequences:") +
  QString("<ul><li>") +
     tr("Alt+1 to switch to 'Info' tab") +
  QString("</li><li>") +
     tr("Alt+2 to switch to 'Files' tab") +
  QString("</li><li>") +
     tr("Alt+3 to switch to 'Transaction' tab") +
  QString("</li><li>") +
     tr("Alt+4 to switch to 'Output' tab") +
  QString("</li><li>") +
     tr("Alt+5 to switch to 'News' tab") +
  QString("</li><li>") +
     tr("Alt+6 or 'F1' to show this help page") +
  QString("</li></ul>") +

     tr("Control+key sequences:") +
  QString("<ul><li>") +
     tr("Ctrl+D or 'File/Sync database' to sync the local database with latest remote changes (pacman -Sy)") +
  QString("</li><li>") +
     tr("Ctrl+U or 'File/System upgrade' to make a full system upgrade (pacman -Su)") +
  QString("</li><li>") +
     tr("Ctrl+L to find a package in the package list") +
  QString("</li><li>") +
     tr("Ctrl+F to search for text inside tab Files, News and Usage") +
  QString("</li><li>") +
     tr("Ctrl+N or 'View/Non installed' to show/hide non installed packages") +
  QString("</li><li>") +
     tr("Ctrl+M or 'Transaction/Commit' to start installation/removal of selected packages") +
  QString("</li><li>") +
     tr("Ctrl+B or 'Transaction/Rollback' to clear the selection of to be removed/installed packages") +
  QString("</li><li>") +
     tr("Ctrl+G or 'File/Get latest distro news' to retrieve the latest RSS based distro news") +
  QString("</li><li>") +
     tr("Ctrl+Q or 'File/Exit' to exit the application") +
  QString("</li></ul>") +

     tr("Control+shift+key sequences:") +
  QString("<ul><li>") +
     tr("Ctrl+Shift+C to clean local packages cache (pacman -Sc)") +
  QString("</li><li>") +
     tr("Ctrl+Shift+G to display all package groups") +
  QString("</li><li>") +
     tr("Ctrl+Shift+R to remove Pacman's transaction lock file") +
  QString("</li><li>") +
     tr("Ctrl+Shift+Y to display Yaourt group") +
  QString("</li></ul>") +

     tr("F+key sequences:") +
  QString("<ul><li>") +
     tr("F1 to show this help page") +
  QString("</li><li>") +
     tr("F4 to open a Terminal whitin the selected directory at Files tab") +
  QString("</li><li>") +
     tr("F6 to open a File Manager whitin the selected directory at Files tab") +
  QString("</li><li>") +
     tr("F10 to maximize/demaximize package list view") +
  QString("</li><li>") +
     tr("F12 to maximize/demaximize Tab's view") +
  QString("</li></ul>");

  text->setText(html);
  int tindex = ui->twProperties->addTab(tabHelpUsage, StrConstants::getHelpUsage() );
  ui->twProperties->setTabText(ui->twProperties->indexOf(tabHelpUsage), StrConstants::getHelpUsage());

  SearchBar *searchBar = new SearchBar(this);
  MyHighlighter *highlighter = new MyHighlighter(text, "");
  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPrevious()));
  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);
  gridLayoutX->addWidget(new SyntaxHighlighterWidget(this, highlighter));

  text->show();
  ui->twProperties->setCurrentIndex(tindex);
  text->setFocus();
}

/*
 * Slot to position twProperties at Help tab
 */
void MainWindow::onHelpUsage()
{
  _changeTabWidgetPropertiesIndex(ctn_TABINDEX_HELPUSAGE);
}

/*
 * Slot which opens the About dialog
 */
void MainWindow::onHelpAbout()
{
  QString aboutText = "<b>" + StrConstants::getApplicationName() +
      " - " + StrConstants::getApplicationVersion() + "</b><br>";
  aboutText += "<a href=\"http://octopiproject.wordpress.com/\">http://octopiproject.wordpress.com</a><br><br>";
  aboutText += "&copy; Alexandre Albuquerque Arnt";
  QMessageBox::about(this, StrConstants::getHelpAbout(), aboutText);
}
