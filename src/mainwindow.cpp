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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "strconstants.h"
#include "package.h"
#include "uihelper.h"
#include "wmhelper.h"
#include "treeviewpackagesitemdelegate.h"
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>
#include <QTextBrowser>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QTimer>
#include <QLabel>
#include <QMessageBox>
#include <iostream>

/*
 * MainWindow's constructor: init the UI and member variables as well as remove garbage files
 */
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_PackageListOrderedCol=1;
  m_PackageListSortOrder=Qt::AscendingOrder;
  m_currentTarget=0;
  m_commandExecuting=ectn_NONE;
  m_commandQueued=ectn_NONE;

  setWindowTitle(StrConstants::getApplicationName() + " " + StrConstants::getApplicationVersion());
  setMinimumSize(QSize(850, 600));

  initStatusBar();
  initTabOutput();
  initTabInfo();
  initTabFiles();
  initTabTransaction();
  initTabHelpAbout();
  initLineEditFilterPackages();
  initPackageTreeView();
  initActions();
  initAppIcon();
  initToolBar();

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_INFORMATION);

  //Let's watch for changes in the Pacman db dir!
  m_pacmanDatabaseSystemWatcher = new QFileSystemWatcher(QStringList() << ctn_PACMAN_DATABASE_DIR, this);
  connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

  /* This timer is needed to beautify GUI initialization... */
  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(buildPackageList()));
  timer->start(5);
}

/*
 * MainWindow's destructor
 */
MainWindow::~MainWindow()
{
  //Let's garbage collect transaction files...
  m_unixCommand->removeTemporaryFiles();
  delete ui;
}

/*
 * If we have some outdated packages, let's put an angry red face icon in this app!
 */
void MainWindow::refreshAppIcon()
{
  if(m_outdatedPackageList->count() > 0)
  {
    setWindowIcon(IconHelper::getIconOctopiRed());
  }
  else
  {
    setWindowIcon(IconHelper::getIconOctopiYellow());
  }
}

/*
 * Prints the list of outdated packages to the Output tab.
 */
void MainWindow::outputOutdatedPackageList()
{
  //We cannot output any list if there is a running transaction!
  if (m_commandExecuting != ectn_NONE) return;

  m_numberOfOutdatedPackages = m_outdatedPackageList->count();

  if(m_numberOfOutdatedPackages > 0)
  {
    QString html;

    clearTabOutput();

    if(m_outdatedPackageList->count()==1){
      html += "<b>" + StrConstants::getOneOutdatedPackage() + "</b>";
    }
    else
    {
      html += "<b>" +
          StrConstants::getOutdatedPackages().arg(m_outdatedPackageList->count()) + "</b>";
    }

    html += "<br><br><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
    html += "<table border=\"0\">";
    html += "<tr><th width=\"25%\" align=\"left\">" + StrConstants::getName() +
        "</th><th width=\"18%\" align=\"right\">" +
        StrConstants::getOutdatedVersion() +
        "</th><th width=\"18%\" align=\"right\">" +
        StrConstants::getAvailableVersion() + "</th></tr>";

    for (int c=0; c < m_outdatedPackageList->count(); c++)
    {
      QString pkg = m_outdatedPackageList->at(c);
      QString outdatedVersion = getOutdatedPackageVersionByName(m_outdatedPackageList->at(c));
      QString availableVersion = getInstalledPackageVersionByName(m_outdatedPackageList->at(c));

      html += "<tr><td>" + pkg +
          "</td><td align=\"right\"><font color=\"red\">" +
          outdatedVersion +
          "</font></td><td align=\"right\">" +
          availableVersion + "</td></tr>";
    }

    writeToTabOutput(html);
    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  }
}

/*
 * Removes all text inside the TabOutput editor
 */
void MainWindow::clearTabOutput()
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    text->clear();
  }
}

/*
 * Searchs model modelInstalledPackages by a package name and returns it's OUTDATED version
 */
