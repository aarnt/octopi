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
 * This is a MainWindow's Transaction related code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "uihelper.h"
#include "wmhelper.h"
#include "strconstants.h"
#include <QMessageBox>
#include <QStandardItem>
#include <QSortFilterProxyModel>

/*
 * Watches the state of tvTransaction treeview to see if Commit/Rollback actions must be activated/deactivated
 */
void MainWindow::changeTransactionActionsState()
{
  bool state = _isThereAPendingTransaction();

  ui->actionCommit->setEnabled(state);
  ui->actionRollback->setEnabled(state);
}

/*
 * Removes all packages from the current transaction
 */
void MainWindow::clearTransactionTreeView()
{
  removePackagesFromRemoveTransaction();
  removePackagesFromInstallTransaction();
}

/*
 * Looks if the Transaction's Remove or Install parent items have any package inside them
 */
bool MainWindow::_isThereAPendingTransaction()
{
  return (getRemoveTransactionParentItem()->hasChildren() ||
          getInstallTransactionParentItem()->hasChildren());
}

/*
 * Retrieves the Remove parent item of the Transaction treeview
 */
QStandardItem * MainWindow::getRemoveTransactionParentItem()
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());
  QStandardItem *si;

  if(sim)
  {
    si = sim->item(0, 0);
  }

  return si;
}

/*
 * Retrieves the Install parent item of the Transaction treeview
 */
QStandardItem * MainWindow::getInstallTransactionParentItem()
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());
  QStandardItem *si;

  if(sim)
  {
    si = sim->item(1, 0);
  }

  return si;
}

/*
 * Inserts the given package into the Remove parent item of the Transaction treeview
 */
void MainWindow::insertRemovePackageIntoTransaction(const QString &pkgName)
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToRemove = new QStandardItem(pkgName);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siRemoveParent->model());

  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  if (foundItems.size() == 0)
  {
    siRemoveParent->appendRow(siPackageToRemove);
  }
  else if (foundItems.size() == 1 && foundItems.at(0)->parent() == siInstallParent)
  {
    siInstallParent->removeRow(foundItems.at(0)->row());
    siRemoveParent->appendRow(siPackageToRemove);
  }

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_TRANSACTION);
  tvTransaction->expandAll();

  changeTransactionActionsState();
}

/*
 * Inserts the given package into the Install parent item of the Transaction treeview
 */
void MainWindow::insertInstallPackageIntoTransaction(const QString &pkgName)
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToInstall = new QStandardItem(pkgName);
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siInstallParent->model());

  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  if (foundItems.size() == 0)
  {
    siInstallParent->appendRow(siPackageToInstall);
  }
  else if (foundItems.size() == 1 && foundItems.at(0)->parent() == siRemoveParent)
  {
    siRemoveParent->removeRow(foundItems.at(0)->row());
    siInstallParent->appendRow(siPackageToInstall);
  }

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_TRANSACTION);
  tvTransaction->expandAll();

  changeTransactionActionsState();
}

/*
 * Removes all packages from the Remove parent item of the Transaction treeview
 */
void MainWindow::removePackagesFromRemoveTransaction()
{
  QStandardItem * siRemove = getRemoveTransactionParentItem();
  siRemove->removeRows(0, siRemove->rowCount());
  changeTransactionActionsState();
}

/*
 * Removes all packages from the Install parent item of the Transaction treeview
 */
void MainWindow::removePackagesFromInstallTransaction()
{
  QStandardItem * siInstall = getInstallTransactionParentItem();
  siInstall->removeRows(0, siInstall->rowCount());
  changeTransactionActionsState();
}

/*
 * Retrieves the number of "to be removed" packages
 */
int MainWindow::getNumberOfTobeRemovedPackages()
{
  QStandardItem * siRemoval = getRemoveTransactionParentItem();
  return siRemoval->rowCount();
}

/*
 * Retrieves the list of all packages scheduled to be removed
 */
