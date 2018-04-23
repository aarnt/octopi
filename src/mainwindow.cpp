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

#include "mainwindow.h"
#include "searchlineedit.h"
#include "ui_mainwindow.h"
#include "strconstants.h"
#include "package.h"
#include "uihelper.h"
#include "wmhelper.h"
#include "treeviewpackagesitemdelegate.h"
#include "searchbar.h"
#include "utils.h"
#include "globals.h"
#include <iostream>
#include "optionsdialog.h"

#ifdef QTERMWIDGET
  #include "termwidget.h"
#endif

#include <QDropEvent>
#include <QMimeData>
#include <QStandardItemModel>
#include <QString>
#include <QTextBrowser>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QComboBox>
#include <QModelIndex>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHash>
#include <QFutureWatcher>
#include <QToolTip>
#include <QtConcurrent/QtConcurrentRun>

/*
 * MainWindow's constructor: basic UI init
 */
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent), ui(new Ui::MainWindow), m_packageModel(new PackageModel(m_packageRepo))
{
  m_hasAURTool =
      UnixCommand::hasTheExecutable(Package::getForeignRepositoryToolName()) && !UnixCommand::isRootRunning();

  m_packageRepo.registerDependency(*m_packageModel);
  m_foundFilesInPkgFileList = new QList<QModelIndex>();
  m_indFoundFilesInPkgFileList = 0;
  m_callSystemUpgrade = false;
  m_callSystemUpgradeNoConfirm = false;
  m_initializationCompleted=false;
  m_systemUpgradeDialog = false;
  m_refreshPackageLists = false;
  m_refreshForeignPackageList = false;
  m_cic = NULL;
  m_outdatedStringList = new QStringList();
  m_outdatedAURStringList = new QStringList();
  m_outdatedAURPackagesNameVersion = new QHash<QString, QString>();
  m_selectedViewOption = ectn_ALL_PKGS;
  m_selectedRepository = "";
  m_numberOfInstalledPackages = 0;
  m_debugInfo = false;
  m_console = nullptr;

  m_time = new QTime();
  m_unrequiredPackageList = nullptr;
  m_foreignPackageList = nullptr;
  m_groupWidgetNeedsFocus = false;
  m_outdatedAURTimer = new QTimer();
  m_outdatedAURTimer->setInterval(50);
  connect(m_outdatedAURTimer, SIGNAL(timeout()), this, SLOT(postBuildPackageList()));

  //Here we try to speed up first pkg list build!
  m_time->start();

  retrieveOutdatedPackageList();
  retrieveUnrequiredPackageList();
  retrieveForeignPackageList();

  ui->setupUi(this);
  switchToViewAllPackages();  

  m_pacmanDatabaseSystemWatcher =
            new QFileSystemWatcher(QStringList() << ctn_PACMAN_DATABASE_DIR, this);

  connect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(onPacmanDatabaseChanged()));

  setAcceptDrops(true);
}

/*
 * MainWindow's destructor
 */
MainWindow::~MainWindow()
{
  savePackageColumnWidths();

  //Let's garbage collect transaction files...
  UnixCommand::removeTemporaryFiles();
  delete ui;
}

/*
 * The show() public SLOT, when the app is being drawn!!!
 * Init member variables and all UI widgets
 */
void MainWindow::show()
{
  if(m_initializationCompleted == false)
  {
    restoreGeometry(SettingsManager::getWindowSize());
    m_commandExecuting=ectn_NONE;
    m_commandQueued=ectn_NONE;
    m_leFilterPackage = new SearchLineEdit(this, m_hasSLocate);

    setWindowTitle(StrConstants::getApplicationName());
    setMinimumSize(QSize(820, 520));

    initTabOutput();
    initTabInfo();
    initTabFiles();
    initTabTransaction();
    initTabHelpUsage();
    initTabNews();
    initPackageTreeView();
    initActions();
    loadSettings();
    initLineEditFilterPackages();
    loadPanelSettings();
    initStatusBar();
    initToolButtonPacman();
    initToolButtonAUR();
    initAppIcon();
    initMenuBar();
    initToolBar();

#ifdef QTERMWIDGET
    onTerminalChanged();
#endif

    if (m_hasAURTool) m_actionSwitchToAURTool->setEnabled(false);

    initTabWidgetPropertiesIndex();
    refreshDistroNews(false);

    if (Package::hasPacmanDatabase())
    {
      refreshGroupsWidget();
    }        

    ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_POPULARITY_COLUMN, true);
    QMainWindow::show();

    m_listOfVisitedPackages.clear();
    m_indOfVisitedPackage = 0;

    if (Package::hasPacmanDatabase())
    {
      metaBuildPackageList();
    }
    // Maybe this system has never run a pacman -Syy
    else
    {
      doSyncDatabase();
    }
  }
  else
    QMainWindow::show();
}

/*
 * Whenever a user drops a package from any outside source, we try to install it
 */
void MainWindow::dropEvent(QDropEvent *ev)
{
  m_packagesToInstallList.clear();

  QList<QUrl> urls = ev->mimeData()->urls();

  foreach(QUrl url, urls)
  {
    QString str = url.fileName();
    QFileInfo f(str);
    if (f.completeSuffix().contains("pkg.tar"))
    {
      m_packagesToInstallList.append(url.toLocalFile());
    }
  }

  if (m_packagesToInstallList.count() > 0)
    doInstallLocalPackages();
}

/*
 * Whenever an outside package enters Octopi mainwindow space, we check to see if it's really a package
 */
void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
  bool success = false;
  QList<QUrl> urls = ev->mimeData()->urls();

  foreach(QUrl url, urls)
  {
    QString str = url.fileName();
    QFileInfo f(str);
    if (f.completeSuffix().contains("pkg.tar"))
    {
      success=true;
      break;
    }
  }

  if (success) ev->accept();
  else ev->ignore();
}

/*
 * Whenever there is a change in the pacman database...
 */
void MainWindow::onPacmanDatabaseChanged()
{
  if (m_initializationCompleted) m_refreshPackageLists = true;
}

/*
 * Whenever user changes the Groups widget item...
 */
void MainWindow::onPackageGroupChanged()
{
  if (isAllGroupsSelected())
  {
    if (m_commandExecuting == ectn_NONE) m_actionSwitchToAURTool->setEnabled(true);
    ui->actionSearchByName->setChecked(true);
    tvPackagesSearchColumnChanged(ui->actionSearchByName);
  }
  else m_actionSwitchToAURTool->setEnabled(false);
}