QString MainWindow::getOutdatedPackageVersionByName(const QString &pkgName)
{
  QList<QStandardItem *> foundItems =
      m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
  QString res;

  if (foundItems.count() > 0)
  {
    QStandardItem * si = foundItems.first();
    QStandardItem * siIcon = m_modelInstalledPackages->item(si->row(), ctn_PACKAGE_ICON_COLUMN);

    int mark = siIcon->text().indexOf('^');
    if (mark >= 0)
    {
      res = siIcon->text().right(siIcon->text().size()-mark-1);
    }
  }

  return res;
}

/*
 * Searchs model modelInstalledPackages by a package name and returns it's AVAILABLE version
 */
QString MainWindow::getInstalledPackageVersionByName(const QString &pkgName)
{
  QList<QStandardItem *> foundItems =
      m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
  QString res;

  if (foundItems.count() > 0)
  {
    QStandardItem * si = foundItems.first();
    QStandardItem * aux = m_modelInstalledPackages->item(si->row(), ctn_PACKAGE_VERSION_COLUMN);
    res = aux->text();
  }

  return res;
}

/*
 * Searchs model modelInstalledPackages by a package name and returns if it is already installed
 */
bool MainWindow::isPackageInstalled(const QString &pkgName)
{
  QList<QStandardItem *> foundItems =
      m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);

  return (foundItems.count() > 0);
}

/*
 * Populates the list of available packages (installed [+ non-installed])
 */
void MainWindow::buildPackageList()
{
  disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

  static bool firstTime = true;
  timer->stop();

  CPUIntensiveComputing cic;  
  m_modelPackages->clear();
  m_modelInstalledPackages->clear();
  QStringList sl;

  if(!firstTime) //If it's not the starting of the app...
  {
    //Let's get outdatedPackages list again!
    m_outdatedPackageList = Package::getOutdatedPackageList();
    m_numberOfOutdatedPackages = m_outdatedPackageList->count();
  }

  QStringList *unrequiredPackageList = Package::getUnrequiredPackageList();
  QList<PackageListData> *list = Package::getPackageList();
  QList<PackageListData> *listForeign = Package::getForeignPackageList();
  QList<PackageListData>::const_iterator itForeign = listForeign->begin();

  while (itForeign != listForeign->end())
  {
    list->append(*itForeign);
    itForeign++;
  }

  QStandardItem *parentItem = m_modelPackages->invisibleRootItem();
  QStandardItem *parentItemInstalledPackages = m_modelInstalledPackages->invisibleRootItem();

  QList<PackageListData>::const_iterator it = list->begin();
  QList<QStandardItem*> lIcons, lNames, lVersions, lRepositories;
  QList<QStandardItem*> lIcons2, lNames2, lVersions2, lRepositories2;

  QProgressDialog progress(StrConstants::getBuildingPackageList(), "", 0, list->count()+2, this);
  progress.setValue(0);
  progress.setMinimumDuration(10);
  progress.setCancelButton(0);
  progress.setWindowModality(Qt::WindowModal);

  int counter=0;
  while(it != list->end())
  {
    PackageListData pld = *it;
    //If this is an installed package, it can be also outdated!
    switch (pld.status)
    {
      case ectn_FOREIGN:
        lIcons << new QStandardItem(IconHelper::getIconForeign(), "_Foreign");
        break;
      case ectn_OUTDATED:
        lIcons << new QStandardItem(IconHelper::getIconOutdated(), "_OutDated^"+pld.outatedVersion);
        break;
      case ectn_INSTALLED:
        //Is this package unrequired too?
        if (unrequiredPackageList->contains(pld.name))
        {
          lIcons << new QStandardItem(IconHelper::getIconUnrequired(), "_Unrequired");
        }
        else
        {
          lIcons << new QStandardItem(IconHelper::getIconInstalled(), "_Installed");
        }
        break;
      case ectn_NON_INSTALLED:
        lIcons << new QStandardItem(IconHelper::getIconNonInstalled(), "_NonInstalled");
        break;
      default:;
    }

    lNames << new QStandardItem(pld.name);
    lVersions << new QStandardItem(pld.version);
    lRepositories << new QStandardItem(pld.repository);

    //If this is an INSTALLED package, we add it to the model view of installed packages!
    if (pld.status != ectn_NON_INSTALLED)
    {
      lIcons2 << lIcons.last()->clone();
      lNames2 << lNames.last()->clone();
      lVersions2 << lVersions.last()->clone();
      lRepositories2 << lRepositories.last()->clone();
    }

    counter++;
    progress.setValue(counter);
    qApp->processEvents();
    it++;
  }

  parentItem->insertColumn(0, lIcons);
  parentItem->insertColumn(1, lNames);
  parentItem->insertColumn(2, lVersions);
  parentItem->insertColumn(3, lRepositories);

  parentItemInstalledPackages->insertColumn(0, lIcons2);
  parentItemInstalledPackages->insertColumn(1, lNames2);
  parentItemInstalledPackages->insertColumn(2, lVersions2);
  parentItemInstalledPackages->insertColumn(3, lRepositories2);

  ui->tvPackages->setColumnWidth(0, 24);
  ui->tvPackages->setColumnWidth(1, 500);
  ui->tvPackages->setColumnWidth(2, 160);
  ui->tvPackages->sortByColumn(m_PackageListOrderedCol, m_PackageListSortOrder);

  m_modelPackages->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());

  sl.clear();
  m_modelInstalledPackages->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());

  if (ui->leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);    

  list->clear();
  refreshTabInfo();
  ui->tvPackages->setFocus();
  counter+=2;
  progress.setValue(counter);

  //Refresh counters
  m_numberOfInstalledPackages = m_modelInstalledPackages->invisibleRootItem()->rowCount(); 
  m_numberOfAvailablePackages = m_modelPackages->invisibleRootItem()->rowCount() - m_numberOfInstalledPackages;

  //Refresh statusbar widget
  refreshStatusBar();
  firstTime = false;

  //Refresh application icon
  refreshAppIcon();

  connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));
  counter++;
  progress.setValue(counter);
}