QString MainWindow::getTobeRemovedPackages()
{
  QStandardItem * siRemoval = getRemoveTransactionParentItem();
  QString res;

  for(int c=0; c < siRemoval->rowCount(); c++)
  {
    res += siRemoval->child(c)->text() + " ";
  }

  res = res.trimmed();
  return res;
}

/*
 * Retrieve the list of all packages scheduled to be installed
 */
QString MainWindow::getTobeInstalledPackages()
{
  QStandardItem * siInstall = getInstallTransactionParentItem();
  QString res;

  for(int c=0; c < siInstall->rowCount(); c++)
  {
    res += siInstall->child(c)->text() + " ";
  }

  res = res.trimmed();
  return res;
}

/*
 * Inserts the current selected packages for removal into the Transaction Treeview
 */
void MainWindow::insertIntoRemovePackage()
{
  QStandardItemModel *sim;
  if(ui->actionNonInstalledPackages->isChecked())
  {
    sim = m_modelPackages;
  }
  else
  {
    sim = m_modelInstalledPackages;
  }

  foreach(QModelIndex item, ui->tvPackages->selectionModel()->selectedRows())
  {

    QModelIndex mi = m_proxyModelPackages->mapToSource(item);
    QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);

    insertRemovePackageIntoTransaction(si->text());
  }
}

/*
 * Inserts the current selected packages for installation into the Transaction Treeview
 */
void MainWindow::insertIntoInstallPackage()
{
  QStandardItemModel *sim;
  if(ui->actionNonInstalledPackages->isChecked())
  {
    sim = m_modelPackages;
  }
  else
  {
    sim = m_modelInstalledPackages;
  }

  foreach(QModelIndex item, ui->tvPackages->selectionModel()->selectedRows())
  {

    QModelIndex mi = m_proxyModelPackages->mapToSource(item);
    QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);

    insertInstallPackageIntoTransaction(si->text());
  }
}

/*
 * Does a repository sync with "pacman -Sy" !
 */
void MainWindow::doSyncDatabase()
{
  //If there are no means to run the actions, we must warn!
  if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand());
    return;
  }

  m_commandExecuting = ectn_SYNC_DATABASE;
  disableTransactionActions();
  disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

  m_unixCommand = new UnixCommand(this);

  QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                   this, SLOT( actionsProcessReadOutput() ));
  QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                   this, SLOT( actionsProcessRaisedError() ));

  QStringList commands;
  commands << "pacman -Sy";
  m_unixCommand->executePackageActions(commands);
}

/*
 * Does a system upgrade with "pacman -Su" !
 */
void MainWindow::doSystemUpgrade(bool syncDatabase)
{
  if(syncDatabase)
  {
    m_commandQueued = ectn_SYSTEM_UPGRADE;
    doSyncDatabase();
  }
  else
  {
    //Shows a dialog indicating the targets needed to be retrieved and asks for the user's permission.
    m_targets = Package::getTargetUpgradeList();

    //There are no new updates to install!
    if (m_targets->count() == 0)
    {
      writeToTabOutput("<b>" + StrConstants::getNoNewUpdatesAvailable() + "</b>");
      return;
    }

    m_currentTarget=0;
    QString list;

    foreach(QString target, *m_targets)
    {
      list = list + target + "\n";
    }
    list.remove(list.size()-1, 1);

    QMessageBox question;

    if(m_targets->count()==1)
      question.setText(StrConstants::getRetrieveTarget());
    else
      question.setText(StrConstants::getRetrieveTargets().arg(m_targets->count()));

    question.setWindowTitle(StrConstants::getConfirmation());
    question.setInformativeText(StrConstants::getConfirmationQuestion());
    question.setDetailedText(list);
    question.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Close);
    question.setDefaultButton(QMessageBox::No);

    int result = question.exec();
    if(result == QMessageBox::Yes)
    {
      //If there are no means to run the actions, we must warn!
      if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
        QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand());
        return;
      }

      m_commandExecuting = ectn_SYSTEM_UPGRADE;

      disableTransactionActions();
      disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));
      m_unixCommand = new UnixCommand(this);

      QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
      QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                       this, SLOT( actionsProcessReadOutput() ));
      QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                       this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
      QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                       this, SLOT( actionsProcessRaisedError() ));

      m_currentTarget = 0;

      QStringList command;
      command << "pacman -Su --noconfirm";
      m_unixCommand->executePackageActions(command);
      m_commandQueued = ectn_NONE;
    }
  }
}

