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
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>
#include <QTextBrowser>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  setWindowTitle(StrConstants::getApplicationName());
  setWindowIcon(QIcon(":/resources/images/octopi_yellow.png"));
  setMinimumSize(QSize(850, 600));

  initTabInfo();
  initLineEditFilterPackages();
  initPackageTreeView();
  refreshPackageList();

  initActions();

}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::initLineEditFilterPackages(){
  connect(ui->leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
}

void MainWindow::initPackageTreeView()
{
  m_proxyModelPackages = new QSortFilterProxyModel(this);
  m_modelPackages = new QStandardItemModel(this);
  m_proxyModelPackages->setSourceModel(m_modelPackages);
  m_proxyModelPackages->setFilterKeyColumn(1);

  //if (SettingsManager::instance()->getShowPackageTooltip())
  //  tvPackages->setItemDelegate(new tvPackagesItemDelegate(tvPackage));

  ui->tvPackages->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tvPackages->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  ui->tvPackages->setAllColumnsShowFocus( true );
  ui->tvPackages->setModel(m_proxyModelPackages);
  ui->tvPackages->setSortingEnabled( true );
  ui->tvPackages->sortByColumn( 1, Qt::AscendingOrder);
  ui->tvPackages->setIndentation( 0 );
  ui->tvPackages->header()->setSortIndicatorShown(true);
  ui->tvPackages->header()->setClickable(true);
  ui->tvPackages->header()->setMovable(false);
  ui->tvPackages->header()->setDefaultAlignment( Qt::AlignLeft );
  ui->tvPackages->header()->setResizeMode( QHeaderView::Fixed );
  ui->tvPackages->setStyleSheet(
        StrConstants::getTreeViewCSS(SettingsManager::getPackagesInDirFontSize()));

  //Prepare it for drag operations
  //tvPackage->setSelectionMode(QAbstractItemView::ExtendedSelection);
  //tvPackage->setDragEnabled(true);
  //dockPackages->setWindowTitle(ctn_LABEL_TREEVIEW_PACKAGES);
}

/*
 * This tab has a QTextBrowser which shows information about the selected package
 */
void MainWindow::initTabInfo(){
  QWidget *tabInfo = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout ( tabInfo );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTextBrowser *text = new QTextBrowser(tabInfo);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget ( text, 0, 0, 1, 1 );

  QString aux(tr("Info"));
  //QString translated_about = QApplication::translate ( "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 );

  ui->twProperties->removeTab(0);
  /*int tindex =*/ ui->twProperties->insertTab(0, tabInfo, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  //ui->twProperties->setTabText(ui->twProperties->indexOf(tabInfo), QApplication::translate(
  //    "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  ui->twProperties->setCurrentIndex(0);
  text->show();
  text->setFocus();
}

/*
 * Populates the list of packages available (installed [+ non-installed])
 */
void MainWindow::refreshPackageList(){
  CPUIntensiveComputing cic;

  m_modelPackages->clear();
  QStringList sl;

  QList<PackageListData> *list = Package::getPackageList();
  QStandardItem *parentItem = m_modelPackages->invisibleRootItem();
  QList<QStandardItem*> lIcons, lNames, lVersions, lRepositories;
  QList<PackageListData>::const_iterator it = list->begin();

  while(it != list->end()){
    PackageListData pld = *it;
    if (!pld.installed && !ui->actionNon_installed_pkgs->isChecked()){
      it++;
      continue;
    }

    if (pld.installed)
      lIcons << new QStandardItem(IconHelper::getIconInstalled(), "_Installed");
    else
      lIcons << new QStandardItem(IconHelper::getIconNonInstalled(), "_NonInstalled");

    lNames << new QStandardItem(pld.name);
    lVersions << new QStandardItem(pld.version);
    lRepositories << new QStandardItem(pld.repository);

    it++;
  }

  parentItem->insertColumn(0, lIcons );
  parentItem->insertColumn(1, lNames );
  parentItem->insertColumn(2, lVersions);
  parentItem->insertColumn(3, lRepositories);

  ui->tvPackages->setColumnWidth(0, 24);
  ui->tvPackages->setColumnWidth(1, 500);
  ui->tvPackages->setColumnWidth(2, 160);
  ui->tvPackages->sortByColumn(1, Qt::AscendingOrder);

  m_modelPackages->setHorizontalHeaderLabels(
        sl << "" << tr("Name") << tr("Version") << tr("Repository"));

  if (ui->leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);

  refreshTabInfo();
  ui->tvPackages->setFocus();
}

/*
 * Re-populates the HTML view with selected package's information (tab ONE)
 */
void MainWindow::refreshTabInfo(){
  if(ui->twProperties->currentIndex() != 0) return;

  static QString strSelectedPackage;
  QModelIndex item = ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME).first();
  QModelIndex mi = m_proxyModelPackages->mapToSource(item);
  QStandardItem *siName = m_modelPackages->item( mi.row(), ctn_PACKAGE_NAME );
  QStandardItem *siRepository = m_modelPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY );
  QStandardItem *siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION );

  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == siRepository->text()+"#"+siName->text()+"#"+siVersion->text())
    return;

  CPUIntensiveComputing cic;

  /* Appends all info from the selected package! */
  QString pkgName=siName->text();
  PackageInfoData pid = Package::getInformation(pkgName);
  int justification = 25;

  QString repository = StrConstants::getRepository().leftJustified(justification) + ": ";
  QString name = StrConstants::getName().leftJustified(justification) + ": ";
  QString version = StrConstants::getVersion().leftJustified(justification) + ": ";
  QString url = StrConstants::getURL().leftJustified(justification) + ": ";
  QString licenses = StrConstants::getLicenses().leftJustified(justification) + ": ";
  QString groups = StrConstants::getGroups().leftJustified(justification) + ": ";
  QString provides = StrConstants::getProvides().leftJustified(justification) + ": ";
  QString dependsOn = StrConstants::getDependsOn().leftJustified(justification) + ": ";
  QString optionalDeps = StrConstants::getOptionalDeps().leftJustified(justification) + ": ";
  QString conflictsWith = StrConstants::getConflictsWith().leftJustified(justification) + ": ";
  QString replaces = StrConstants::getReplaces().leftJustified(justification) + ": ";
  QString downloadSize = StrConstants::getDownloadSize().leftJustified(justification) + ": ";
  QString installedSize = StrConstants::getInstalledSize().leftJustified(justification) + ": ";
  QString packager = StrConstants::getPackager().leftJustified(justification) + ": ";
  QString architecture = StrConstants::getArchitecture().leftJustified(justification) + ": ";
  QString buildDate = StrConstants::getBuildDate().leftJustified(justification) + ": ";
  QString description = StrConstants::getDescription().leftJustified(justification) + ": ";

  QTextBrowser *text = ui->twProperties->widget(0)->findChild<QTextBrowser*>("textBrowser");

  if (text){
    QString valDownloadSize =
        QString("%1 KB").arg(pid.downloadSize, 6, 'f', 2);

    QString valInstalledSize =
        QString("%1 KB").arg(pid.installedSize, 6, 'f', 2);

    text->clear();
    QString anchorBegin = "anchorBegin";
    text->insertHtml("<a id=\"" + anchorBegin + "\"></a>");

    text->insertHtml("<pre>" + description + pid.description + "</pre><br>");
    text->insertHtml("<pre>" + repository + siRepository->text() + "</pre><br>");
    text->insertHtml("<pre>" + name + siName->text() + "</pre><br>");
    text->insertHtml("<pre>" + version + siVersion->text() + "</pre><br>");
    text->insertHtml("<pre>" + url + pid.url + "<pre><br>");
    text->insertHtml("<pre>" + licenses + pid.license + "</pre><br>");
    text->insertHtml("<pre>" + groups + pid.group + "</pre><br>");
    text->insertHtml("<pre>" + provides + pid.provides + "</pre><br>");
    text->insertHtml("<pre>" + dependsOn + pid.dependsOn + "</pre><br>");
    text->insertHtml("<pre>" + optionalDeps + pid.optDepends + "</pre><br>");
    text->insertHtml("<font color=\"red\"><pre>" + conflictsWith + pid.conflictsWith + "</pre></font><br>");
    text->insertHtml("<pre>" + replaces + pid.replaces + "</pre></h4><br>");
    text->insertHtml("<pre>" + downloadSize + valDownloadSize + "</pre><br>");
    text->insertHtml("<pre>" + installedSize + valInstalledSize + "</pre><br>");
    text->insertHtml("<pre>" + packager + pid.packager + "</pre><br>");
    text->insertHtml("<pre>" + architecture + pid.arch + "</pre><br>");
    text->insertHtml("<pre>" + buildDate + pid.buildDate.toString("ddd - dd/MM/yyyy hh:mm:ss") + "</pre><br>");

    text->scrollToAnchor(anchorBegin);
  }

  strSelectedPackage = siRepository->text()+"#"+siName->text()+"#"+siVersion->text();
}