/*
 * Prints the values of the package counters at the right of the statusBar
 */
void MainWindow::refreshStatusBar()
{
  QString text;

  if(m_numberOfOutdatedPackages > 0)
  {
    text = StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages) +
        " | <font color=\"red\"><a href=\"dummy\" style=\"color:\'red\'\">" +
        StrConstants::getNumberOutdatedPackages().arg(m_numberOfOutdatedPackages) + "</a></font> | " +
        StrConstants::getNumberAvailablePackages().arg(m_numberOfAvailablePackages);
  }
  else
  {
    text = StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages) +
        " | " + StrConstants::getNumberAvailablePackages().arg(m_numberOfAvailablePackages);
  }

  m_lblCounters->setText(text);
  ui->statusBar->addPermanentWidget(m_lblCounters);
}

/*
 * Whenever the user changes the checkbox menu to show non installed packages,
 * we have to change the model from the Packages treeview...
 */
void MainWindow::changePackageListModel()
{
  QStringList sl;

  if (ui->actionNonInstalledPackages->isChecked())
  {
    m_modelPackages->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
    m_proxyModelPackages->setSourceModel(m_modelPackages);
  }
  else
  {
    m_modelInstalledPackages->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
    m_proxyModelPackages->setSourceModel(m_modelInstalledPackages);
  }

  if (ui->leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtTop);

  if (ui->leFilterPackage->text() == "")
    ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);
  else
    ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::SelectCurrent);

  changedTabIndex();
}

/*
 * Brings the context menu when the user clicks the right button above the package list
 */
