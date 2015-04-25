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

#if QT_VERSION >= 0x050300
  #include "terminalselectordialog.h"
#endif

#if QT_VERSION >= 0x050000
  #include <QtConcurrent/QtConcurrentRun>  
#else
  #include <QtConcurrentRun>
#endif

#if QT_VERSION < 0x050000
  using namespace QtConcurrent;
#endif

/*
 * Before we close the application, let's confirm if there is a pending transaction...
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
  //We cannot quit while there is a running transaction!
  if(m_commandExecuting != ectn_NONE)
  {
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
    //We are searching for AUR foreign packages...
    if (isAURGroupSelected() && m_leFilterPackage->hasFocus() && m_cic == NULL)
    {
      if (UnixCommand::getLinuxDistro() == ectn_KAOS)
        return;

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
  else if (ke->key() == Qt::Key_F12)
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
    if (isPropertiesTabWidgetVisible() &&
        (ui->twProperties->currentIndex() == ctn_TABINDEX_OUTPUT ||
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
    //The user wants to use "AUR tool" to search for pkgs
    m_actionSwitchToAURTool->trigger();
    m_leFilterPackage->setFocus();
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

  #if QT_VERSION >= 0x050300
  else if(ke->key() == Qt::Key_T && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier)
          && m_initializationCompleted)
  {
    QStringList terminals = Terminal::getListOfAvailableTerminals();

    if (terminals.count() > 2)
    {
      int index = terminals.indexOf(SettingsManager::getTerminal());
      int newIndex = selectTerminal(index);

      if (index != newIndex)
      {
        SettingsManager::setTerminal(terminals.at(newIndex));
      }
    }
  }
  #endif

  else ke->ignore();
}

/*
 * Calls TerminalSelectorDialog to let user chooses which terminal to use with Octopi
 */
#if QT_VERSION >= 0x050300
int MainWindow::selectTerminal(const int initialTerminalIndex)
{
  int result = initialTerminalIndex;
  std::unique_ptr<TerminalSelectorDialog> d(
        new TerminalSelectorDialog(this, Terminal::getListOfAvailableTerminals()));

  d->setInitialTerminalIndex(initialTerminalIndex);

  if (d->exec() == QDialog::Accepted)
  {
    result = d->selectedTerminalIndex();
  }
  else
  {
    result = initialTerminalIndex;
  }

  return result;
}
#endif

//If we are using Qt5 libs, this method is native !
#if QT_VERSION < 0x050000
/*
 * This Event method is called whenever the user releases a key (useful to navigate in the packagelist)
 */
void MainWindow::keyReleaseEvent(QKeyEvent *ke)
{  
  static int i=0;
  static int k=-9999; //last key pressed
  static int k_count=0;

  if ((ui->tvPackages->hasFocus() && ke->modifiers() == Qt::NoModifier) &&
      (((ke->key() >= Qt::Key_A) && (ke->key() <= Qt::Key_Z)) ||
       ((ke->key() >= Qt::Key_0 && (ke->key() <= Qt::Key_9)))))
  {
    QModelIndex searchColumn = m_packageModel->index(0,
                                                     PackageModel::ctn_PACKAGE_NAME_COLUMN,
                                                     QModelIndex());
    QModelIndexList fi = m_packageModel->match(searchColumn, Qt::DisplayRole,
                                               ke->text(), -1);

    if (fi.count() > 0) {
      if ( (ke->key() != k) || (fi.count() != k_count) ) i=0;

      QModelIndex currentIndex = ui->tvPackages->currentIndex();
      QModelIndex firstIndex = fi.first();
      QModelIndex lastIndex = fi.last();

      if (currentIndex.row() < firstIndex.row() || currentIndex.row() > lastIndex.row())
      {
        i=0;
      }
      else
      {
        for(int ind=0; ind<fi.count(); ind++)
        {
          QModelIndex miAux = fi.at(ind);

          if(miAux == ui->tvPackages->currentIndex() )
          {
            int newIndex = ind+1;
            if(newIndex > fi.count()-1)
            {
              i=0;
            }
            else
            {
              i=newIndex;
            }
          }
        }
      }

      if (ui->tvPackages->selectionModel() != NULL) {
        ui->tvPackages->selectionModel()->clear();
        QModelIndex mi = fi[i];
        ui->tvPackages->scrollTo(mi);
        ui->tvPackages->selectionModel()->setCurrentIndex(mi, QItemSelectionModel::Select);
        ui->tvPackages->setCurrentIndex(mi);
      }

      //If we happen to be over the last package of the list...
      if (currentIndex.row() == lastIndex.row()-1)
      {
        i=0;
      }
    }

    k = ke->key();
    k_count = fi.count();
  }

  else ke->ignore();
}
#else
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
}
#endif