/*
 * Re-populates the treeview with file list of selected package (tab TWO)
 */
void MainWindow::refreshTabFiles(){

}

/*
 * When the user changes the current selected tab, we must take care of data refresh.
 */
void MainWindow::changedTabIndex(){
  if(ui->twProperties->currentIndex() == 0)
    refreshTabInfo();
  else if (ui->twProperties->currentIndex() == 1)
    refreshTabFiles();
}

/*
 * This SLOT is called every time we press a key at FilterLineEdit
 */
void MainWindow::reapplyPackageFilter(){
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
    refreshPackageList();
  }

  /*if (numPkgs > 1)
    dockPackages->setWindowTitle(tr("%1 Packages in Directory").arg(QString::number(numPkgs)));
  else if (numPkgs == 1)
    dockPackages->setWindowTitle(tr("1 Package in Directory"));
  else
    dockPackages->setWindowTitle(tr("0 Packages in Directory"));*/

  if (isFilterPackageSelected) ui->leFilterPackage->setFocus();
  m_proxyModelPackages->sort(1, Qt::AscendingOrder);

  ui->tvPackages->selectionModel()->clear();
  QModelIndex mi = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  changedTabIndex();
}


void MainWindow::initActions(){
  connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
  connect(ui->actionNon_installed_pkgs, SIGNAL(changed()), this, SLOT(refreshPackageList()));

  //connect(ui->tvPackages, SIGNAL(entered(QModelIndex)), ui->tvPackages, SIGNAL(clicked(QModelIndex)));
  connect(ui->tvPackages, SIGNAL(activated(QModelIndex)), ui->tvPackages, SIGNAL(clicked(QModelIndex)));
  connect(ui->tvPackages, SIGNAL(clicked(QModelIndex)), this, SLOT(refreshTabInfo()));

  connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
}
