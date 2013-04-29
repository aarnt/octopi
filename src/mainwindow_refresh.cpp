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
 * This is MainWindow's refresh UI components related code
 */

#include "mainwindow.h"
#include "searchlineedit.h"
#include "ui_mainwindow.h"
#include "strconstants.h"
#include "uihelper.h"

#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QTextBrowser>
#include <QStandardItem>
#include <QSortFilterProxyModel>

/*
 * Populates the list of available packages from the given groupName
 */
void MainWindow::buildPackagesFromGroupList(const QString &groupName)
{
  m_progressWidget->show();

  if (m_cbGroups->currentIndex() == 0)
  {
    QStringList sl;
    m_modelPackagesFromGroup->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());

    sl.clear();
    m_modelInstalledPackagesFromGroup->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());

    if (m_leFilterPackage->text() != "") reapplyPackageFilter();

    if (ui->actionNonInstalledPackages->isChecked())
    {
      m_proxyModelPackages->setSourceModel(m_modelPackages);
    }
    else
    {
      m_proxyModelPackages->setSourceModel(m_modelInstalledPackages);
    }

    QModelIndex maux = m_proxyModelPackages->index(0, 0);
    ui->tvPackages->setCurrentIndex(maux);
    ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
    ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);

    refreshTabInfo();
    refreshTabFiles();
    ui->tvPackages->setFocus();

    return;
  }

  disconnect(m_pacmanDatabaseSystemWatcher,
             SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  CPUIntensiveComputing cic;

  m_modelPackagesFromGroup->clear();
  m_modelInstalledPackagesFromGroup->clear();

  QStringList sl;
  QList<QString> *list = Package::getPackagesOfGroup(groupName);

  QStandardItem *parentItemPackagesFromGroup = m_modelPackagesFromGroup->invisibleRootItem();
  QStandardItem *parentItemInstalledPackagesFromGroup = m_modelInstalledPackagesFromGroup->invisibleRootItem();

  QList<QString>::const_iterator it = list->begin();
  QList<QStandardItem*> lIcons, lNames, lVersions, lRepositories, lDescriptions;
  QList<QStandardItem*> lIcons2, lNames2, lVersions2, lRepositories2, lDescriptions2;

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);

  int counter=0;
  while(it != list->end())
  {
    QString packageName = *it;

    QStandardItem *siIcon = getAvailablePackage(packageName, ctn_PACKAGE_ICON_COLUMN);
    QStandardItem *siName = getAvailablePackage(packageName, ctn_PACKAGE_NAME_COLUMN);
    QStandardItem *siVersion = getAvailablePackage(packageName, ctn_PACKAGE_VERSION_COLUMN);
    QStandardItem *siRepository = getAvailablePackage(packageName, ctn_PACKAGE_REPOSITORY_COLUMN);
    QStandardItem *siDescription = getAvailablePackage(packageName, ctn_PACKAGE_DESCRIPTION_COLUMN);

    lIcons << siIcon;
    lNames << siName;
    lVersions << siVersion;
    lRepositories << siRepository;
    lDescriptions << siDescription;

    //If this is an INSTALLED package, we add it to the model view of installed packages!
    if (siIcon && siIcon->icon().pixmap(QSize(22,22)).toImage() !=
        IconHelper::getIconNonInstalled().pixmap(QSize(22,22)).toImage())
    {
      lIcons2 << lIcons.last()->clone();
      lNames2 << lNames.last()->clone();
      lVersions2 << lVersions.last()->clone();
      lRepositories2 << lRepositories.last()->clone();
      lDescriptions2 << lDescriptions.last()->clone();
    }

    counter++;
    m_progressWidget->setValue(counter);
    qApp->processEvents();
    it++;
  }

  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_ICON_COLUMN, lIcons);
  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_NAME_COLUMN, lNames);
  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_VERSION_COLUMN, lVersions);
  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_REPOSITORY_COLUMN, lRepositories);
  parentItemPackagesFromGroup->insertColumn(ctn_PACKAGE_DESCRIPTION_COLUMN, lDescriptions);

  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_ICON_COLUMN, lIcons2);
  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_NAME_COLUMN, lNames2);
  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_VERSION_COLUMN, lVersions2);
  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_REPOSITORY_COLUMN, lRepositories2);
  parentItemInstalledPackagesFromGroup->insertColumn(ctn_PACKAGE_DESCRIPTION_COLUMN, lDescriptions2);

  ui->tvPackages->setColumnWidth(ctn_PACKAGE_ICON_COLUMN, 24);
  ui->tvPackages->setColumnWidth(ctn_PACKAGE_NAME_COLUMN, 500);
  ui->tvPackages->setColumnWidth(ctn_PACKAGE_VERSION_COLUMN, 160);
  ui->tvPackages->header()->setSectionHidden(ctn_PACKAGE_DESCRIPTION_COLUMN, true);

  ui->tvPackages->sortByColumn(m_PackageListOrderedCol, m_PackageListSortOrder);

  m_modelPackagesFromGroup->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository() << "");

  sl.clear();
  m_modelInstalledPackagesFromGroup->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository() << "");

  if (m_leFilterPackage->text() != "") reapplyPackageFilter();

  if (ui->actionNonInstalledPackages->isChecked())
  {
    m_proxyModelPackages->setSourceModel(m_modelPackagesFromGroup);
  }
  else
  {
    m_proxyModelPackages->setSourceModel(m_modelInstalledPackagesFromGroup);
  }

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);

  list->clear();
  refreshTabInfo();
  refreshTabFiles();
  ui->tvPackages->setFocus();
  m_progressWidget->setValue(list->count());

  connect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  m_progressWidget->close();
}