/*
 * Removes ALL the packages selected by the user with "pacman -Rcs (CASCADE)" !
 */
void MainWindow::doRemove()
{
  QString listOfTargets = getTobeRemovedPackages();
  m_targets = Package::getTargetRemovalList(listOfTargets);
  m_currentTarget=0;
  QString list;

  foreach(QString target, *m_targets)
  {
    list = list + target + "\n";
  }
  list.remove(list.size()-1, 1);

  QMessageBox question;

  Q_ASSERT(m_targets->count() > 0);

  //Shows a dialog indicating the targets which will be removed and asks for the user's permission.
  if(m_targets->count()==1)
  {
    if (m_targets->at(0).indexOf("HoldPkg was found in target list.") != -1)
    {
      QMessageBox::warning(this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
    else question.setText(StrConstants::getRemoveTarget());
  }
  else
    question.setText(StrConstants::getRemoveTargets().arg(m_targets->count()));

  if (getNumberOfTobeRemovedPackages() < m_targets->count())
    question.setWindowTitle(StrConstants::getWarning());
  else
    question.setWindowTitle(StrConstants::getConfirmation());

  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  question.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Close);
  question.setDefaultButton(QMessageBox::No);

  int result = question.exec();
  if(result == QMessageBox::Yes)
  {
    //If there are no means to run the actions, we must warn!
    if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
      QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand());
      return;
    }

    m_commandExecuting = ectn_REMOVE;

    disableTransactionActions();
    disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));
    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                     this, SLOT( actionsProcessReadOutput() ));
    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                     this, SLOT( actionsProcessRaisedError() ));

    QStringList command;
    command << "pacman -Rcs --noconfirm " + listOfTargets;
    m_unixCommand->executePackageActions(command);
  }
}

/*
 * Installs ALL the packages selected by the user with "pacman -S (INCLUDING DEPENDENCIES)" !
 */
void MainWindow::doInstall()
{
  QString listOfTargets = getTobeInstalledPackages();
  m_targets = Package::getTargetUpgradeList(listOfTargets);
  m_currentTarget=0;
  QString list;

  foreach(QString target, *m_targets)
  {
    list = list + target + "\n";
  }
  list.remove(list.size()-1, 1);

  QMessageBox question;

  Q_ASSERT(m_targets->count() > 0);

  if(m_targets->count()==1)
  {
    if (m_targets->at(0).indexOf("HoldPkg was found in target list.") != -1)
    {
      QMessageBox::warning(this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
    else question.setText(StrConstants::getRetrieveTarget());
  }
  else
    question.setText(StrConstants::getRetrieveTargets().arg(m_targets->count()));

  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  question.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Close);
  question.setDefaultButton(QMessageBox::No);

  int result = question.exec();
  if(result == QMessageBox::Yes)
  {
    //If there are no means to run the actions, we must warn!
    if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
      QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand());
      return;
    }

    m_commandExecuting = ectn_INSTALL;

    disableTransactionActions();
    disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));
    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                     this, SLOT( actionsProcessReadOutput() ));
    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                     this, SLOT( actionsProcessRaisedError() ));

    QStringList command;
    command << "pacman -S --noconfirm " + listOfTargets;
    m_unixCommand->executePackageActions(command);
  }
}

/*
 * Clears the local package cache using "pacman -Sc"
 */
void MainWindow::doCleanCache()
{
  CPUIntensiveComputing cic;
  UnixCommand::cleanPacmanCache();
  UnixCommand::removeTemporaryActionFile();
}

/*
 * Disables all Transaction related actions
 */
void MainWindow::disableTransactionActions()
{
  toggleTransactionActions(false);
}