/*
 * Whenever we call Options menu
 */
void MainWindow::onOptions()
{
  if (m_commandExecuting != ectn_NONE) return;

  OptionsDialog *od = new OptionsDialog(this);
  connect(od, SIGNAL(AURToolChanged()), this, SLOT(onAURToolChanged()));

#ifdef QTERMWIDGET
  connect(od, SIGNAL(terminalChanged()), this, SLOT(onTerminalChanged()));
#endif

  od->exec();
  Options::result res = od->result();

  delete od;

  if (res & Options::ectn_ICON)
  {
    refreshAppIcon();
  }

  if (res & Options::ectn_BACKEND)
  {
    AURToolSelected();
  }
}

/*
 * Prepares UI and logic to show all available packages
 */
void MainWindow::switchToViewAllPackages()
{
  m_selectedViewOption = ectn_ALL_PKGS;
  disconnect(ui->actionViewAllPackages, SIGNAL(triggered()), this, SLOT(selectedAllPackagesMenu()));
  ui->actionViewAllPackages->setChecked(true);
  connect(ui->actionViewAllPackages, SIGNAL(triggered()), this, SLOT(selectedAllPackagesMenu()));
}

/*
 * Retrieves a pointer to Output's QTextBrowser object
 */
QTextBrowser *MainWindow::getOutputTextBrowser()
{
  QTextBrowser *ret=0;
  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");

  if (text)
  {
    ret = text;
  }

  return ret;
}

/*
 * Shows comment of given anchor
 */
void MainWindow::showAnchorDescription(const QUrl &link)
{
  if (link.toString().contains("goto:"))
  {            
    QString pkgName = link.toString().mid(5);
    //Let's remove any "<" and "<=" symbol...
    pkgName.remove(QRegularExpression("%3C\\S*"));

    if (pkgName == "sh") pkgName = "bash";
    QFuture<QString> f;
    disconnect(&g_fwToolTipInfo, SIGNAL(finished()), this, SLOT(execToolTip()));
    f = QtConcurrent::run(showPackageInfo, pkgName);
    g_fwToolTipInfo.setFuture(f);
    connect(&g_fwToolTipInfo, SIGNAL(finished()), this, SLOT(execToolTip()));
  }
}

/*
 * When the tooltip QFuture method is finished, we show the selected tooltip to the user
 */
void MainWindow::execToolTip()
{
  QPoint point = QCursor::pos();
  if (g_fwToolTipInfo.result().trimmed().isEmpty())
    return;

  point.setX(point.x());
  point.setY(point.y());
  QToolTip::showText(point, g_fwToolTipInfo.result());
}

/*
 * Whenever we want to position the cursor in a specific package in the list
 */
void MainWindow::positionInPackageList(const QString &pkgName)
{
  QModelIndex columnIndex = m_packageModel->index(0, PackageModel::ctn_PACKAGE_NAME_COLUMN, QModelIndex());
  QModelIndexList foundItems = m_packageModel->match(columnIndex, Qt::DisplayRole, pkgName, -1, Qt::MatchExactly);
  QModelIndex proxyIndex;

  if (foundItems.count() == 1)
  {
    proxyIndex = foundItems.first();
    if(proxyIndex.isValid())
    {
      ui->tvPackages->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);
      ui->tvPackages->setCurrentIndex(proxyIndex);
      changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
    }
  }
  if (foundItems.count() == 0 || !proxyIndex.isValid())
  {
    refreshTabInfo(pkgName);
    disconnect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
    ensureTabVisible(ctn_TABINDEX_INFORMATION);
    connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
  }
}

/*
 * This SLOT is called whenever user clicks a url inside output's textBrowser
 */
void MainWindow::outputTextBrowserAnchorClicked(const QUrl &link)
{
  if (link.toString().contains("goto:"))
  {
    QString pkgName = link.toString().mid(5);
    if (pkgName == "sh") pkgName = "bash";
    bool indIncremented = false;
    QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();

    if (selectionModel->selectedRows().count() <= 0) return;

    QModelIndex item = selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).first();
    const PackageRepository::PackageData*const selectedPackage = m_packageModel->getData(item);

    if (!m_listOfVisitedPackages.isEmpty())
    {
      int limit = m_listOfVisitedPackages.count()-1;

      if (m_indOfVisitedPackage <= limit)
      {
        if (m_listOfVisitedPackages.at(m_indOfVisitedPackage) != selectedPackage->name)
        {
          m_indOfVisitedPackage++;
          indIncremented = true;
          m_listOfVisitedPackages.insert(m_indOfVisitedPackage, selectedPackage->name);
          m_listOfVisitedPackages.insert(m_indOfVisitedPackage+1, pkgName);
          m_indOfVisitedPackage++;
        }
        else
        {
          if ((m_indOfVisitedPackage+1) <= limit)
          {
            if (m_listOfVisitedPackages.at(m_indOfVisitedPackage+1) != pkgName)
            {
              m_listOfVisitedPackages.insert(m_indOfVisitedPackage+1, pkgName);
            }
          }
          else
            m_listOfVisitedPackages.insert(m_indOfVisitedPackage+1, pkgName);
        }
      }
      else if (m_indOfVisitedPackage == 1)
      {
        m_indOfVisitedPackage++;
        indIncremented = true;
        m_listOfVisitedPackages.insert(m_indOfVisitedPackage, selectedPackage->name);
        m_listOfVisitedPackages.insert(m_indOfVisitedPackage+1, pkgName);
      }
    }
    else //The list is EMPTY!
    {
      m_indOfVisitedPackage++;
      indIncremented = true;
      m_listOfVisitedPackages.insert(m_indOfVisitedPackage, selectedPackage->name);
      m_listOfVisitedPackages.insert(m_indOfVisitedPackage+1, pkgName);
    }

    if (indIncremented == false) m_indOfVisitedPackage++;

    positionInPackageList(pkgName);
  }
  //Otherwise, it's a remote URL which needs to be opened outside Octopi
  else
  {
    QDesktopServices::openUrl(link);
  }
}

/*
 * Prints the list of outdated packages to the Output tab.
 */
