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
#include "uihelper.h"
#include "searchbar.h"
#include "globals.h"
#include "settingsmanager.h"
#include "utils.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QTreeView>
#include <QComboBox>
#include <QStandardItem>
#include <QTextBrowser>
#include <QFutureWatcher>
#include <QClipboard>
#include <QProgressBar>
#include <QtConcurrent/QtConcurrentRun>
#include <QSharedMemory>
#include <QTimer>

/*
 * Before we close the application, let's confirm if there is a pending transaction...
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
  //We cannot quit while there is a running transaction!
  if(m_commandExecuting != ectn_NONE)
  {
    int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                          StrConstants::getThereIsARunningTransaction() + QLatin1Char('\n') +
                          StrConstants::getDoYouReallyWantToQuit(),
                          QMessageBox::Yes | QMessageBox::No,
                          QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
      event->accept();
      qApp->quit();
    }
    else
    {
      event->ignore();
    }
  }
  else if(areTherePendingActions())
  {
    int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                    StrConstants::getThereArePendingActions() + QLatin1Char('\n') +
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
  QTreeView *tb = ui->twProperties->getTvPkgFileList();
  if (tb && tb->hasFocus())
  {
    QString path = utils::showFullPathOfItem(tb->currentIndex());
    QClipboard *clip = qApp->clipboard();
    clip->setText(path);
  }
}

/*
 * THIS IS JUST A TEST CODE!
 */
void testSharedMem()
{
  QSharedMemory *sharedMem=new QSharedMemory(QStringLiteral("org.arnt.test"));
  QByteArray sharedData="abracadabra -abcd\ncadabraabra -xyz";

  sharedMem->create(sharedData.size());
  sharedMem->lock();
  memcpy(sharedMem->data(), sharedData.data(), sharedData.size());
  sharedMem->unlock();

  //Let's retrieve commands from sharedmem pool
  QSharedMemory *sharedMem2 = new QSharedMemory(QStringLiteral("org.arnt.test"));
  if (!sharedMem2->attach(QSharedMemory::ReadOnly))
  {
    QTextStream qout(stdout);
    qout << Qt::endl << "ERROR: Couldn't attach to memory" << Qt::endl;
  }

  QByteArray sharedData2(sharedMem2->size(), '\0');
  sharedMem2->lock();
  memcpy(sharedData2.data(), sharedMem2->data(), sharedMem2->size());
  sharedMem2->unlock();
  QString contents=QString::fromLatin1(sharedData2);
  QTextStream qout(stdout);
  qout << Qt::endl << "What is in memory: " << contents << Qt::endl;

  sharedMem2->detach();
  delete sharedMem2;
  delete sharedMem;
}

/*
 * This Event method is called whenever the user presses a key
 */