void MainWindow::execContextMenuPackages(QPoint point)
{
  if(ui->tvPackages->selectionModel()->selectedRows().count() > 0)
  {
    QStandardItemModel * sim;
    bool allSameType = true;
    bool allInstallable = true;
    bool allRemovable = true;

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
      QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_ICON_COLUMN);

      if((si->icon().pixmap(QSize(22,22)).toImage()) ==
         IconHelper::getIconForeign().pixmap(QSize(22,22)).toImage())
      {
        allInstallable = false;
      }
      else if((si->icon().pixmap(QSize(22,22)).toImage()) ==
              IconHelper::getIconNonInstalled().pixmap(QSize(22,22)).toImage())
      {
        allRemovable = false;
      }
    }

    if (allSameType)
    {
      QMenu *menu = new QMenu(this);

      if(allInstallable)
      {
        menu->addAction(ui->actionInstall);
      }

      if(allRemovable)
      {
        menu->addAction(ui->actionRemove);
      }

      if(menu->actions().count() > 0)
      {
        QPoint pt2 = ui->tvPackages->mapToGlobal(point);
        pt2.setY(pt2.y() + ui->tvPackages->header()->height());
        menu->exec(pt2);
      }
    }
  }
}

/*
 * Returns true if tabWidget height is greater than 0. Otherwise, returns false
 */
bool MainWindow::_isPropertiesTabWidgetVisible()
{
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  return (rl[1] > 0);
}

/*
 * Re-populates the HTML view with selected package's information (tab ONE)
 */