void MainWindow::outputOutdatedPackageList()
{
  //We cannot output any list if there is a running transaction!
  if (m_commandExecuting != ectn_NONE ||
      isAURGroupSelected())
    return;

  m_numberOfOutdatedPackages = m_outdatedStringList->count();

  if(m_numberOfOutdatedPackages > 0)
  {
    QString html = "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
    QString anchorBegin = "anchorBegin";
    html += "<a id=\"" + anchorBegin + "\"></a>";

    clearTabOutput();

    if(m_outdatedStringList->count()==1){
      html += "<h3>" + StrConstants::getOneOutdatedPackage() + "</h3>";
    }
    else
    {
      html += "<h3>" +
          StrConstants::getOutdatedPackages(m_outdatedStringList->count()) + "</h3>";
    }

    html += "<br><table border=\"0\">";
    html += "<tr><th width=\"25%\" align=\"left\">" + StrConstants::getName() +
        "</th><th width=\"18%\" align=\"right\">" +
        StrConstants::getOutdatedVersion() +
        "</th><th width=\"18%\" align=\"right\">" +
        StrConstants::getAvailableVersion() + "</th></tr>";

    for (int c=0; c < m_outdatedStringList->count(); c++)
    {
      QString pkg = m_outdatedStringList->at(c);
      const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkg);

      if (package != NULL) {
        html += "<tr><td><a href=\"goto:" + pkg + "\">" + pkg +
            "</td><td align=\"right\"><b><font color=\"#E55451\">" +
            package->outdatedVersion +
            "</b></font></td><td align=\"right\">" +
            package->version + "</td></tr>";
      }
    }

    writeToTabOutput(html);

    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");

    if (text)
    {
      text->scrollToAnchor(anchorBegin);
    }

    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  }
}

/*
 * Prints the list of outdated AUR packages to the Output tab.
 */
void MainWindow::outputOutdatedAURPackageList()
{
  //We cannot output any list if there is a running transaction!
  if (m_commandExecuting != ectn_NONE) return;

  QString html = "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
  QString anchorBegin = "anchorBegin";
  html += "<a id=\"" + anchorBegin + "\"></a>";

  clearTabOutput();

  if(m_outdatedAURStringList->count()==1){
    html += "<h3>" + StrConstants::getOneOutdatedPackage() + "</h3>";
  }
  else
  {
    html += "<h3>" +
        StrConstants::getOutdatedPackages(m_outdatedAURStringList->count()) + "</h3>";
  }

  html += "<br><table border=\"0\">";
  html += "<tr><th width=\"25%\" align=\"left\">" + StrConstants::getName() +
      "</th><th width=\"18%\" align=\"right\">" +
      StrConstants::getOutdatedVersion() +
      "</th><th width=\"18%\" align=\"right\">" +
      StrConstants::getAvailableVersion() + "</th></tr>";

  for (int c=0; c < m_outdatedAURStringList->count(); c++)
  {
    QString pkg = m_outdatedAURStringList->at(c);
    const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkg);
    if (package != NULL) {
      QString availableVersion = m_outdatedAURPackagesNameVersion->value(m_outdatedAURStringList->at(c));
  
      html += "<tr><td><a href=\"goto:" + pkg + "\">" + pkg +
          "</td><td align=\"right\"><b><font color=\"#E55451\">" +
          package->version +
          "</b></font></td><td align=\"right\">" +
          availableVersion + "</td></tr>";
    }
  }

  writeToTabOutput(html);

  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");

  if (text)
  {
    text->scrollToAnchor(anchorBegin);
  }

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
}

/*
 * Removes all text inside the TabOutput editor
 */
void MainWindow::clearTabOutput()
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    text->clear();
  }
}

/*
 * Retrieves the selected package group from the treeWidget
 */
QString MainWindow::getSelectedGroup()
{
  return ui->twGroups->currentItem()->text(0);
}

/*
 * Helper to analyse if <Display All Groups> is selected
 */
bool MainWindow::isAllGroupsSelected()
{
  QModelIndex index = ui->twGroups->currentIndex();
  QString group = ui->twGroups->model()->data(index).toString();

  return isAllGroups(group);
}

bool MainWindow::isAllGroups(const QString& group)
{
  return ((group == "<" + StrConstants::getDisplayAllGroups() + ">") && !(m_actionSwitchToAURTool->isChecked()));
}

/*
 * Helper to analyse if < AUR > is selected
 */
bool MainWindow::isAURGroupSelected()
{
  return (m_actionSwitchToAURTool->isChecked());
}

/*
 * Helper to retrieve if "Search/By file" is selected
 */
bool MainWindow::isSearchByFileSelected()
{
  return ui->actionSearchByFile->isChecked();
}

/*
 * Enables/disables INSTANT SEARCH feature
 */
void MainWindow::toggleInstantSearch()
{
  if (ui->actionUseInstantSearch->isChecked())
  {
    SettingsManager::setInstantSearchSelected(true);
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
  }
  else
  {
    SettingsManager::setInstantSearchSelected(false);
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
  }
}

/*
 * Switches debugInfo ON!
 */
void MainWindow::turnDebugInfoOn()
{
  m_debugInfo = true;
}

/*
 * Gets the first package which has name "pkgName" from default pkg cache
 */
const PackageRepository::PackageData* MainWindow::getFirstPackageFromRepo(const QString pkgName)
{
  return m_packageRepo.getFirstPackageByName(pkgName);
}

/*
 * Sets a flag to call the System Upgrade action as soom as it's possible
 */
void MainWindow::setCallSystemUpgrade()
{
  m_callSystemUpgrade = true;
}

/*
 * Sets a flag to call the System Upgrade action, without confirmation dialogs!
 */
void MainWindow::setCallSystemUpgradeNoConfirm()
{
  m_callSystemUpgradeNoConfirm = true;
}

/*
 * Sets a flag that holds the remove command to be used in transactions
 */
void MainWindow::setRemoveCommand(const QString &removeCommand)
{
  m_removeCommand = removeCommand;
  m_removeCommand.remove("=");
  m_removeCommand.remove("-");
}

/*
 * Searchs model modelInstalledPackages by a package name and returns if it is already installed
 */
bool MainWindow::isPackageInstalled(const QString &pkgName)
{
  const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkgName);
  return (package != NULL && package->installed());
}

/*
 * User changed the search column used by the SortFilterProxyModel in tvPackages
 */