void MainWindow::keyPressEvent(QKeyEvent* ke)
{
  if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
  {
    if (m_commandExecuting != ectn_NONE) return;

    if (ui->tvPackages->hasFocus())
    {
      execKeyActionOnPackage(ectn_INSTALL);
    }
    else if (!isSearchByFileSelected() && !isAURGroupSelected() && m_leFilterPackage->hasFocus() && !ui->actionUseInstantSearch->isChecked())
    {
      reapplyPackageFilter();
    }
    //We are searching for AUR foreign packages...
    else if (isAURGroupSelected() && m_leFilterPackage->hasFocus() && m_cic == nullptr)
    {
      if (UnixCommand::getLinuxDistro() == ectn_KAOS && ui->actionUseInstantSearch->isChecked()) return;

      if (!isInternetAvailable()) return;

      m_progressWidget->setMaximum(100);
      m_progressWidget->setValue(0);
      m_progressWidget->show();

      ui->twGroups->setEnabled(false);

      QFuture<QList<PackageListData> *> f;
      disconnect(&g_fwAUR, SIGNAL(finished()), this, SLOT(preBuildAURPackageList()));
      m_cic = new CPUIntensiveComputing();
      f = QtConcurrent::run(searchForeignToolPackages, m_leFilterPackage->text());
      g_fwAUR.setFuture(f);
      connect(&g_fwAUR, SIGNAL(finished()), this, SLOT(preBuildAURPackageList()));
    }
    //We are searching for packages that own some file typed by user...
    else if (isSearchByFileSelected() && m_leFilterPackage->hasFocus() && m_cic == nullptr)
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
          ui->twProperties->getTvPkgFileList();

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
    if (m_commandExecuting != ectn_NONE) return;

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
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_ACTIONS);
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
  else if(ke->key() == Qt::Key_7 && ke->modifiers() == Qt::AltModifier)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_TERMINAL);
  }
  else if(ke->key() == Qt::Key_F2)
  {
    if (isPackageTreeViewVisible())
    {
      if (!ui->tvPackages->hasFocus()) ui->tvPackages->setFocus();
    }
    else
    {
      maxDemaxPropertiesTabWidget(false);
      if (!ui->tvPackages->hasFocus()) ui->tvPackages->setFocus();
    }
  }
  else if(ke->key() == Qt::Key_F4)
  {
    openTerminal();
  }
  else if(ke->key() == Qt::Key_F5)
  {
    if (m_commandExecuting != ectn_NONE) return;

    metaBuildPackageList();
  }
  else if(ke->key() == Qt::Key_F6)
  {
    openDirectory();
  }
  else if (ke->key() == Qt::Key_F10)
  {
    maxDemaxPackagesTreeView(true);
  }
  else if (ke->key() == Qt::Key_F11)
  {
    maxDemaxPropertiesTabWidget(true);
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
      QTextBrowser *tb = ui->twProperties->currentWidget()->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));
      SearchBar *searchBar = ui->twProperties->currentWidget()->findChild<SearchBar*>(QStringLiteral("searchbar"));

      if (tb && tb->toPlainText().size() > 0 && searchBar)
      {
        if (searchBar) searchBar->show();
      }
    }
    else if (isPropertiesTabWidgetVisible() && ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    {
      QTreeView *tb = ui->twProperties->getTvPkgFileList();
      SearchBar *searchBar = ui->twProperties->currentWidget()->findChild<SearchBar*>(QStringLiteral("searchbar"));

      if (tb && tb->model()->rowCount() > 0 && searchBar)
      {
        if (searchBar) searchBar->show();
      }
    }
  }
  else if (ke->key() == Qt::Key_P && ke->modifiers() == Qt::ControlModifier)
  {
    if (!ui->tvPackages->hasFocus())
      ui->tvPackages->setFocus();
  }
  else if (ke->key() == Qt::Key_U && ke->modifiers() == Qt::ControlModifier)
  {
    if (m_commandExecuting != ectn_NONE) return;

    if (ui->actionSystemUpgrade->isEnabled()) doSystemUpgrade();
  }
  else if(ke->key() == Qt::Key_D && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    if (m_commandExecuting != ectn_NONE) return;

    //The user wants to know which packages have no description!
    showPackagesWithNoDescription();
  }
  else if(ke->key() == Qt::Key_G && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    if (m_commandExecuting != ectn_NONE) return;

    //The user wants to go to "Display All groups"
    if (!isAllGroupsSelected())
    {
      ui->twGroups->setCurrentItem(m_AllGroupsItem);
    }
  }
  else if(ke->key() == Qt::Key_Y && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier)
          && m_hasForeignTool && m_actionSwitchToForeignTool->isEnabled())
  {
    if (m_commandExecuting != ectn_NONE) return;

    //The user wants to use "AUR tool" to search for pkgs
    m_actionSwitchToForeignTool->trigger();
    m_leFilterPackage->setFocus();
  }
  else if(ke->key() == Qt::Key_S && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    if (m_commandExecuting != ectn_NONE) return;

    doSysInfo();
  }
  else if(ke->key() == Qt::Key_A && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    if (m_commandExecuting != ectn_NONE) return;

    outputAURVotedPackageList();
  }
  else if(ke->key() == Qt::Key_O && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    if (m_commandExecuting != ectn_NONE) return;

    LinuxDistro ld=UnixCommand::getLinuxDistro();
    if((ld == ectn_KAOS || ld == ectn_CHAKRA) || (
       m_hasForeignTool && SettingsManager::getAURToolName() != ctn_NO_AUR_TOOL &&
       !isAURGroupSelected() /*&& !SettingsManager::getSearchOutdatedAURPackages()*/))
    {
      m_outdatedAURTimer->start();

      QFuture<FTOutdatedPackages *> f;
      f = QtConcurrent::run(getOutdatedForeignToolPackages);
      g_fwOutdatedAURPackages.setFuture(f);
      connect(&g_fwOutdatedAURPackages, SIGNAL(finished()), this, SLOT(showToolButtonAUR()));
    }
  }
  /*else if(ke->key() == Qt::Key_Z && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    //TEST CODE
  }*/

  else ke->ignore();
}

/*
 * This Event method is called whenever the user releases a key
 */
void MainWindow::keyReleaseEvent(QKeyEvent* ke)
{
  if(ui->tvPackages->hasFocus() && (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down ||
                                    ke->key() == Qt::Key_Home || ke->key() == Qt::Key_End ||
                                    ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown))
  {
    clearTabsInfoOrFiles();
    ui->tvPackages->setFocus();
  }
  else if (ui->tvPackages->hasFocus() && ke->key() == Qt::Key_Space)
  {
    invalidateTabs();
    ui->tvPackages->setFocus();
  }
  else if(ke->key() == Qt::Key_Home && ke->modifiers() == Qt::AltModifier)
  {
    if (!m_leFilterPackage->text().isEmpty()) m_leFilterPackage->clear();
    m_indOfVisitedPackage = 0;

    if (!m_listOfVisitedPackages.isEmpty())
      positionInPackageList(m_listOfVisitedPackages.at(m_indOfVisitedPackage));
  }
  else if(ke->key() == Qt::Key_Left && ke->modifiers() == Qt::AltModifier)
  {
    if (!m_leFilterPackage->text().isEmpty()) m_leFilterPackage->clear();
    if (m_indOfVisitedPackage > 0)
    {
      --m_indOfVisitedPackage;
    }

    if (!m_listOfVisitedPackages.isEmpty())
      positionInPackageList(m_listOfVisitedPackages.at(m_indOfVisitedPackage));
  }
  else if(ke->key() == Qt::Key_Right && ke->modifiers() == Qt::AltModifier)
  {
    if (!m_leFilterPackage->text().isEmpty()) m_leFilterPackage->clear();
    if (m_indOfVisitedPackage < (m_listOfVisitedPackages.count()-1))
    {
      ++m_indOfVisitedPackage;
    }

    if (!m_listOfVisitedPackages.isEmpty())
      positionInPackageList(m_listOfVisitedPackages.at(m_indOfVisitedPackage));
  }
  else if(ke->key() == Qt::Key_End && ke->modifiers() == Qt::AltModifier)
  {
    if (!m_leFilterPackage->text().isEmpty()) m_leFilterPackage->clear();
    m_indOfVisitedPackage = m_listOfVisitedPackages.count()-1;

    if (!m_listOfVisitedPackages.isEmpty())
      positionInPackageList(m_listOfVisitedPackages.at(m_indOfVisitedPackage));
  }
}