/*
 * Enables all Transaction related actions
 */
void MainWindow::enableTransactionActions()
{
  toggleTransactionActions(true);
}

/*
 * Sets with the given boolean the state of all Transaction related actions
 */
void MainWindow::toggleTransactionActions(const bool value)
{
  bool state = _isThereAPendingTransaction();

  if (value == true && state == true)
  {
    ui->actionCommit->setEnabled(true);
    ui->actionRollback->setEnabled(true);
  }
  else if ((value == true && state == false) || value == false)
  {
    ui->actionCommit->setEnabled(false);
    ui->actionRollback->setEnabled(false);
  }

  ui->actionInstall->setEnabled(value);
  ui->actionRemove->setEnabled(value);
  ui->actionSyncPackages->setEnabled(value);
  ui->actionSystemUpgrade->setEnabled(value);
}

/*
 * Triggers the especific methods that need to be called given the packages in the transaction
 */
void MainWindow::doCommitTransaction()
{
  //Are there any remove actions to be commited?
  if(getRemoveTransactionParentItem()->rowCount() > 0 && getInstallTransactionParentItem()->rowCount() > 0)
  {
    doRemove();
    m_commandQueued = ectn_INSTALL;
  }
  else if(getRemoveTransactionParentItem()->rowCount() > 0)
  {
    doRemove();
  }
  else if(getInstallTransactionParentItem()->rowCount() > 0)
  {
    doInstall();
  }
}

/*
 * Clears the transaction treeview
 */
void MainWindow::doRollbackTransaction()
{
  int res = QMessageBox::question(this,
                        StrConstants::getConfirmation(),
                        StrConstants::getRollbackTransactionConfirmation(),
                        QMessageBox::Yes|QMessageBox::No,
                        QMessageBox::No);

  if(res == QMessageBox::Yes)
  {
    clearTransactionTreeView();
  }
}

/*
 * This SLOT is called whenever Pacman's process has just started execution
 */
void MainWindow::actionsProcessStarted()
{
  clearTabOutput();

  //First we output the name of action we are starting to execute!
  if (m_commandExecuting == ectn_SYNC_DATABASE)
  {
    writeToTabOutput("<b>" + StrConstants::getSyncDatabases() + "</b>");
  }
  else if (m_commandExecuting == ectn_SYSTEM_UPGRADE)
  {
    writeToTabOutput("<b>" + StrConstants::getSystemUpgrade() + "</b>");
  }
  else if (m_commandExecuting == ectn_REMOVE)
  {
    writeToTabOutput("<b>" + StrConstants::getRemovingPackages() + "</b>");
  }
  else if (m_commandExecuting == ectn_INSTALL)
  {
    writeToTabOutput("<b>" + StrConstants::getInstallingPackages() + "</b>");
  }

  QString msg = m_unixCommand->readAllStandardOutput();
  msg = msg.trimmed();

  if (!msg.isEmpty())
  {
    writeToTabOutput(msg);
  }
}

/*
 * This SLOT is called when Pacman's process has finished execution
 */
void MainWindow::actionsProcessFinished(int exitCode, QProcess::ExitStatus)
{
  ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName());

  if (exitCode == 0){
    writeToTabOutput("<br><b>" +
                     StrConstants::getCommandFinishedOK() + "</b><br>");
  }
  else
  {
    writeToTabOutput("<br><b>" +
                     StrConstants::getCommandFinishedWithErrors() + "</b><br>");
  }

  if(m_commandQueued == ectn_SYSTEM_UPGRADE)
  {
    doSystemUpgrade(false);
    m_commandQueued = ectn_NONE;
  }
  else if (m_commandQueued == ectn_INSTALL && m_commandExecuting == ectn_REMOVE)
  {
    if(exitCode == 0) //If the removal actions were OK...
    {
      removePackagesFromRemoveTransaction();
      doInstall();
      removePackagesFromInstallTransaction();
      m_commandQueued = ectn_NONE;
      return;
    }
  }
  else if (m_commandQueued == ectn_NONE)
  {
    if(exitCode == 0)
    {
      //After the command, we can refresh the package list, so any change can be seem.
      buildPackageList();
      connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

      clearTransactionTreeView();
    }
  }

  enableTransactionActions();

  m_commandExecuting = ectn_NONE;
  m_unixCommand->removeTemporaryActionFile();
}