void MainWindow::tvPackagesSearchColumnChanged(QAction *actionSelected)
{
  //We are in the realm of tradictional NAME search
  if (actionSelected->objectName() == ui->actionSearchByName->objectName())
  {
    if (ui->actionUseInstantSearch->isChecked())
    {
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    }

    ui->menuView->setEnabled(true);
    if (!m_actionSwitchToAURTool->isChecked()) ui->twGroups->setEnabled(true);

    if (isAURGroupSelected())
      m_leFilterPackage->setRefreshValidator(ectn_AUR_VALIDATOR);
    else
      m_leFilterPackage->setRefreshValidator(ectn_DEFAULT_VALIDATOR);

    m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_NAME_COLUMN);
  }
  //We are talking about slower 'search by description'...
  else if (actionSelected->objectName() == ui->actionSearchByDescription->objectName())
  {
    if (ui->actionUseInstantSearch->isChecked())
    {
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    }

    ui->menuView->setEnabled(true);
    if (!m_actionSwitchToAURTool->isChecked()) ui->twGroups->setEnabled(true);

    if (isAURGroupSelected())
      m_leFilterPackage->setRefreshValidator(ectn_AUR_VALIDATOR);
    else
      m_leFilterPackage->setRefreshValidator(ectn_DEFAULT_VALIDATOR);

    m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN);
  }
  else if (actionSelected->objectName() == ui->actionSearchByFile->objectName())
  {
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));

    m_leFilterPackage->clear();
    m_packageModel->applyFilter("");
    ui->actionViewAllPackages->trigger();
    m_actionRepositoryAll->trigger();
    ui->menuView->setEnabled(false);

    ui->twGroups->setEnabled(false);
    m_leFilterPackage->setRefreshValidator(ectn_FILE_VALIDATOR);
  }

  if (!isSearchByFileSelected() && m_packageModel->getPackageCount() <= 1)
  {
    m_leFilterPackage->clear();
  }

  QModelIndex mi = m_packageModel->index(0, PackageModel::ctn_PACKAGE_NAME_COLUMN, QModelIndex());
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  changedTabIndex();
  m_leFilterPackage->setFocus();
  //if (isPackageTreeViewVisible() && !ui->tvPackages->hasFocus()) ui->tvPackages->setFocus();
  //else m_leFilterPackage->setFocus();
}

/*
 * Whenever the user selects a different item in View menu,
 * we have to change the model of the Packages treeview...
 */
void MainWindow::changePackageListModel(ViewOptions viewOptions, QString selectedRepo)
{  
  if (m_actionSwitchToAURTool->isChecked())
    m_packageModel->applyFilter(viewOptions, "", StrConstants::getForeignToolGroup());
  else
    m_packageModel->applyFilter(viewOptions, selectedRepo, isAllGroupsSelected() ? "" : getSelectedGroup());

  if (m_leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex cIcon = m_packageModel->index(0, PackageModel::ctn_PACKAGE_ICON_COLUMN, QModelIndex());

  ui->tvPackages->setCurrentIndex(cIcon);
  ui->tvPackages->scrollTo(cIcon, QAbstractItemView::PositionAtTop);

  tvPackagesSelectionChanged(QItemSelection(),QItemSelection());
  changedTabIndex();
}

/*
 * Slot that treats <ENTER/RETURN/DELETE> key interactions in the package list
 */
void MainWindow::execKeyActionOnPackage(CommandExecuting command)
{
  if (m_commandExecuting != ectn_NONE) return;

  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (selectionModel != NULL && selectionModel->selectedRows().count() > 0)
  {
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.count() == 1)
    {
      QModelIndex item = selectedRows.at(0);
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      if (package)
      {
        if (package->installed() && command == ectn_REMOVE)
        {
          if (package->repository == StrConstants::getForeignRepositoryName())
          {
            doRemoveAURPackage();
          }
          else if (!package->required)
          {
            insertIntoRemovePackage();
          }
        }
        else if (command == ectn_INSTALL)
        {
          if (package->repository == StrConstants::getForeignRepositoryName())
          {
            doInstallAURPackage();
          }
          else
          {
            insertIntoInstallPackage();
          }
        }
      }
    }
    else //There are more than 1 package selected
    {
      foreach(QModelIndex item, selectedRows)
      {
        const PackageRepository::PackageData*const package = m_packageModel->getData(item);

        if (package->repository == StrConstants::getForeignRepositoryName())
        {
          return;
        }
        if (command == ectn_INSTALL)
        {
          insertIntoInstallPackage(&item);
        }
        else if (package->installed() && command == ectn_REMOVE && !package->required)
        {
          insertIntoRemovePackage(&item);
        }
      }
    }
  }
}

/*
 * Brings the context menu when the user clicks the right button above the package list
 */
void MainWindow::execContextMenuPackages(QPoint point)
{
/* The following SLOTs will be called:
 *   connect(ui->actionFindFileInPackage, SIGNAL(triggered()), this, SLOT(findFileInPackage()));
 *   connect(ui->actionInstall, SIGNAL(triggered()), this, SLOT(insertIntoInstallPackage()));
 *   connect(ui->actionInstallAUR, SIGNAL(triggered()), this, SLOT(doInstallAURPackage()));
 *   connect(ui->actionInstallGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoInstallPackage()));
 *   connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(insertIntoRemovePackage()));
 *   connect(ui->actionRemoveGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoRemovePackage()));
 */
  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (selectionModel != NULL && selectionModel->selectedRows().count() > 0)
  {
    QMenu* menu = new QMenu(this);
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.count() == 1) // enable entry "browse installed files" ?
    {
      QModelIndex item = selectedRows.at(0);
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      if (package && package->installed()) {
        menu->addAction(ui->actionFindFileInPackage);
        menu->addSeparator();
      }
    }

    bool allInstallable = true;
    bool allRemovable = true;    
    int numberOfSelPkgs = selectedRows.count();
    int numberOfAUR = 0;

    foreach(QModelIndex item, selectedRows)
    {
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);

      if (package->repository == StrConstants::getForeignRepositoryName())
      {
        allInstallable = false;
        numberOfAUR++;
      }
      if (package->installed() == false)
      {
        allRemovable = false;
      }
    }

    if (allInstallable) // implicitly foreign packages == 0
    {
      if (!isAllGroupsSelected() && !isAURGroupSelected()) menu->addAction(ui->actionInstallGroup);
      menu->addAction(ui->actionInstall);

      if (!isAllGroupsSelected() && !isAURGroupSelected()) //&& numberOfSelPkgs > 1)
      {
        //Is this group already installed?
        const QList<PackageRepository::PackageData*> packageList = m_packageRepo.getPackageList(getSelectedGroup());
        if (packageList.size() == numberOfSelPkgs)
        {
          //If we select all packages, let's subtract the install action...
          menu->removeAction(ui->actionInstall);
        }
      }
    }
    else if (allInstallable == false && numberOfAUR == numberOfSelPkgs)
    {
      menu->addAction(ui->actionInstallAUR); // installs directly
    }

    if (allRemovable)
    {
      menu->addAction(ui->actionRemove);

      if (!isAllGroupsSelected() && !isAURGroupSelected())
      {
        //Is this group already installed?
        const QList<PackageRepository::PackageData*> packageList = m_packageRepo.getPackageList(getSelectedGroup());
        if (packageList.size() == numberOfSelPkgs)
        {
          //If we select all packages, let's subtract the remove action...
          menu->removeAction(ui->actionRemove);
          menu->addAction(ui->actionRemoveGroup);
        }
      }
    }

    if (menu->actions().count() > 0)
    {
      QPoint pt2 = ui->tvPackages->mapToGlobal(point);
      pt2.setY(pt2.y() + ui->tvPackages->header()->height());
      menu->exec(pt2);
    }
  }
}