/*
 * Populates the list of available packages (installed [+ non-installed])
 *
 * It's called Only: when the selected group is <All> !
 */
void MainWindow::buildPackageList()
{
  m_progressWidget->show();
  disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  static bool firstTime = true;
  timer->stop();

  CPUIntensiveComputing *cic = new CPUIntensiveComputing();

  //Refresh the list of Group names
  refreshComboBoxGroups();

  m_modelPackages->clear();
  m_modelInstalledPackages->clear();
  m_modelInstalledPackagesFromGroup->clear();
  m_modelPackagesFromGroup->clear();

  if (ui->actionNonInstalledPackages->isChecked())
  {
    m_proxyModelPackages->setSourceModel(m_modelPackages);
  }
  else
  {
    m_proxyModelPackages->setSourceModel(m_modelInstalledPackages);
  }

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

  m_progressWidget->setRange(0, list->count() + listForeign->count());
  m_progressWidget->setValue(0);

  int counter=0;
  while (itForeign != listForeign->end())
  {
    counter++;
    m_progressWidget->setValue(counter);

    PackageListData pld = PackageListData(
          itForeign->name, itForeign->repository, itForeign->version,
          itForeign->name + " " + Package::getInformationDescription(itForeign->name, true),
          ectn_FOREIGN);

    list->append(pld);
    itForeign++;
  }

  QStandardItem *parentItem = m_modelPackages->invisibleRootItem();
  QStandardItem *parentItemInstalledPackages = m_modelInstalledPackages->invisibleRootItem();
  QList<PackageListData>::const_iterator it = list->begin();
  QList<QStandardItem*> lIcons, lNames, lVersions, lRepositories, lDescriptions;
  QList<QStandardItem*> lIcons2, lNames2, lVersions2, lRepositories2, lDescriptions2;

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

    //Let's put package description in UTF-8 format
    QString pkgDescription = pld.description;
    pkgDescription = pkgDescription.fromUtf8(pkgDescription.toAscii().data());

    lDescriptions << new QStandardItem(pkgDescription);

    if (pld.repository.isEmpty())
    {
      lRepositories << new QStandardItem(StrConstants::getForeignRepositoryName());
    }
    else
      lRepositories << new QStandardItem(pld.repository);

    //If this is an INSTALLED package, we add it to the model view of installed packages!
    if (pld.status != ectn_NON_INSTALLED)
    {
      lIcons2 << lIcons.last()->clone();
      lNames2 << lNames.last()->clone();
      lVersions2 << lVersions.last()->clone();
      lRepositories2 << lRepositories.last()->clone();
      lDescriptions2 << lDescriptions.last()->clone();
    }

    counter++;
    m_progressWidget->setValue(counter);
    it++;
  }

  parentItem->insertColumn(0, lIcons);
  parentItem->insertColumn(1, lNames);
  parentItem->insertColumn(2, lVersions);
  parentItem->insertColumn(3, lRepositories);
  parentItem->insertColumn(4, lDescriptions);

  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_ICON_COLUMN, lIcons2);
  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_NAME_COLUMN, lNames2);
  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_VERSION_COLUMN, lVersions2);
  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_REPOSITORY_COLUMN, lRepositories2);
  parentItemInstalledPackages->insertColumn(ctn_PACKAGE_DESCRIPTION_COLUMN, lDescriptions2);

  ui->tvPackages->setColumnWidth(ctn_PACKAGE_ICON_COLUMN, 24);
  ui->tvPackages->setColumnWidth(ctn_PACKAGE_NAME_COLUMN, 500);
  ui->tvPackages->setColumnWidth(ctn_PACKAGE_VERSION_COLUMN, 160);
  ui->tvPackages->header()->setSectionHidden(ctn_PACKAGE_DESCRIPTION_COLUMN, true);
  ui->tvPackages->sortByColumn(m_PackageListOrderedCol, m_PackageListSortOrder);

  m_modelPackages->sort(m_PackageListOrderedCol, m_PackageListSortOrder);
  m_modelInstalledPackages->sort(m_PackageListOrderedCol, m_PackageListSortOrder);

  m_modelPackages->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository() << "");

  sl.clear();
  delete cic;

  m_modelInstalledPackages->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository() << "");

  if (m_leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);

  list->clear();
  refreshTabInfo();
  refreshTabFiles();

  if (_isPackageTreeViewVisible())
  {
    ui->tvPackages->setFocus();
  }

  //Refresh counters
  m_numberOfInstalledPackages = m_modelInstalledPackages->invisibleRootItem()->rowCount();
  m_numberOfAvailablePackages = m_modelPackages->invisibleRootItem()->rowCount() - m_numberOfInstalledPackages;

  //Refresh statusbar widget
  refreshStatusBar();

  //Refresh application icon
  refreshAppIcon();

  connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  counter = list->count() + listForeign->count();
  m_progressWidget->setValue(counter);
  m_progressWidget->close();

  if (firstTime)
  {
    if (_isPackageTreeViewVisible())
    {
      m_leFilterPackage->setFocus();
    }

    m_initializationCompleted = true;
    firstTime = false;

    if (m_callSystemUpgrade)
    {
      doSystemUpgrade();
    }
  }
}

