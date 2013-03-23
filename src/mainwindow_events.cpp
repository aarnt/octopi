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
 * This is MainWindow events code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "wmhelper.h"
#include "uihelper.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QTreeView>
#include <QComboBox>
#include <QStandardItem>
#include <QSortFilterProxyModel>

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
  }
}

/*
 * This Event method is called whenever the user presses a key
 */
void MainWindow::keyPressEvent(QKeyEvent* ke)
{
  if (ke->key() == Qt::Key_Return)
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
  else if(ke->key() == Qt::Key_Escape)
  {
    if(ui->leFilterPackage->hasFocus())
    {
      ui->leFilterPackage->clear();
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
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
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
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_HELPABOUT);
  }
  else if(ke->key() == Qt::Key_F4)
  {
    openTerminal();
  }
  else if(ke->key() == Qt::Key_F5)
  {
    invalidateTabs();
    metaBuildPackageList();
  }
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
    ui->leFilterPackage->setFocus();
    ui->leFilterPackage->selectAll();
  }
  else if(ke->key() == Qt::Key_G && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    //The user wants to go to "Display All groups"
    if (m_cbGroups->currentIndex() != 0)
    {
      m_cbGroups->setCurrentIndex(0);
    }
  }
  else if(ke->key() == Qt::Key_C && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    doCleanCache();
  }
}

/*
 * This Event method is called whenever the user releases a key
 * (useful to navigate in the packagelist)
 */
void MainWindow::keyReleaseEvent(QKeyEvent *ke)
{
  static int i=0;
  static int k=-9999;
  static int k_count=0;
  QStandardItemModel *sim;

  if ((ui->tvPackages->hasFocus()) &&
      (((ke->key() >= Qt::Key_A) && (ke->key() <= Qt::Key_Z)) ||
       ((ke->key() >= Qt::Key_0 && (ke->key() <= Qt::Key_9)))))
  {
    //Which model of tbPackage is being used?
    if(ui->actionNonInstalledPackages->isChecked())
    {
      if(m_cbGroups->currentIndex() == 0)
        sim = m_modelPackages;
      else
        sim = m_modelPackagesFromGroup;
    }
    else
    {
      if(m_cbGroups->currentIndex() == 0)
        sim = m_modelInstalledPackages;
      else
        sim = m_modelInstalledPackagesFromGroup;
    }

    QList<QStandardItem*> fi = sim->findItems( ke->text(), Qt::MatchStartsWith, ctn_PACKAGE_NAME_COLUMN );
    if (fi.count() > 0){
      if ( (ke->key() != k) || (fi.count() != k_count) ) i=0;

      foreach (QStandardItem* si, fi){
        QModelIndex mi = si->index();
        mi = m_proxyModelPackages->mapFromSource(mi);
        if (!m_proxyModelPackages->hasIndex(mi.row(), mi.column())) fi.removeAll(si);
      }

      if (fi.count()==0)
      {
        return;
      }

      /*QList<SelectedPackage> sp = getSelectedPackage();
      if (sp.count() == 1) {
        int a=0;
        while (a<=fi.count()-1) {
          QStandardItem* si = fi[a];
          if (si->text() == sp[sp.count()-1].getFileName()) break;
          a++;
          i=a;
        }
        if((a+1)<=fi.count()-1) { a++; i=a; }
        else { a=i=0; }
      }*/

      ui->tvPackages->selectionModel()->clear();
      QModelIndex mi = fi[i]->index();
      mi = m_proxyModelPackages->mapFromSource(mi);
      ui->tvPackages->scrollTo(mi);

      QModelIndex maux = m_proxyModelPackages->index( mi.row(), ctn_PACKAGE_ICON_COLUMN );
      ui->tvPackages->selectionModel()->setCurrentIndex(mi, QItemSelectionModel::Select);
      ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);
      ui->tvPackages->setCurrentIndex(mi);

      if ((i <= fi.count()-1)) i++;
      if (i == fi.count()) i = 0;
    }

    k = ke->key();
    k_count = fi.count();
  }

  else ke->ignore();
}