/*
 * This SLOT collapses all treeview items
 */
void MainWindow::collapseAllContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if (tv != 0)
    tv->collapseAll();
}

/*
 * This SLOT collapses only the currently selected item
 */
void MainWindow::collapseThisContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if ( tv != 0 )
  {
    tv->repaint(tv->rect());
    QCoreApplication::processEvents();
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tv->model());

    if (sim)
    {
      QModelIndex mi = tv->currentIndex();
      if (sim->hasChildren(mi))	collapseItem(tv, sim, mi);
    }
  }
}

/*
 * This SLOT expands all treeview items
 */
void MainWindow::expandAllContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if ( tv != 0 )
  {
    tv->repaint(tv->rect());
    QCoreApplication::processEvents();
    tv->expandAll();
  }
}

/*
 * This SLOT expands only the currently selected item
 */
void MainWindow::expandThisContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if ( tv != 0 )
  {
    tv->repaint(tv->rect());
    QCoreApplication::processEvents();
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tv->model());

    if (sim)
    {
      QModelIndex mi = tv->currentIndex();
      if (sim->hasChildren(mi))	expandItem(tv, sim, &mi);
    }
  }
}

/*
 * This method does the job of collapsing the given item and its children
 */
void MainWindow::collapseItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex mi){
  for (int i=0; i<sim->rowCount(mi); i++)
  {
    if (sim->hasChildren(mi))
    {
      QCoreApplication::processEvents();
      tv->collapse(mi);
      QModelIndex mi2 = mi.child(i, 0);
      collapseItem(tv, sim, mi2);
    }
  }
}

/*
 * This method does the job of expanding the given item and its children
 */
void MainWindow::expandItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex* mi){
  for (int i=0; i<sim->rowCount(*mi); i++){
    if (sim->hasChildren(*mi)){
      tv->expand(*mi);
      QModelIndex mi2 = mi->child(i, 0);
      expandItem(tv, sim, &mi2);
    }
  }
}

/*
 * Brings the context menu when the user clicks the right button
 * above the Files treeview in "Files" Tab
 */
void MainWindow::execContextMenuPkgFileList(QPoint point)
{
  QTreeView *tvPkgFileList =
      ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");

  if (tvPkgFileList == 0)
  {
    return;
  }

  QModelIndex mi = tvPkgFileList->currentIndex();
  QString selectedPath = utils::showFullPathOfItem(mi);
  QMenu menu(this);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());

  if (sim)
  {
    QStandardItem *si = sim->itemFromIndex(mi);

    if (si == 0) return;
    if (si->hasChildren() && (!tvPkgFileList->isExpanded(mi)))
      menu.addAction(ui->actionExpandItem);

    if (si->hasChildren() && (tvPkgFileList->isExpanded(mi)))
      menu.addAction(ui->actionCollapseItem);

    if (menu.actions().count() > 0)
      menu.addSeparator();

    menu.addAction(ui->actionCollapseAllItems);
    menu.addAction(ui->actionExpandAllItems);
    menu.addSeparator();

    menu.addAction(m_actionCopyFullPath);

    QDir d;
    QFile f(selectedPath);

    if (si->icon().pixmap(QSize(22,22)).toImage() ==
        IconHelper::getIconFolder().pixmap(QSize(22,22)).toImage())
    {
      if (d.exists(selectedPath))
      {
        menu.addAction(ui->actionOpenDirectory);
        menu.addAction(ui->actionOpenTerminal);
      }
    }
    else if (f.exists())
    {
      menu.addAction(ui->actionOpenFile);
    }
    if (!UnixCommand::isRootRunning() && f.exists() && UnixCommand::isTextFile(selectedPath))
    {
      menu.addAction(ui->actionEditFile);
    }

    QPoint pt2 = tvPkgFileList->mapToGlobal(point);
    pt2.setY(pt2.y() + tvPkgFileList->header()->height());
    menu.exec(pt2);
  }
}

/*
 * Brings the context menu when the user clicks the right button
 * above the Transaction treeview in "Transaction" Tab
 */
void MainWindow::execContextMenuTransaction(QPoint point)
{
  QTreeView *tvTransaction = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvTransaction");
  if (!tvTransaction) return;

  if ((tvTransaction->currentIndex() == getRemoveTransactionParentItem()->index() &&
      tvTransaction->model()->hasChildren(tvTransaction->currentIndex())) ||
      (tvTransaction->currentIndex() == getInstallTransactionParentItem()->index() &&
            tvTransaction->model()->hasChildren(tvTransaction->currentIndex())))
  {
    QMenu menu(this);
    menu.addAction(ui->actionRemoveTransactionItems);
    QPoint pt2 = tvTransaction->mapToGlobal(point);
    pt2.setY(pt2.y() + tvTransaction->header()->height());
    menu.exec(pt2);
  }
  else if (tvTransaction->currentIndex() != getRemoveTransactionParentItem()->index() &&
           tvTransaction->currentIndex() != getInstallTransactionParentItem()->index() &&
           tvTransaction->currentIndex().isValid())
  {
    QMenu menu(this);

    if (tvTransaction->selectionModel()->selectedIndexes().count() == 1)
    {
      ui->actionRemoveTransactionItem->setText(StrConstants::getRemoveItem());
    }
    else
    {
      ui->actionRemoveTransactionItem->setText(StrConstants::getRemoveItems());
    }

    menu.addAction(ui->actionRemoveTransactionItem);
    QPoint pt2 = tvTransaction->mapToGlobal(point);
    pt2.setY(pt2.y() + tvTransaction->header()->height());
    menu.exec(pt2);
  }
}