/*
 * Decides which SLOT to call: buildPackageList or buildPackagesFromGroupList
 */
void MainWindow::metaBuildPackageList()
{
  qApp->processEvents();
  if (m_cbGroups->currentIndex() != 0)
  {
    buildPackagesFromGroupList(m_cbGroups->currentText());
  }
  else
  {
    buildPackageList();
  }
}

/*
 * Prints the values of the package counters at the right of the statusBar
 */
void MainWindow::refreshStatusBar()
{
  QString text;

  if(m_numberOfOutdatedPackages > 0)
  {
    text = " | " + StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages) +
        " | <b><font color=\"#E55451\"><a href=\"dummy\" style=\"color:\'#E55451\'\">" +
        StrConstants::getNumberOutdatedPackages().arg(m_numberOfOutdatedPackages) + "</a></font></b>";
        //StrConstants::getNumberAvailablePackages().arg(m_numberOfAvailablePackages);
  }
  else
  {
    text = "| " + StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages); // +
        //" | " + StrConstants::getNumberAvailablePackages().arg(m_numberOfAvailablePackages);
  }

  m_lblTotalCounters->setText(text);
  ui->statusBar->addWidget(m_lblTotalCounters);
}

/*
 * Re-populates the HTML view with selected package's information (tab ONE)
 */