void MainWindow::refreshTabInfo(bool clearContents)
{
  static QString strSelectedPackage;

  if(ui->twProperties->currentIndex() != ctn_TABINDEX_INFORMATION || !_isPropertiesTabWidgetVisible()) return;

  if (clearContents || ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");
    if (text)
    {
      text->clear();
    }

    strSelectedPackage="";
    return;
  }

  QModelIndex item = ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).first();
  QModelIndex mi = m_proxyModelPackages->mapToSource(item);

  QStandardItem *siIcon;
  QStandardItem *siName;
  QStandardItem *siRepository;
  QStandardItem *siVersion;

  if (ui->actionNonInstalledPackages->isChecked())
  {
    siIcon = m_modelPackages->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
    siName = m_modelPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
    siRepository = m_modelPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
    siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
  }
  else
  {
    siIcon = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
    siName = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
    siRepository = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
    siVersion = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
  }

  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == siRepository->text()+"#"+siName->text()+"#"+siVersion->text())
    return;

  CPUIntensiveComputing cic;

  /* Appends all info from the selected package! */
  QString pkgName=siName->text();
  PackageInfoData pid;

  if (!siRepository->text().isEmpty()){
    pid = Package::getInformation(pkgName);
  }
  else
  {
    pid = Package::getInformation(pkgName, true); //This is a foreign package!!!
  }

  //QString repository = StrConstants::getRepository();
  //QString name = StrConstants::getName();
  QString version = StrConstants::getVersion();
  QString url = StrConstants::getURL();
  QString licenses = StrConstants::getLicenses();
  QString groups = StrConstants::getGroups();
  QString provides = StrConstants::getProvides();
  QString dependsOn = StrConstants::getDependsOn();
  QString optionalDeps = StrConstants::getOptionalDeps();
  QString conflictsWith = StrConstants::getConflictsWith();
  QString replaces = StrConstants::getReplaces();
  QString downloadSize = StrConstants::getDownloadSize();
  QString installedSize = StrConstants::getInstalledSize();
  QString packager = StrConstants::getPackager();
  QString architecture = StrConstants::getArchitecture();
  QString buildDate = StrConstants::getBuildDate();
  //QString description = StrConstants::getDescription();

  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

  if (text)
  {
    QString html;
    QString valDownloadSize =
        QString("%1 KB").arg(pid.downloadSize, 6, 'f', 2);

    QString valInstalledSize =
        QString("%1 KB").arg(pid.installedSize, 6, 'f', 2);

    text->clear();
    QString anchorBegin = "anchorBegin";

    html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
    html += "<a id=\"" + anchorBegin + "\"></a>";

    html += "<h2>" + pkgName + "</h2>";
    html += "<a style=\"font-size:16px;\">" + pid.description + "</a>";

    html += "<table border=\"0\">";

    html += "<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>";
    /*html += "<tr><td>" + description + "</td><td style=\"font-size:16px;\">" +
        pid.description + "</td></tr>";*/
    html += "<tr><td>" + url + "</td><td style=\"font-size:14px;\">" + pid.url + "</td></tr>";

    //html += "<tr><td>" + repository + "</td><td>" + siRepository->text() + "</td></tr>";
    //html += "<tr><td>" + name + "</td><td>" + siName->text() + "</td></tr>";

    int mark = siIcon->text().indexOf('^');
    if (mark >= 0)
    {
      QString outdatedVersion = siIcon->text().right(siIcon->text().size()-mark-1);
      html += "<tr><td>" + version + "</td><td>" + siVersion->text() + "<b><font color=\"red\">"
                       + StrConstants::getOutdatedInstalledVersion().arg(outdatedVersion) +
                       "</b></font></td></tr>";
    }
    else
    {
      html += "<tr><td>" + version + "</td><td>" + siVersion->text() + "</td></tr>";
    }

    html += "<tr><td>" + licenses + "</td><td>" + pid.license + "</td></tr>";
    html += "<tr><td>" + groups + "</td><td>" + pid.group + "</td></tr>";
    html += "<tr><td>" + provides + "</td><td>" + pid.provides + "</td></tr>";
    html += "<tr><td>" + dependsOn + "</td><td>" + pid.dependsOn + "</td></tr>";
    html += "<tr><td>" + optionalDeps + "</td><td>" + pid.optDepends + "</td></tr>";
    html += "<tr><td><b>" /*<font color=\"red\">*/ + conflictsWith +
        "</b></td><td><b>" /*<font color=\"red\"><b>"*/ + pid.conflictsWith + "</b></font></td></tr>";
    html += "<tr><td>" + replaces + "</td><td>" + pid.replaces + "</td></tr>";
    html += "<tr><td>" + downloadSize + "</td><td>" + valDownloadSize + "</td></tr>";
    html += "<tr><td>" + installedSize + "</td><td>" + valInstalledSize + "</td></tr>";
    html += "<tr><td>" + packager + "</td><td>" + pid.packager + "</td></tr>";
    html += "<tr><td>" + architecture + "</td><td>" + pid.arch + "</td></tr>";
    html += "<tr><td>" + buildDate + "</td><td>" + pid.buildDate.toString("ddd - dd/MM/yyyy hh:mm:ss") + "</td></tr>";

    html += "</table>";

    text->setHtml(html);
    text->scrollToAnchor(anchorBegin);
  }

  strSelectedPackage = siRepository->text()+"#"+siName->text()+"#"+siVersion->text();
}

/*
 * Re-populates the treeview with file list of selected package (tab TWO)
 */