/*
 * This SLOT is called whenever Pacman's process has something to output to Standard out
 */
void MainWindow::actionsProcessReadOutput()
{
  QString msg = m_unixCommand->readAllStandardOutput();
  msg = msg.trimmed();

  if(!msg.isEmpty() &&
     msg.indexOf(":: Synchronizing package databases...") == -1 &&
     msg.indexOf(":: Starting full system upgrade...") == -1)
  {
    writeToTabOutput(msg);
  }
}

/*
 * Searches the given msg for a series of verbs that a Pacman transaction may produce
 */
bool MainWindow::_searchForKeyVerbs(const QString &msg)
{
  return (msg.contains(QRegExp("checking ")) ||
          msg.contains(QRegExp("loading ")) ||
          msg.contains(QRegExp("installing ")) ||
          msg.contains(QRegExp("upgrading ")) ||
          msg.contains(QRegExp("resolving ")) ||
          msg.contains(QRegExp("looking ")) ||
          msg.contains(QRegExp("removing ")));
}

/*
 * Processes the output of the 'pacman process' so we can update percentages and messages at real time
 */
void MainWindow::_treatProcessOutput(const QString &pMsg)
{
  QString perc;
  QString msg = pMsg;
  msg.remove(QRegExp(".+\\[Y/n\\].+"));
  msg.remove(QRegExp("warning:\\S{0}"));

  //std::cout << "out: " << msg.toAscii().data() << std::endl;

  //If it is a percentage, we are talking about curl output...
  if(msg.indexOf("#]") != -1)
  {
    perc = "100%";
    ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName() + " (100%)");
    qApp->processEvents();
  }
  else if (msg.indexOf("-]") != -1)
  {
    QString target;
    perc = msg.right(4).trimmed();

    if (m_commandExecuting == ectn_INSTALL ||
        m_commandExecuting == ectn_SYSTEM_UPGRADE ||
        m_commandExecuting == ectn_SYNC_DATABASE)
    {
      QString order;
      int ini = msg.indexOf(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "));
      if (ini == 0)
      {
        int rp = msg.indexOf(")");
        order = msg.left(rp+2);
        msg = msg.remove(0, rp+2);

        if (_searchForKeyVerbs(msg))
        {
          int end = msg.indexOf("[");
          msg = msg.remove(end, msg.size()-end).trimmed();

          if(!_textInTabOutput(msg))
          {
            writeToTabOutput(msg);
          }
        }
        else
        {
          int pos = msg.indexOf(" ");
          if (pos >=0)
          {
            target = msg.left(pos);
            target = target.trimmed();
            //std::cout << "target: " << target.toAscii().data() << std::endl;

            if(!target.isEmpty() && !_textInTabOutput(target))
            {
              writeToTabOutput("<font color=\"brown\">" + target + "</font>");
            }
          }
          else if (!_textInTabOutput(msg))
            writeToTabOutput("<font color=\"brown\">" + msg + "</font>");
        }
      }
      else if (ini == -1)
      {
        if (_searchForKeyVerbs(msg))
        {
          int end = msg.indexOf("[");
          msg = msg.remove(end, msg.size()-end);
          msg = msg.trimmed();

          if(!_textInTabOutput(msg))
            writeToTabOutput(msg);
        }
        else
        {
          int pos = msg.indexOf(" ");
          if (pos >=0)
          {
            target = msg.left(pos);
            target = target.trimmed();
            //std::cout << "target: " << target.toAscii().data() << std::endl;

            if(!target.isEmpty() && !_textInTabOutput(target))
            {
              if(m_commandExecuting == ectn_SYNC_DATABASE)
              {
                writeToTabOutput("<font color=\"blue\">" + StrConstants::getSynchronizing() + " " + target + "</font>");
              }
              else
              {
                writeToTabOutput("<font color=\"brown\">" + target + "</font>");
              }
            }
          }
          else if (!_textInTabOutput(msg))
            writeToTabOutput("<font color=\"blue\">" + msg + "</font>");
        }
      }
    }
    else if (m_commandExecuting == ectn_REMOVE)
    {
      QString order;
      int ini = msg.indexOf(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "));
      if (ini == 0)
      {
        int rp = msg.indexOf(")");
        order = msg.left(rp+2);
        msg = msg.remove(0, rp+2);
      }

      int pos = msg.indexOf("[");
      if (pos >=0)
      {
        target = msg.left(pos);
        target = target.trimmed();
        //std::cout << "target: " << target.toAscii().data() << std::endl;

        if(!target.isEmpty() && !_textInTabOutput(target))
          writeToTabOutput("<font color=\"red\">" + target + "</font>");
      }
      else
      {
        msg = msg.trimmed();
        if (!_textInTabOutput(msg))
          writeToTabOutput("<font color=\"red\">" + msg + "</font>");
      }
    }

    if(!perc.isEmpty() && perc.indexOf("%") > 0)
    {
      qApp->processEvents();
      ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName() + " (" + perc + ")");
      qApp->processEvents();
    }
  }
  //It's another error, so we have to output it
  else
  {
    //Let's supress some annoying string bugs...
    msg.remove(QRegExp("\\(process.+"));
    msg.remove(QRegExp("Using the fallback.+"));
    msg.remove(QRegExp("Gkr-Message: secret service operation failed:.+"));
    msg.remove(QRegExp("gksu-run.+"));
    msg.remove(QRegExp("Do you want.+"));

    msg = msg.trimmed();

    QString order;
    int ini = msg.indexOf(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "));
    if (ini == 0)
    {
      int rp = msg.indexOf(")");
      order = msg.left(rp+2);
      msg = msg.remove(0, rp+2);
    }

    if (!msg.isEmpty() && !_textInTabOutput(msg))
    {
      if (msg.contains(QRegExp("removing ")))
        writeToTabOutput("<font color=\"red\">" + msg + "</font>");
      else
      {
        if (msg.indexOf(":: Synchronizing package databases...") == -1 &&
            msg.indexOf(":: Starting full system upgrade...") == -1)
        {
          writeToTabOutput(msg); //it was font color = black
        }
      }
    }
  }

  if(m_commandExecuting == ectn_NONE)
    ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName());
}

