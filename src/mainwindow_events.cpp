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

/*
 * This is MainWindow's event related code
 */

#include "ui_mainwindow.h"
#include "searchlineedit.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "wmhelper.h"
#include "uihelper.h"
#include "searchbar.h"
#include "globals.h"
#include "terminal.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QTreeView>
#include <QComboBox>
#include <QStandardItem>
#include <QTextBrowser>
#include <QFutureWatcher>
#include <QClipboard>
#include <QtConcurrent/QtConcurrentRun>

/*
 * Before we close the application, let's confirm if there is a pending transaction...
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
  //We cannot quit while there is a running transaction!
  if(m_commandExecuting != ectn_NONE)
  {
    QMessageBox::information(this, StrConstants::getAttention(),
                             StrConstants::getThereIsAPendingTransaction(),
                             QMessageBox::Ok, QMessageBox::Ok);
    event->ignore();
  }
  else if(isThereAPendingTransaction())
  {
    int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                    StrConstants::getThereIsAPendingTransaction() + "\n" +
                                    StrConstants::getDoYouReallyWantToQuit(),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
      QByteArray windowSize=saveGeometry();
      SettingsManager::setWindowSize(windowSize);
      SettingsManager::setSplitterHorizontalState(ui->splitterHorizontal->saveState());
      event->accept();
      qApp->quit();
    }
    else
    {
      event->ignore();
    }
  }
  else
  {
    QByteArray windowSize=saveGeometry();
    SettingsManager::setWindowSize(windowSize);
    SettingsManager::setSplitterHorizontalState(ui->splitterHorizontal->saveState());
    event->accept();
    qApp->quit();
  }
}

/*
 * Copies the full path of the selected item in pkgFileListTreeView to clipboard
 */
void MainWindow::copyFullPathToClipboard()
{
  QTreeView *tb = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  if (tb && tb->hasFocus())
  {
    QString path = utils::showFullPathOfItem(tb->currentIndex());
    QClipboard *clip = qApp->clipboard();
    clip->setText(path);
  }
}

/*
 * This Event method is called whenever the user presses a key
 */