void MainWindow::refreshTabFiles(bool clearContents)
{
  static QString strSelectedPackage;

  if(ui->twProperties->currentIndex() != ctn_TABINDEX_FILES || !_isPropertiesTabWidgetVisible()) return;

  if (clearContents || ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0){
    QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
    if(tvPkgFileList)
    {
      QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
      modelPkgFileList->clear();
      strSelectedPackage="";
      return;
    }
  }

  QModelIndex item = ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).first();
  QModelIndex mi = m_proxyModelPackages->mapToSource(item);
  QStandardItem *siName;
  QStandardItem *siRepository;
  QStandardItem *siVersion;

  if (ui->actionNonInstalledPackages->isChecked())
  {
    siName = m_modelPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
    siRepository = m_modelPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
    siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
  }
  else
  {
    siName = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
    siRepository = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
    siVersion = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
  }

  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == siRepository->text()+"#"+siName->text()+"#"+siVersion->text())
    return;

  //Maybe this is a non-installed package...
  bool nonInstalled = (ui->actionNonInstalledPackages->isChecked() &&
                       (m_modelPackages->item(mi.row(), ctn_PACKAGE_ICON_COLUMN)->text() == "_NonInstalled"));

  QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
  if(tvPkgFileList){
    CPUIntensiveComputing cic;

    QString pkgName = siName->text();
    QStringList fileList = Package::getContents(pkgName);

    QStandardItemModel *fakeModelPkgFileList = new QStandardItemModel(this);
    QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
    modelPkgFileList->clear();
    QStandardItem *fakeRoot = fakeModelPkgFileList->invisibleRootItem();
    QStandardItem *root = modelPkgFileList->invisibleRootItem();
    QStandardItem *bkpDir, *item, *bkpItem=root, *parent;
    bool first=true;
    bkpDir = root;

    if(nonInstalled){
      strSelectedPackage="";
      return;
    }

    foreach ( QString file, fileList ){
      QFileInfo fi ( file );

      if(fi.isDir()){
        if ( first == true ){
          item = new QStandardItem ( IconHelper::getIconFolder(), file );
          item->setAccessibleDescription("directory " + item->text());
          fakeRoot->appendRow ( item );
        }
        else{
          if ( file.indexOf ( bkpDir->text() ) != -1 ){
            item = new QStandardItem ( IconHelper::getIconFolder(), file );
            item->setAccessibleDescription("directory " + item->text());
            bkpDir->appendRow ( item );
          }
          else{
            parent = bkpItem->parent();
            do{
              if ( parent == 0 || file.indexOf ( parent->text() ) != -1 ) break;
              parent = parent->parent();
            }
            while ( parent != fakeRoot );

            item = new QStandardItem ( IconHelper::getIconFolder(), file );
            item->setAccessibleDescription("directory " + item->text());
            if ( parent != 0 ) parent->appendRow ( item );
            else fakeRoot->appendRow ( item );
          }
        }
        bkpDir = item;
      }
      else{
        item = new QStandardItem ( IconHelper::getIconBinary(), fi.fileName() );
        item->setAccessibleDescription("file " + item->text());
        parent = bkpDir;

        do{
          if ( parent == 0 || file.indexOf ( parent->text() ) != -1 ) break;
          parent = parent->parent();
        }
        while ( parent != fakeRoot );

        parent->appendRow ( item );
      }

      bkpItem = item;
      first = false;
    }

    root = fakeRoot;
    fakeModelPkgFileList->sort(0);
    modelPkgFileList = fakeModelPkgFileList;
    tvPkgFileList->setModel(modelPkgFileList);
    tvPkgFileList->header()->setDefaultAlignment( Qt::AlignCenter );
    modelPkgFileList->setHorizontalHeaderLabels( QStringList() <<
                                                 StrConstants::getContentsOf().arg(pkgName));

    QList<QStandardItem*> lit = modelPkgFileList->findItems( "/", Qt::MatchStartsWith | Qt::MatchRecursive );

    foreach( QStandardItem* it, lit ){
      QFileInfo fi( it->text() );
      if ( fi.isFile() == false ){
        QString s( it->text() );
        s.remove(s.size()-1, 1);
        s = s.right(s.size() - s.lastIndexOf('/') -1);
        it->setText( s );
      }
    }
  }

  strSelectedPackage = siRepository->text()+"#"+siName->text()+"#"+siVersion->text();
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
}

/*
 * Clears the information showed on the current tab (Info or Files).
 */