/*
 * Returns true if tabWidget height is greater than 0. Otherwise, returns false.
 */
bool MainWindow::isPropertiesTabWidgetVisible()
{
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  return (rl[1] > 0);
}

/*
 * Returns true if tvPackages height is greater than 0. Otherwise, returns false.
 */
bool MainWindow::isPackageTreeViewVisible()
{
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  return (rl[0] > 0);
}

/*
 * Selects the very first item in the tvPkgFileList treeview
 */
void MainWindow::selectFirstItemOfPkgFileList()
{
  QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
  if(tvPkgFileList)
  {
    tvPkgFileList->setFocus();
    QModelIndex maux = tvPkgFileList->model()->index(0, 0);
    tvPkgFileList->setCurrentIndex(maux);
  }
}

/*
 * Searches for the SearchBar at the tab Files. If it is found, closes it.
 */
void MainWindow::closeTabFilesSearchBar()
{
  SearchBar *searchBar = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<SearchBar*>("searchbar");
  if (searchBar)
  {
    if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    {
      searchBar->close();
    }
  }
}

/*
 * Extracts the base file name from an absolute file name.
 */
QString MainWindow::extractBaseFileName(const QString &fileName)
{
  QString baseFileName(fileName);

  if (fileName.endsWith('/'))
  {
    baseFileName.remove(baseFileName.size()-1, 1);
  }

  return baseFileName.right(baseFileName.size() - baseFileName.lastIndexOf('/') -1);
}

/*
 * Whenever user double clicks the package list items, app shows the contents of the selected package
 */
void MainWindow::onDoubleClickPackageList()
{
  const PackageRepository::PackageData* package = m_packageModel->getData(ui->tvPackages->currentIndex());
  if (package != NULL && package->installed())
  {
    refreshTabFiles(false, true);
  }
  else
  {
    refreshTabInfo(false, true);
  }
}

/*
 * Whenever user selects a package in the pkg list
 */
void MainWindow::refreshInfoAndFileTabs()
{
  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION)
    refreshTabInfo();
  else if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    refreshTabFiles();

  if(m_initializationCompleted)
    saveSettings(ectn_CurrentTabIndex);
}

/*
 * When the user changes the current selected tab, we must take care of data refresh.
 */
void MainWindow::changedTabIndex()
{
  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION)
    refreshTabInfo();
  else if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    refreshTabFiles();

#ifdef QTERMWIDGET
  else if (ui->twProperties->currentIndex() == ctn_TABINDEX_TERMINAL)
  {
    m_console->setFocus();
  }
#endif

  if(m_initializationCompleted)
    saveSettings(ectn_CurrentTabIndex);
}

/*
 * Clears the information showed on the current tab (Info or Files).
 */
void MainWindow::invalidateTabs()
{
  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION) //This is TabInfo
  {
    refreshTabInfo(false, false);
    return;
  }
  else if(ui->twProperties->currentIndex() == ctn_TABINDEX_FILES) //This is TabFiles
  {
    refreshTabFiles(); //true
    return;
  }
}

/*
 * Shows or hides the group's widget
 */
void MainWindow::hideGroupsWidget(bool pSaveSettings)
{
  static int tvPackagesWidth = ui->tvPackages->width();

  QList<int> l, rl;
  rl = ui->splitterVertical->sizes();

  if (( rl[1] != 0 ) || (!m_initializationCompleted))
  {       
    ui->splitterVertical->setSizes( l << tvPackagesWidth << 0);
    m_actionShowGroups->setChecked(false);

    ui->tvPackages->setFocus();
    ui->twGroups->setFocusPolicy(Qt::NoFocus);

    if(pSaveSettings)
      saveSettings(ectn_GROUPS);
  }
  else
  {
    //First we test if the UI doesnt have the lower pane maximized
    QList<int> splitterHoriz;
    splitterHoriz = ui->splitterHorizontal->sizes();
    if ( splitterHoriz[1] != 0 )
    {
      QList<int> savedSizes;
      savedSizes << 200 << 235;
      ui->splitterHorizontal->setSizes(savedSizes);
    }

    ui->splitterVertical->setSizes( l << tvPackagesWidth << ui->twGroups->maximumWidth() );
    ui->twGroups->setFocusPolicy(Qt::TabFocus);
    ui->twGroups->setFocus();
    m_actionShowGroups->setChecked(true);

    if(pSaveSettings)
      saveSettings(ectn_NORMAL);
  }
}

/*
 * Maximizes/de-maximizes the upper pane (tvPackages)
 */
void MainWindow::maximizePackagesTreeView(bool pSaveSettings)
{
  QList<int> savedSizes;
  savedSizes << 200 << 235;

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[1] != 0 )
  {
    ui->splitterHorizontal->setSizes( l << ui->tvPackages->maximumHeight() << 0);
    if(!ui->tvPackages->hasFocus())
      ui->tvPackages->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_MAXIMIZE_PACKAGES);
  }
  else
  {
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_NORMAL);
  }
}

/*
 * Maximizes/de-maximizes the lower pane (tabwidget)
 */
void MainWindow::maximizePropertiesTabWidget(bool pSaveSettings)
{
  QList<int> savedSizes;
  savedSizes << 200 << 235;

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[0] != 0 )
  {
    ui->splitterHorizontal->setSizes( l << 0 << ui->twProperties->maximumHeight());
    ui->twProperties->currentWidget()->childAt(1,1)->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_MAXIMIZE_PROPERTIES);
  }
  else
  {
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();

    if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    {
      QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList");
      if (tv)
        tv->scrollTo(tv->currentIndex());
    }

    if(pSaveSettings)
      saveSettings(ectn_NORMAL);
  }
}

/*
 * Whenever a user clicks on the Sort indicator of the package treeview, we keep values to mantain his choices
 */
void MainWindow::headerViewPackageListSortIndicatorClicked( int col, Qt::SortOrder order )
{
  // prevent infinite loop
  disconnect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
             SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));
  // sort
  ui->tvPackages->sortByColumn(col, order);

  // reconnect
  connect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
          SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));

  if (isAURGroupSelected())
    saveSettings(ectn_AUR_PackageList);
  else
    saveSettings(ectn_PackageList);
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void MainWindow::positionTextEditCursorAtEnd()
{
  QTextBrowser *textEdit =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");
  if (textEdit)
  {
    utils::positionTextEditCursorAtEnd(textEdit);
  }
}

