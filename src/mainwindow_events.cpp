/*
* This file is part of Octopi, an open-source GUI for ArchLinux pacman.
* Copyright (C) 2013  Alexandre Albuquerque Arnt
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
 * This is a MainWindows' events code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "wmhelper.h"
#include "uihelper.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QTreeView>

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
      event->accept();
    }
    else
    {
      event->ignore();
    }
  }
  else
  {
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
        openFile(tvPkgFileList->currentIndex());
    } }
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
  else if(ke->key() == Qt::Key_F4)
  {
    QString dir = getSelectedDirectory();
    if (!dir.isEmpty())
      WMHelper::openTerminal(dir);
  }
  else if(ke->key() == Qt::Key_F5)
  {
    invalidateTabs();
    buildPackageList();
  }
  else if(ke->key() == Qt::Key_F6)
  {
    QString dir = getSelectedDirectory();
    if (!dir.isEmpty())
      WMHelper::openDirectory(dir);
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
  else if(ke->key() == Qt::Key_C && ke->modifiers() == (Qt::ShiftModifier|Qt::ControlModifier))
  {
    CPUIntensiveComputing cic;
    UnixCommand::cleanPacmanCache();
  }
}