void MainWindow::invalidateTabs()
{
  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION) //This is TabInfo
  {
    refreshTabInfo(true);
    return;
  }
  else if(ui->twProperties->currentIndex() == ctn_TABINDEX_FILES) //This is TabFiles
  {
    refreshTabFiles(true);
    return;
  }
}

/*
 * Maximizes/de-maximizes the upper pane (tvPackages)
 */
void MainWindow::maximizePackagesTreeView()
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
  }
  else
  {
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();
  }
}

/*
 * Maximizes/de-maximizes the lower pane (tabwidget)
 */
void MainWindow::maximizePropertiesTabWidget()
{
  QList<int> savedSizes;
  savedSizes << 200 << 235;

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[0] != 0 )
  {
    ui->splitterHorizontal->setSizes( l << 0 << ui->twProperties->maximumHeight());
    ui->twProperties->currentWidget()->childAt(1,1)->setFocus();
  }
  else
  {
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();
  }
}

/*
 * This SLOT is called every time we press a key at FilterLineEdit
 */
void MainWindow::reapplyPackageFilter()
{
  CPUIntensiveComputing cic;

  bool isFilterPackageSelected = ui->leFilterPackage->hasFocus();
  QString search = Package::parseSearchString(ui->leFilterPackage->text());
  QRegExp regExp(search, Qt::CaseInsensitive, QRegExp::RegExp);

  m_proxyModelPackages->setFilterRegExp(regExp);
  int numPkgs = m_proxyModelPackages->rowCount();

  if (ui->leFilterPackage->text() != ""){
    if (numPkgs > 0)
      ui->leFilterPackage->setFoundStyle();
    else ui->leFilterPackage->setNotFoundStyle();
  }
  else{
    ui->leFilterPackage->initStyleSheet();;
    m_proxyModelPackages->setFilterRegExp("");
  }

  if (isFilterPackageSelected) ui->leFilterPackage->setFocus();
  m_proxyModelPackages->sort(m_PackageListOrderedCol, m_PackageListSortOrder);

  ui->tvPackages->selectionModel()->clear();
  QModelIndex mi = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  invalidateTabs();
}

/*
 * Whenever a user clicks on the Sort indicator of the package treeview, we keep values to mantain his choices
 */
void MainWindow::headerViewPackageListSortIndicatorClicked( int col, Qt::SortOrder order )
{
  m_PackageListOrderedCol = col;
  m_PackageListSortOrder = order;
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void MainWindow::_positionTextEditCursorAtEnd()
{
  QTextBrowser *textEdit = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (textEdit){
    QTextCursor tc = textEdit->textCursor();
    tc.clearSelection();
    tc.movePosition(QTextCursor::End);
    textEdit->setTextCursor(tc);
  }
}

/*
 * Ensures that TabOutput is visible, so the user can see the messages!
 */
void MainWindow::_ensureTabOutputVisible()
{
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);

  if(rl[1] <= 50)
  {
    rl.clear();
    rl << 200 << 235;
    ui->splitterHorizontal->setSizes(rl);
  }
}

/*
 * Helper method to find the given "findText" in the Output TextEdit
 */
bool MainWindow::_textInTabOutput(const QString& findText)
{
  bool res;
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    res = text->find(findText, QTextDocument::FindBackward);
    _positionTextEditCursorAtEnd();
  }

  return res;
}

/*
 * A helper method which writes the given string to OutputTab's textbrowser
 */
