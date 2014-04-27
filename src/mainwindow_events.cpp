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

#include <QCloseEvent>
#include <QMessageBox>
#include <QTreeView>
#include <QComboBox>
#include <QStandardItem>
#include <QTextBrowser>
#include <QFutureWatcher>

#if QT_VERSION > 0x050000
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
  else if(_isThereAPendingTransaction())
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
 * This Event method is called whenever the user presses a key
 */
void MainWindow::keyPressEvent(QKeyEvent* ke)
{
  if (ke->key() == Qt::Key_Return)
  {
    if (isYaourtGroupSelected() && m_leFilterPackage->hasFocus() && m_cic == NULL)
    {
      QFuture<QList<PackageListData> *> f;
      disconnect(&g_fwYaourt, SIGNAL(finished()), this, SLOT(preBuildYaourtPackageList()));
      m_cic = new CPUIntensiveComputing();
      f = QtConcurrent::run(searchYaourtPackages, m_leFilterPackage->text());
      g_fwYaourt.setFuture(f);
      connect(&g_fwYaourt, SIGNAL(finished()), this, SLOT(preBuildYaourtPackageList()));
    }
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
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
  }
  else if(ke->key() == Qt::Key_2 && ke->modifiers() == Qt::AltModifier)
  {
    ui->twProperties->setCurrentIndex(ctn_TABINDEX_FILES);
    refreshTabFiles(false, true);
  }
  else if(ke->key() == Qt::Key_3 && ke->modifiers() == Qt::AltModifier)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_TRANSACTION);
  }
  else if(ke->key() == Qt::Key_4 && ke->modifiers() == Qt::AltModifier)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_OUTPUT);
  }
  else if(ke->key() == Qt::Key_5 && ke->modifiers() == Qt::AltModifier)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_NEWS);
  }
  else if(ke->key() == Qt::Key_6 && ke->modifiers() == Qt::AltModifier)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_HELPUSAGE);
  }
  else if(ke->key() == Qt::Key_F4)
  {
    openTerminal();
  }
  /*else if(ke->key() == Qt::Key_F5)
  {
    invalidateTabs();
    metaBuildPackageList();
  }*/
  else if(ke->key() == Qt::Key_F6)
  {
    openDirectory();
  }
  else if (ke->key() == Qt::Key_F10)
  {
    maximizePackagesTreeView();
  }
  else if (ke->key() == Qt::Key_F12)
  {
    maximizePropertiesTabWidget();
  }
  else if(ke->key() == Qt::Key_L && ke->modifiers() == Qt::ControlModifier)
  {
    m_leFilterPackage->setFocus();
    m_leFilterPackage->selectAll();
  }
  else if(ke->key() == Qt::Key_F && ke->modifiers() == Qt::ControlModifier)
  {
    if (_isPropertiesTabWidgetVisible() &&
        (ui->twProperties->currentIndex() == ctn_TABINDEX_NEWS ||
         ui->twProperties->currentIndex() == ctn_TABINDEX_HELPUSAGE))
    {
      QTextBrowser *tb = ui->twProperties->currentWidget()->findChild<QTextBrowser*>("textBrowser");
      SearchBar *searchBar = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");

      if (tb && tb->toPlainText().size() > 0 && searchBar)
      {
        if (searchBar) searchBar->show();
      }
    }
    else if (_isPropertiesTabWidgetVisible() && ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
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
    _showPackagesWithNoDescription();
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
          && m_hasYaourt)
  {
    //The user wants to go to fake "Yaourt" group
    if (!isYaourtGroupSelected())
    {
      ui->twGroups->setCurrentItem(m_YaourtItem);
      //...and let us focus the search edit!
      m_leFilterPackage->setFocus();
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
}

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

      //std::cout << "CurrentIndex row: " << currentIndex.row() << std::endl;
      //std::cout << "LastIndex row: " << lastIndex.row() << std::endl;

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