/*
 * Ensures the given index tab is visible
 */
void MainWindow::ensureTabVisible(const int index)
{
  QList<int> rl = ui->splitterHorizontal->sizes();

  if(rl[1] <= 50)
  {
    rl.clear();
    rl << 200 << 235;
    ui->splitterHorizontal->setSizes(rl);

    saveSettings(ectn_NORMAL);
  }

  ui->twProperties->setCurrentIndex(index);
}

/*
 * Helper method to find the given "findText" in the Output TextEdit
 */
bool MainWindow::textInTabOutput(const QString& findText)
{
  bool res = false;
  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    res = utils::strInQTextEdit(text,findText);
  }

  return res;
}

/*
 * Helper method to find the "Synching repo..." strings
 */
bool MainWindow::IsSyncingRepoInTabOutput()
{
  bool res = false;
  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    positionTextEditCursorAtEnd();
    //We have to find at least two times, as "Synching" string will always be the first text in output
    res = text->find(StrConstants::getSyncing(), QTextDocument::FindBackward | QTextDocument::FindWholeWords);
    res = text->find(StrConstants::getSyncing(), QTextDocument::FindBackward | QTextDocument::FindWholeWords);

    if (!res)
    {
      positionTextEditCursorAtEnd();
      res = text->find("downloading", QTextDocument::FindBackward | QTextDocument::FindWholeWords);
    }

    positionTextEditCursorAtEnd();
  }

  return res;
}

/*
 * Helper method that opens an existing file using the available program/DE.
 */
void MainWindow::openFile()
{
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if (tv)
  {
    QString path = utils::showFullPathOfItem(tv->currentIndex());
    QFileInfo file(path);

    if (file.isFile())
    {
      WMHelper::openFile(path);
    }
  }
}

/*
 * Helper method that edits an existing file using the available program/DE.
 */
void MainWindow::editFile()
{
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if (tv)
  {
    QString path = utils::showFullPathOfItem(tv->currentIndex());
    WMHelper::editFile(path);
  }
}

/*
 * Helper method that opens a terminal in the selected directory, using the available program/DE.
 */
void MainWindow::openTerminal()
{
  QString dir = getSelectedDirectory();
  if (!dir.isEmpty())
  {
    WMHelper::openTerminal(dir);
  }
}

/*
 * Helper method that opens an existing directory using the available program/DE.
 */
void MainWindow::openDirectory(){
  QString dir = getSelectedDirectory();

  if (!dir.isEmpty())
  {
    WMHelper::openDirectory(dir);
  }
}

/*
 * Helper method which opens a root terminal
 */
void MainWindow::openRootTerminal()
{
  //If there are no means to run the actions, we must warn!
  if (!isSUAvailable()) return;

  m_unixCommand = new UnixCommand(this);
  m_unixCommand->openRootTerminal();
}

/*
 * Open a file chooser dialog for the user to select local packages to install (pacman -U)
 */
void MainWindow::installLocalPackage()
{
  if (!isSUAvailable()) return;

  m_packagesToInstallList =
      QFileDialog::getOpenFileNames(this,
                                    StrConstants::getFileChooserTitle(),
                                    QDir::homePath(),
                                    StrConstants::getPackages() + " (*.pkg.tar*)");

  if (m_packagesToInstallList.count() > 0)
    doInstallLocalPackages();
}

/*
 * Brings the user to the tab Files and position cursor inside searchBar
 * so he can find any file the selected package may have
 */
void MainWindow::findFileInPackage()
{
  refreshTabFiles(false, true);

  QTreeView *tb = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  SearchBar *searchBar = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");

  if (tb && tb->model()->rowCount() > 0 && searchBar)
  {
    if (searchBar)
    {
      searchBar->clear();
      searchBar->show();
    }
  }
}

/*
 * Returns the current selected directory of the FileList treeview in FilesTab
 * In case nothing is selected, returns an empty string
 */
QString MainWindow::getSelectedDirectory()
{
  QString targetDir;
  if (isPropertiesTabWidgetVisible() && ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
  {
    QTreeView *t = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
    if(t && t->currentIndex().isValid())
    {
      QString itemPath = utils::showFullPathOfItem(t->currentIndex());
      QFileInfo fi(itemPath);

      if (fi.isDir())
        targetDir = itemPath;
      else targetDir = fi.path();
    }
  }

  return targetDir;
}

/*
 * Changes the number of selected items in tvPackages: YY in XX(YY) Packages
 */
void MainWindow::tvPackagesSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  if (m_packageModel->getPackageCount() == 0)
  {
    clearStatusBar();
    return;
  }

  const QItemSelectionModel*const selection = ui->tvPackages->selectionModel();
  const int selected = selection != NULL ? selection->selectedRows().count() : 0;

  QString newMessage = StrConstants::getTotalPackages(m_packageModel->getPackageCount()) +
      " (" + StrConstants::getSelectedPackages(selected) + ") ";

  QString text;
  int numberOfInstalledPackages = m_packageModel->getInstalledPackagesCount();

  if(numberOfInstalledPackages > 0)
  {
    text = StrConstants::getNumberInstalledPackages(numberOfInstalledPackages);
  }
  else if (m_leFilterPackage->text().isEmpty() && !m_packageModel->isFiltered())
  {
    text = StrConstants::getNumberInstalledPackages(m_numberOfInstalledPackages);
  }
  else
  {
    text = StrConstants::getNumberInstalledPackages(0);
  }

  m_lblTotalCounters->setText(text);
  m_lblSelCounter->setText(newMessage);
}

/*
 * Launch Pacman Log Viewer
 */
void MainWindow::launchPLV()
{
  QProcess *proc = new QProcess();
  proc->startDetached("plv");
}

/*
 * Launch Repo Editor
 */
void MainWindow::launchRepoEditor()
{
  m_unixCommand = new UnixCommand(this);
  m_unixCommand->execCommandAsNormalUser(QLatin1String("octopi-repoeditor"));
}

/*
 * Launch Cache Cleaner
 */
void MainWindow::launchCacheCleaner()
{
  m_unixCommand = new UnixCommand(this);
  m_unixCommand->execCommandAsNormalUser(QLatin1String("octopi-cachecleaner"));
}

/*
 * Makes a gist with a bunch of system file contents.
 */