void MainWindow::writeToTabOutput(const QString &msg)
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    QString newMsg = msg;
    _ensureTabOutputVisible();
    _positionTextEditCursorAtEnd();

    if(newMsg.contains(QRegExp("<font color")))
    {
      //std::cout << "Already coloured: " << newMsg.toAscii().data() << std::endl;
      newMsg += "<br>";
    }
    else
    {
      if(newMsg.contains("removing ") ||
         newMsg.contains("could not ") ||
         newMsg.contains("error"))
      {
        newMsg = "<font color=\"red\">" + newMsg + "</font><br>";
      }
      else if(newMsg.contains("checking ") ||
              newMsg.contains("installing ") ||
              newMsg.contains("upgrading ") ||
              newMsg.contains("loading ") ||
              newMsg.contains("resolving ") ||
              newMsg.contains("looking "))
       {
         newMsg = "<font color=\"green\">" + newMsg + "</font><br>";
         //std::cout << "alt: " << newMsg.toAscii().data() << std::endl;
       }
      else if (newMsg.contains("warning") ||
               newMsg.contains("-- reinstalling"))
      {
        newMsg = "<font color=\"blue\">" + newMsg + "</font><br>";
      }
      else
      {
        newMsg += "<br>";
      }
    }
    if(!newMsg.contains("<b>", Qt::CaseInsensitive))
    {
      if(newMsg.contains("::"))
      {
        newMsg = "<br><B>" + newMsg + "</B><br>";
      }
    }
    else
    {
      newMsg += "<br>";
    }

    text->insertHtml(newMsg);
    text->ensureCursorVisible();
  }
}

/*
 * Whenever the user presses DEL over the Transaction TreeView, we:
 * - Delete the package if it's bellow of "To be removed" or "To be installed" parent;
 * - Delete all the parent's packages if the user clicked in "To be removed" or "To be installed" items.
 */
void MainWindow::onPressDelete()
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());

  if (tvTransaction->hasFocus())
  {
    if(tvTransaction->currentIndex() == getRemoveTransactionParentItem()->index()){
      removePackagesFromRemoveTransaction();
    }
    else if(tvTransaction->currentIndex() == getInstallTransactionParentItem()->index()){
      removePackagesFromInstallTransaction();
    }
    else
      sim->removeRow(tvTransaction->currentIndex().row(), tvTransaction->currentIndex().parent());

    changeTransactionActionsState();
  }
}

/*
 * Helper method that opens an existing file using the available program/DE.
 */
void MainWindow::openFile(const QModelIndex& mi){
  const QStandardItemModel *sim = qobject_cast<const QStandardItemModel*>(mi.model());
  QStandardItem *si = sim->itemFromIndex(mi);

  if ((si->icon().pixmap(QSize(22,22)).toImage() ==
       IconHelper::getIconBinary().pixmap(QSize(22,22)).toImage()))
  {
    QString path = showFullPathOfObject(si->index());
    QFileInfo selectedFile(path);

    if (selectedFile.exists())
    {
      WMHelper::openFile(path);
    }
  }
}

/*
 * Returns the current selected directory of the FileList treeview in FilesTab
 * In case nothing is selected, return an empty string
 */
QString MainWindow::getSelectedDirectory()
{
  QString targetDir;

  if (_isPropertiesTabWidgetVisible() && ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
  {
    QTreeView *t = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
    if(t && t->currentIndex().isValid())
    {
      QString itemPath = showFullPathOfObject(t->currentIndex());
      QFileInfo fi(itemPath);

      if (fi.isDir())
        targetDir = itemPath;
      else targetDir = fi.path();
    }
  }

  return targetDir;
}

/*
 * Returns the full path of the selected file in any given TreeView that represents a directory path.
 */
QString MainWindow::showFullPathOfObject(const QModelIndex & index){
  if (!index.isValid()) return "";

  const QStandardItemModel *sim = qobject_cast<const QStandardItemModel*>( index.model() );

  QStringList sl;
  QModelIndex nindex;
  QString str;
  sl << sim->itemFromIndex( index )->text();
  nindex = index;

  while (1){
    nindex = sim->parent( nindex );
    if ( nindex != sim->invisibleRootItem()->index() ) sl << sim->itemFromIndex( nindex )->text();
    else break;
  }
  str = QDir::separator() + str;

  for ( int i=sl.count()-1; i>=0; i-- ){
    if ( i < sl.count()-1 ) str += QDir::separator();
    str += sl[i];
  }

  return str;
}