/*
 * This SLOT is called whenever Pacman's process has something to output to Standard error
 */
void MainWindow::actionsProcessRaisedError()
{
  QString msg = m_unixCommand->readAllStandardError();
  msg = msg.trimmed();
  QStringList msgs = msg.split(QRegExp("\\n"), QString::SkipEmptyParts);

  foreach (QString m, msgs)
  {
    QStringList m2 = m.split(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "), QString::SkipEmptyParts);

    if (m2.count() == 1)
    {
      //Let's try another test... if it doesn't work, we give up.
      QStringList maux = m.split(QRegExp("%"), QString::SkipEmptyParts);
      if (maux.count() > 1)
      {
        foreach (QString aux, maux)
        {
          aux = aux.trimmed();
          if (!aux.isEmpty())
          {
            if (aux.at(aux.count()-1).isDigit())
            {
              aux += "%";
            }

            //std::cout << "Error1: " << aux.toAscii().data() << std::endl;
            _treatProcessOutput(aux);
          }
        }
      }
      else if (maux.count() == 1)
      {
        if (!m.isEmpty())
        {
          //std::cout << "Error2: " << m.toAscii().data() << std::endl;
          _treatProcessOutput(m);
        }
      }
    }
    else if (m2.count() > 1)
    {
      foreach (QString m3, m2)
      {
        if (!m3.isEmpty())
        {
          //std::cout << "Error3: " << m3.toAscii().data() << std::endl;
          _treatProcessOutput(m3);
        }
      }
    }
  }
}