void MainWindow::ptpbSysInfo()
{
  if (!UnixCommand::hasTheExecutable("curl") ||
      m_commandExecuting != ectn_NONE) return;

  CPUIntensiveComputing *cic = new CPUIntensiveComputing(this);

  disableTransactionActions();
  QTime time = QTime::currentTime();
  qsrand(time.minute() + time.second() + time.msec());
  QFile *tempFile = new QFile(ctn_TEMP_ACTIONS_FILE + QString::number(qrand()));
  tempFile->open(QIODevice::ReadWrite|QIODevice::Text);
  tempFile->setPermissions(QFile::Permissions(QFile::ExeOwner|QFile::ReadOwner));

  QString hostname = UnixCommand::getCommandOutput("hostname");
  hostname.remove("\n");
  QString homePath = QDir::homePath();
  QByteArray out;

  if (UnixCommand::getLinuxDistro() == ectn_KAOS)
  {
    tempFile->write("----------------------------------------------------------------------------------------------------------\n");
    tempFile->write("cat /etc/KaOS-release\n");
    tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
    out = UnixCommand::getCommandOutput("cat /etc/KaOS-release");

    out.replace(hostname, "<HOSTNAME>");
    out.replace(homePath, "<HOME_PATH>");

    tempFile->write(out);
    tempFile->write("\n\n");
  }
  else
  {
    tempFile->write("----------------------------------------------------------------------------------------------------------\n");
    tempFile->write("cat /etc/lsb-release\n");
    tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
    out = UnixCommand::getCommandOutput("cat /etc/lsb-release");

    out.replace(hostname, "<HOSTNAME>");
    out.replace(homePath, "<HOME_PATH>");

    tempFile->write(out);
    tempFile->write("\n\n");
  }

  if (UnixCommand::hasTheExecutable("inxi"))
  {
    tempFile->write("----------------------------------------------------------------------------------------------------------\n");
    tempFile->write("inxi -Fxz\n");
    tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
    out = UnixCommand::getCommandOutput("inxi -Fxz -c 0");

    out.replace(hostname, "<HOSTNAME>");
    out.replace(homePath, "<HOME_PATH>");

    tempFile->write(out);
    tempFile->write("\n\n");
  }
  else
  {
    tempFile->write("----------------------------------------------------------------------------------------------------------\n");
    tempFile->write("uname -a\n");
    tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
    out = UnixCommand::getCommandOutput("uname -a");

    out.replace(hostname, "<HOSTNAME>");
    out.replace(homePath, "<HOME_PATH>");

    tempFile->write(out);
    tempFile->write("\n\n");
  }

  if (UnixCommand::hasTheExecutable("mhwd"))
  {
    tempFile->write("----------------------------------------------------------------------------------------------------------\n");
    tempFile->write("mhwd -li -d\n");
    tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
    out = UnixCommand::getCommandOutput("mhwd -li -d");

    //out.replace(hostname, "<HOSTNAME>");
    out.replace(homePath, "<HOME_PATH>");

    tempFile->write(out);
    tempFile->write("\n\n");
  }

  tempFile->write("----------------------------------------------------------------------------------------------------------\n");
  tempFile->write("journalctl -b -p err\n");
  tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
  out = UnixCommand::getCommandOutput("journalctl -b -p err");

  out.replace(hostname, "<HOSTNAME>");
  out.replace(homePath, "<HOME_PATH>");

  tempFile->write(out);
  tempFile->write("\n\n");

  tempFile->write("----------------------------------------------------------------------------------------------------------\n");
  tempFile->write("cat /etc/pacman.conf\n");
  tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
  out = UnixCommand::getCommandOutput("cat /etc/pacman.conf");

  //out.replace(hostname, "<HOSTNAME>");
  out.replace(homePath, "<HOME_PATH>");

  tempFile->write(out);
  tempFile->write("\n\n");

  tempFile->write("----------------------------------------------------------------------------------------------------------\n");
  tempFile->write("pacman -Qm\n");
  tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
  out = UnixCommand::getCommandOutput("pacman -Qm");

  //out.replace(hostname, "<HOSTNAME>");
  out.replace(homePath, "<HOME_PATH>");

  tempFile->write(out);
  tempFile->write("\n\n");

  if (UnixCommand::getLinuxDistro() == ectn_KAOS)
  {
    tempFile->write("----------------------------------------------------------------------------------------------------------\n");
    tempFile->write("cat /var/log/pacman.log\n");
    tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
    out = UnixCommand::getCommandOutput("cat /var/log/pacman.log");
    //out.replace(hostname, "<HOSTNAME>");
    out.replace(homePath, "<HOME_PATH>");
    tempFile->write(out);
    tempFile->flush();

    tempFile->write("\n\n");
    tempFile->write("----------------------------------------------------------------------------------------------------------\n");
    tempFile->write("cat /var/log/installation.log\n");
    tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
    out = UnixCommand::getCommandOutput("cat /var/log/installation.log");
    out.replace(hostname, "<HOSTNAME>");
    out.replace(homePath, "<HOME_PATH>");
    QString aux = QString::fromLatin1(out.data());
    QByteArray aba;
    aba += aux;
    tempFile->write(aba);
    tempFile->flush();
    tempFile->close();
  }
  else
  {
    tempFile->write("----------------------------------------------------------------------------------------------------------\n");
    tempFile->write("head --bytes=256K /var/log/pacman.log\n");
    tempFile->write("----------------------------------------------------------------------------------------------------------\n\n");
    out = UnixCommand::getCommandOutput("head --bytes=256K /var/log/pacman.log");
    //out.replace(hostname, "<HOSTNAME>");
    out.replace(homePath, "<HOME_PATH>");

    tempFile->write(out);
    tempFile->flush();
    tempFile->close();
  }

  enableTransactionActions();


  //Now we gist the temp file just created!
  QString ptpb = UnixCommand::getCommandOutput("curl -F c=@- https://ptpb.pw/?u=1", tempFile->fileName());
  delete cic;

  QString distroPrettyName = UnixCommand::getLinuxDistroPrettyName();
  QMessageBox::information(this, distroPrettyName + " SysInfo", Package::makeURLClickable(ptpb), QMessageBox::Ok);
}

/*
 * Opens "~/.config/octopi/octopi.conf" file for edition
 */
void MainWindow::editOctopiConf()
{
  WMHelper::editFile(SettingsManager::getOctopiConfPath(), ectn_EDIT_AS_NORMAL_USER);
}