void MainWindow::keyPressEvent(QKeyEvent* ke)
{
  if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
  {
    if (ui->tvPackages->hasFocus())
    {
      execKeyActionOnPackage(ectn_INSTALL);
    }
    //We are searching for AUR foreign packages...
    else if (isAURGroupSelected() && m_leFilterPackage->hasFocus() && m_cic == NULL)
    {
      if (UnixCommand::getLinuxDistro() == ectn_KAOS) return;

      ui->twGroups->setEnabled(false);

      QFuture<QList<PackageListData> *> f;
      disconnect(&g_fwAUR, SIGNAL(finished()), this, SLOT(preBuildAURPackageList()));
      m_cic = new CPUIntensiveComputing();
      f = QtConcurrent::run(searchAURPackages, m_leFilterPackage->text());
      g_fwAUR.setFuture(f);
      connect(&g_fwAUR, SIGNAL(finished()), this, SLOT(preBuildAURPackageList()));
    }
    //We are searching for packages that own some file typed by user...
    else if (isSearchByFileSelected() && m_leFilterPackage->hasFocus() && m_cic == NULL)
    {
      ui->twGroups->setEnabled(false);

      QFuture<QString> f;
      disconnect(&g_fwPackageOwnsFile, SIGNAL(finished()), this, SLOT(positionInPkgListSearchByFile()));
      m_cic = new CPUIntensiveComputing();
      f = QtConcurrent::run(searchPacmanPackagesByFile, m_leFilterPackage->text());
      g_fwPackageOwnsFile.setFuture(f);
      connect(&g_fwPackageOwnsFile, SIGNAL(finished()), this, SLOT(positionInPkgListSearchByFile()));
    }
    //We are probably inside 'Files' tab...
    else
    {
      QTreeView *tvPkgFileList =
          ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");

      if(tvPkgFileList)
      {
        if(tvPkgFileList->hasFocus())
        {
          openFile();
        }
      }
    }
  }
  else if(ke->key() == Qt::Key_Escape)
  {
    if(m_leFilterPackage->hasFocus())
    {
      m_leFilterPackage->clear();
    }
  }
  else if(ke->key() == Qt::Key_Delete)
  {
    onPressDelete();
  }    
  else if(ke->key() == Qt::Key_1 && ke->modifiers() == Qt::AltModifier)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
  }
  else if(ke->key() == Qt::Key_2 && ke->modifiers() == Qt::AltModifier)
  {
    ui->twProperties->setCurrentIndex(ctn_TABINDEX_FILES);
    refreshTabFiles(false, true);
  }
  else if(ke->key() == Qt::Key_3 && ke->modifiers() == Qt::AltModifier)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_TRANSACTION);
  }
  else if(ke->key() == Qt::Key_4 && ke->modifiers() == Qt::AltModifier)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_OUTPUT);
  }
  else if(ke->key() == Qt::Key_5 && ke->modifiers() == Qt::AltModifier)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_NEWS);
  }
  else if(ke->key() == Qt::Key_6 && ke->modifiers() == Qt::AltModifier)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_HELPUSAGE);
  }
  else if(ke->key() == Qt::Key_F2)
  {
    if (isPackageTreeViewVisible())
    {
      if (!ui->tvPackages->hasFocus()) ui->tvPackages->setFocus();
    }
    else
    {
      maximizePropertiesTabWidget(false);
      if (!ui->tvPackages->hasFocus()) ui->tvPackages->setFocus();
    }
  }
  else if(ke->key() == Qt::Key_F4)
  {
    openTerminal();
  }
  else if(ke->key() == Qt::Key_F6)
  {
    openDirectory();
  }
  else if (ke->key() == Qt::Key_F10)
  {
    maximizePackagesTreeView(false);
  }
  else if (ke->key() == Qt::Key_F11)
  {
    maximizePropertiesTabWidget(false);
  }
  else if(ke->key() == Qt::Key_C && ke->modifiers() == Qt::ControlModifier)
  {
    copyFullPathToClipboard();
  }
  else if(ke->key() == Qt::Key_L && ke->modifiers() == Qt::ControlModifier)
  {
    m_leFilterPackage->setFocus();
    m_leFilterPackage->selectAll();
  }
  else if(ke->key() == Qt::Key_F && ke->modifiers() == Qt::ControlModifier)
  {
    if (m_commandExecuting != ectn_NONE) return;

    if (isPropertiesTabWidgetVisible() &&
        (ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION ||
         ui->twProperties->currentIndex() == ctn_TABINDEX_OUTPUT ||
         ui->twProperties->currentIndex() == ctn_TABINDEX_NEWS ||
         ui->twProperties->currentIndex() == ctn_TABINDEX_HELPUSAGE))
    {
      QTextBrowser *tb = ui->twProperties->currentWidget()->findChild<QTextBrowser*>("textBrowser");
      SearchBar *searchBar = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");

      if (tb && tb->toPlainText().size() > 0 && searchBar)
      {
        if (searchBar) searchBar->show();
      }
    }
    else if (isPropertiesTabWidgetVisible() && ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    {
      QTreeView *tb = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
      SearchBar *searchBar = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");

      if (tb && tb->model()->rowCount() > 0 && searchBar)
      {
        if (searchBar) searchBar->show();
      }
    }
  }
  else if(ke->key() == Qt::Key_Z && ke->modifiers() == Qt::ControlModifier)
  {
    if (m_commandExecuting != ectn_NONE && m_pacmanExec != NULL)
    {
      m_pacmanExec->cancelProcess();
    }
  }
  else if(ke->key() == Qt::Key_D && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    //The user wants to know which packages have no description!
    showPackagesWithNoDescription();
  }
  else if(ke->key() == Qt::Key_G && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    //The user wants to go to "Display All groups"
    if (!isAllGroupsSelected())
    {
      ui->twGroups->setCurrentItem(m_AllGroupsItem);
    }
  }
  else if(ke->key() == Qt::Key_Y && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier)
          && m_hasAURTool)
  {
    if (m_commandExecuting != ectn_NONE) return;

    //The user wants to use "AUR tool" to search for pkgs
    m_actionSwitchToAURTool->trigger();
    if (m_actionSwitchToAURTool->isChecked() && UnixCommand::getLinuxDistro() != ectn_KAOS)
    {
      m_leFilterPackage->setFocus();
    }
    else
    {
      ui->tvPackages->setFocus();
    }
  }
  else if(ke->key() == Qt::Key_C && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    if (m_commandExecuting == ectn_NONE)
    {
      doCleanCache(); //If we are not executing any command, let's clean the cache
    }
  }
  else if(ke->key() == Qt::Key_R && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    if (m_commandExecuting == ectn_NONE)
    {
      doRemovePacmanLockFile(); //If we are not executing any command, let's remove Pacman's lock file
    }
  } 
  else if(ke->key() == Qt::Key_S && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    gistSysInfo();
  }

  else ke->ignore();
}

/*
 * This Event method is called whenever the user releases a key
 */
void MainWindow::keyReleaseEvent(QKeyEvent* ke)
{
  if ((ui->tvPackages->hasFocus()) && (
      ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down ||
      ke->key() == Qt::Key_Home || ke->key() == Qt::Key_End ||
      ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown))
  {
    if (ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION)
    {
      refreshTabInfo(false, true);
      ui->tvPackages->setFocus();
    }
  }
  else if(ke->key() == Qt::Key_Home && ke->modifiers() == Qt::AltModifier)
  {
    m_indOfVisitedPackage = 0;

    if (!m_listOfVisitedPackages.isEmpty())
      positionInPackageList(m_listOfVisitedPackages.at(m_indOfVisitedPackage));
  }
  else if(ke->key() == Qt::Key_Left && ke->modifiers() == Qt::AltModifier)
  {
    if (m_indOfVisitedPackage > 0)
    {
      --m_indOfVisitedPackage;
    }

    if (!m_listOfVisitedPackages.isEmpty())
      positionInPackageList(m_listOfVisitedPackages.at(m_indOfVisitedPackage));
  }
  else if(ke->key() == Qt::Key_Right && ke->modifiers() == Qt::AltModifier)
  {
    if (m_indOfVisitedPackage < (m_listOfVisitedPackages.count()-1))
    {
      ++m_indOfVisitedPackage;
    }

    if (!m_listOfVisitedPackages.isEmpty())
      positionInPackageList(m_listOfVisitedPackages.at(m_indOfVisitedPackage));
  }
  else if(ke->key() == Qt::Key_End && ke->modifiers() == Qt::AltModifier)
  {
    m_indOfVisitedPackage = m_listOfVisitedPackages.count()-1;

    if (!m_listOfVisitedPackages.isEmpty())
      positionInPackageList(m_listOfVisitedPackages.at(m_indOfVisitedPackage));
  }
}