void MainWindow::refreshTabInfo(bool clearContents, bool neverQuit)
{
  static QString strSelectedPackage;

  if(neverQuit == false &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_INFORMATION || !_isPropertiesTabWidgetVisible())) return;

  if (clearContents || ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

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
    if (m_cbGroups->currentIndex() == 0)
    {
      siIcon = m_modelPackages->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
      siName = m_modelPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
    else
    {
      siIcon = m_modelPackagesFromGroup->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
      siName = m_modelPackagesFromGroup->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelPackagesFromGroup->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelPackagesFromGroup->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
  }
  else
  {
    if (m_cbGroups->currentIndex() == 0)
    {
      siIcon = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
      siName = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
    else
    {
      siIcon = m_modelInstalledPackagesFromGroup->item( mi.row(), ctn_PACKAGE_ICON_COLUMN);
      siName = m_modelInstalledPackagesFromGroup->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelInstalledPackagesFromGroup->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelInstalledPackagesFromGroup->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
  }
  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == siRepository->text()+"#"+siName->text()+"#"+siVersion->text())
  {
    if (neverQuit)
    {
      _changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
    }

    return;
  }

  CPUIntensiveComputing cic;

  /* Appends all info from the selected package! */
  QString pkgName=siName->text();
  PackageInfoData pid;

  if (siRepository->text() != StrConstants::getForeignRepositoryName()){
    pid = Package::getInformation(pkgName);
  }
  else
  {
    pid = Package::getInformation(pkgName, true); //This is a foreign package!!!
  }

  //Let's put package description in UTF-8 format
  QString pkgDescription = pid.description;
  pkgDescription = pkgDescription.fromUtf8(pkgDescription.toAscii().data());

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

  QTextBrowser *text = ui->twProperties->widget(
        ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

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
    html += "<a style=\"font-size:16px;\">" + pkgDescription + "</a>";

    html += "<table border=\"0\">";

    html += "<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>";
    html += "<tr><td>" + url + "</td><td style=\"font-size:14px;\">" + pid.url + "</td></tr>";

    int mark = siIcon->text().indexOf('^');
    if (mark >= 0)
    {
      QString outdatedVersion = siIcon->text().right(siIcon->text().size()-mark-1);
      html += "<tr><td>" + version + "</td><td>" + siVersion->text() + " <b><font color=\"#E55451\">"
                       + StrConstants::getOutdatedInstalledVersion().arg(outdatedVersion) +
                       "</b></font></td></tr>";
    }
    else
    {
      html += "<tr><td>" + version + "</td><td>" + siVersion->text() + "</td></tr>";
    }

    //This is needed as packager names could be encoded in different charsets, resulting in an error
    QString packagerName = pid.packager;
    packagerName = packagerName.replace("<", "&lt;");
    packagerName = packagerName.replace(">", "&gt;");
    packagerName = packagerName.fromUtf8(packagerName.toAscii().data());

    html += "<tr><td>" + licenses + "</td><td>" + pid.license + "</td></tr>";

    //Show this info only if there's something to show
    if(! pid.group.contains("None"))
      html += "<tr><td>" + groups + "</td><td>" + pid.group + "</td></tr>";
    if(! pid.provides.contains("None"))
      html += "<tr><td>" + provides + "</td><td>" + pid.provides + "</td></tr>";
    if(! pid.dependsOn.contains("None"))
      html += "<tr><td>" + dependsOn + "</td><td>" + pid.dependsOn + "</td></tr>";
    if(! pid.optDepends.contains("None"))
      html += "<tr><td>" + optionalDeps + "</td><td>" + pid.optDepends + "</td></tr>";
    if(! pid.conflictsWith.contains("None"))
      html += "<tr><td><b>" + conflictsWith + "</b></td><td><b>" + pid.conflictsWith + "</b></font></td></tr>";
    if(! pid.replaces.contains("None"))
      html += "<tr><td>" + replaces + "</td><td>" + pid.replaces + "</td></tr>";

    html += "<tr><td>" + downloadSize + "</td><td>" + valDownloadSize + "</td></tr>";
    html += "<tr><td>" + installedSize + "</td><td>" + valInstalledSize + "</td></tr>";
    html += "<tr><td>" + packager + "</td><td>" + packagerName + "</td></tr>";
    html += "<tr><td>" + architecture + "</td><td>" + pid.arch + "</td></tr>";
    html += "<tr><td>" + buildDate + "</td><td>" +
        pid.buildDate.toString("ddd - dd/MM/yyyy hh:mm:ss") + "</td></tr>";

    html += "</table>";
    text->setHtml(html);
    text->scrollToAnchor(anchorBegin);
  }

  strSelectedPackage = siRepository->text()+"#"+siName->text()+"#"+siVersion->text();

  if (neverQuit)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
  }
}

/*
 * Re-populates the treeview which contains the file list of selected package (tab TWO)
 */
void MainWindow::refreshTabFiles(bool clearContents, bool neverQuit)
{
  static QString strSelectedPackage;

  if(neverQuit == false &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_FILES || !_isPropertiesTabWidgetVisible()))
  {
    return;
  }

  if (clearContents ||
      ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTreeView *tvPkgFileList =
        ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");

    if(tvPkgFileList)
    {
      QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
      modelPkgFileList->clear();
      strSelectedPackage="";

      bool filterHasFocus = m_leFilterPackage->hasFocus();
      _closeTabFilesSearchBar();
      if (filterHasFocus) m_leFilterPackage->setFocus();

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
    if (m_cbGroups->currentIndex() == 0)
    {
      siName = m_modelPackages->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelPackages->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
    else
    {
      siName = m_modelPackagesFromGroup->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelPackagesFromGroup->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelPackagesFromGroup->item(mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
  }
  else
  {
    if (m_cbGroups->currentIndex() == 0)
    {
      siName = m_modelInstalledPackages->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelInstalledPackages->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelInstalledPackages->item(mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
    else
    {
      siName = m_modelInstalledPackagesFromGroup->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
      siRepository = m_modelInstalledPackagesFromGroup->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      siVersion = m_modelInstalledPackagesFromGroup->item(mi.row(), ctn_PACKAGE_VERSION_COLUMN);
    }
  }

  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == siRepository->text()+"#"+siName->text()+"#"+siVersion->text())
  {
    if (neverQuit)
    {
      _changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
      _selectFirstItemOfPkgFileList();
    }
    else
    {
      QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
      if (tv)
        tv->scrollTo(tv->currentIndex());
    }

    return;
  }

  //Maybe this is a non-installed package...
  bool nonInstalled = ((ui->actionNonInstalledPackages->isChecked() &&
                        m_cbGroups->currentIndex() == 0) &&
                       (m_modelPackages->item(mi.row(), ctn_PACKAGE_ICON_COLUMN)->text() == "_NonInstalled"));

  if (!nonInstalled)
  {
    //Let's try another test...
    nonInstalled = ((ui->actionNonInstalledPackages->isChecked() &&
                            m_cbGroups->currentIndex() != 0) &&
                           (m_modelPackagesFromGroup->item(
                              mi.row(), ctn_PACKAGE_ICON_COLUMN)->text() == "_NonInstalled"));
  }

  QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
  if (tvPkgFileList)
  {
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

    if (nonInstalled){
      strSelectedPackage="";
      return;
    }

    CPUIntensiveComputing cic;

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

  if (neverQuit)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
    _selectFirstItemOfPkgFileList();
  }

  _closeTabFilesSearchBar();
}

/*
 * This SLOT is called every time we press a key at FilterLineEdit
 */
void MainWindow::reapplyPackageFilter()
{
  CPUIntensiveComputing cic;

  bool isFilterPackageSelected = m_leFilterPackage->hasFocus();
  QString search = Package::parseSearchString(m_leFilterPackage->text());
  QRegExp regExp(search, Qt::CaseInsensitive, QRegExp::RegExp);

  m_proxyModelPackages->setFilterRegExp(regExp);
  int numPkgs = m_proxyModelPackages->rowCount();

  if (m_leFilterPackage->text() != ""){
    if (numPkgs > 0)
      m_leFilterPackage->setFoundStyle();
    else m_leFilterPackage->setNotFoundStyle();
  }
  else{
    m_leFilterPackage->initStyleSheet();;
    m_proxyModelPackages->setFilterRegExp("");
  }

  if (isFilterPackageSelected) m_leFilterPackage->setFocus();
  m_proxyModelPackages->sort(m_PackageListOrderedCol, m_PackageListSortOrder);

  ui->tvPackages->selectionModel()->clear();
  QModelIndex mi = m_proxyModelPackages->index(0, ctn_PACKAGE_NAME_COLUMN);
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  //We need to call this method to refresh package selection counters
  tvPackagesSelectionChanged(QItemSelection(),QItemSelection());
  invalidateTabs();
}
