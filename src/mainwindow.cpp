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
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_PackageListOrderedCol=1;
  m_PackageListSortOrder=Qt::AscendingOrder;

  setWindowTitle(StrConstants::getApplicationName());
  setMinimumSize(QSize(850, 600));

  initTabOutput();
  initTabInfo();
  initTabFiles();
  initLineEditFilterPackages();
  initPackageTreeView();

  initActions();
  initAppIcon();

  //Let's watch for changes in the pacman db dir!
  m_pacmanDatabaseSystemWatcher = new QFileSystemWatcher(QStringList() << ctn_PACMAN_DATABASE_DIR, this);
  connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

  /* This timer is needed to beautify GUI initialization... */
  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(buildPackageList()));
  timer->start(40);
}

MainWindow::~MainWindow()
{
  delete ui;
}

/*
 * If we have some outdated packages, let's put octopi in a red face angry state ;-)
 */
void MainWindow::initAppIcon()
{
  m_outdatedPackageList = Package::getOutdatedPackageList();

  if(m_outdatedPackageList->count() > 0)
  {
    setWindowIcon(QIcon(":/resources/images/octopi_red.png"));
  }
  else
  {
    setWindowIcon(QIcon(":/resources/images/octopi_yellow.png"));
  }
}

/*
 * Prints the list of outdated packages to the Output tab.
 */
void MainWindow::outputOutdatedPackageList()
{
  m_numberOfOutdatedPackages = m_outdatedPackageList->count();

  if(m_numberOfOutdatedPackages > 0)
  {
    clearTabOutput();

    if(m_outdatedPackageList->count()==1){
      writeToTabOutput(StrConstants::getOneOutdatedPackage() + "\n");
    }
    else
    {
      writeToTabOutput("<b><font color=\"red\">" +
                       StrConstants::getOutdatedPackages().arg(m_outdatedPackageList->count()) + "</font></b><br>");
    }

    for (int c=0; c < m_outdatedPackageList->count(); c++)
    {
      QString pkg = m_outdatedPackageList->at(c);
      pkg = pkg.leftJustified(40, QChar(' '));
      writeToTabOutput("<pre><font color=\"red\">" + pkg + "</font> " +
                       StrConstants::getNewVersionAvailable().arg(getInstalledPackageVersionByName(m_outdatedPackageList->at(c))) + "</pre>");
    }

    writeToTabOutput("<br>");
    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  }
}

void MainWindow::initLineEditFilterPackages(){
  connect(ui->leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
}

void MainWindow::initPackageTreeView()
{
  m_proxyModelPackages = new QSortFilterProxyModel(this);
  m_modelPackages = new QStandardItemModel(this);
  m_modelInstalledPackages = new QStandardItemModel(this);
  m_proxyModelPackages->setSourceModel(m_modelPackages);
  m_proxyModelPackages->setFilterKeyColumn(1);

  //if (SettingsManager::instance()->getShowPackageTooltip())  
  ui->tvPackages->setItemDelegate(new TreeViewPackagesItemDelegate(ui->tvPackages));

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

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_INFORMATION);
  text->show();
  text->setFocus();
}

void MainWindow::initTabFiles()
{
  QWidget *tabPkgFileList = new QWidget(this);
  QGridLayout *gridLayoutX = new QGridLayout ( tabPkgFileList );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );
  QStandardItemModel *modelPkgFileList = new QStandardItemModel(this);
  QTreeView *tvPkgFileList = new QTreeView(tabPkgFileList);
  tvPkgFileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvPkgFileList->setDropIndicatorShown(false);
  tvPkgFileList->setAcceptDrops(false);
  tvPkgFileList->header()->setSortIndicatorShown(false);
  tvPkgFileList->header()->setClickable(false);
  tvPkgFileList->header()->setMovable(false);
  tvPkgFileList->setFrameShape(QFrame::NoFrame);
  tvPkgFileList->setFrameShadow(QFrame::Plain);
  tvPkgFileList->setObjectName("tvPkgFileList");
  tvPkgFileList->setStyleSheet(StrConstants::getTreeViewCSS(SettingsManager::getPkgListFontSize()));

  modelPkgFileList->setSortRole(0);
  modelPkgFileList->setColumnCount(0);
  gridLayoutX->addWidget ( tvPkgFileList, 0, 0, 1, 1 );

  tvPkgFileList->setModel(modelPkgFileList);

  QString aux(tr("Files"));
  ui->twProperties->removeTab(1);
  /*int tindex =*/ ui->twProperties->insertTab( 1, tabPkgFileList, QApplication::translate (
                                                  "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );

  /*twTODO->setTabText(twTODO->indexOf(tabPkgFileList), QApplication::translate(
      "MainWindow", tabName.toUtf8(), 0, QApplication::UnicodeUTF8));*/

  /*tvPkgFileList->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tvPkgFileList, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(execContextMenuPkgFileList(QPoint)));
  connect(tvPkgFileList, SIGNAL(clicked (const QModelIndex&)),
          this, SLOT(showFullPathOfObject(const QModelIndex&)));
  connect(tvPkgFileList, SIGNAL(doubleClicked (const QModelIndex&)),
          this, SLOT(openFileOrDirectory(const QModelIndex&)));
  connect(tvPkgFileList, SIGNAL(pressed (const QModelIndex&)),
          tvPkgFileList, SIGNAL(clicked (const QModelIndex&)));
  connect(tvPkgFileList, SIGNAL(activated(const QModelIndex)), tvPkgFileList,
          SIGNAL(clicked(const QModelIndex)));*/
}

void MainWindow::initTabOutput()
{
  QWidget *tabOutput = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout ( tabOutput );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTextEdit *text = new QTextEdit(tabOutput);
  text->setObjectName("textOutputEdit");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  //text->setOpenExternalLinks(true);
  gridLayoutX->addWidget ( text, 0, 0, 1, 1 );

  QString aux(tr("Output"));
  //QString translated_about = QApplication::translate ( "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 );

  ui->twProperties->removeTab(2);
  /*int tindex =*/ ui->twProperties->insertTab(2, tabOutput, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  //ui->twProperties->setTabText(ui->twProperties->indexOf(tabInfo), QApplication::translate(
  //    "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  text->show();
  text->setFocus();

}

void MainWindow::clearTabOutput()
{
  QTextEdit *text = ui->twProperties->widget(2)->findChild<QTextEdit*>("textOutputEdit");
  if (text)
  {
    text->clear();
  }
}

/*
 * This method searchs model modelInstalledPackages by a package name and returns it's version
 */
QString MainWindow::getInstalledPackageVersionByName(const QString &pkgName)
{
  QList<QStandardItem *> foundItems = m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
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
 * This method searchs model modelInstalledPackages by a package name and returns if it is already installed
 */
bool MainWindow::isPackageInstalled(const QString &pkgName)
{
  QList<QStandardItem *> foundItems = m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);

  return (foundItems.count() > 0);
}

/*
 * Populates the list of available packages (installed [+ non-installed])
 */
void MainWindow::buildPackageList()
{
  timer->stop();

  CPUIntensiveComputing cic;
  m_modelPackages->clear();
  m_modelInstalledPackages->clear();
  QStringList sl;

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

  QProgressDialog progress(StrConstants::getBuildingPackageList(), "", 0, list->count(), this);
  progress.setValue(0);
  progress.setMinimumDuration(10);
  progress.setCancelButton(0);
  progress.setWindowModality(Qt::WindowModal);

  int counter=0;
  while(it != list->end())
  {
    PackageListData pld = *it;
    if (pld.status == ectn_NON_INSTALLED && !ui->actionNon_installed_pkgs->isChecked()){
      it++;
      continue;
    }

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
        sl << "" << tr("Name") << tr("Version") << tr("Repository"));

  if (ui->leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);    

  list->clear();
  refreshTabInfo();
  ui->tvPackages->setFocus();

  //Refresh counters
  m_numberOfInstalledPackages = m_modelInstalledPackages->invisibleRootItem()->rowCount();
  m_numberOfAvailablePackages = m_modelPackages->invisibleRootItem()->rowCount() - m_numberOfInstalledPackages;

  outputOutdatedPackageList();

  //Refresh statusbar widget
  refreshStatusBar();
}

/*
 * This method prints the values of the package counters at the right of the statusBar
 */
void MainWindow::refreshStatusBar()
{
  static QLabel *lblCounters = new QLabel(this);

  QString text = StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages) + " | " +
      StrConstants::getNumberOutdatedPackages().arg(m_numberOfOutdatedPackages) + " | " +
      StrConstants::getNumberAvailablePackages().arg(m_numberOfAvailablePackages);

  lblCounters->setText(text);
  ui->statusBar->addPermanentWidget(lblCounters);
}

/*
 * Whenever the user changes the checkbox menu to show non installed packages,
 * we have to change the model from the Packages treeview...
 */
void MainWindow::changePackageListModel()
{
  QStringList sl;

  if (ui->actionNon_installed_pkgs->isChecked())
  {
    m_modelPackages->setHorizontalHeaderLabels(
          sl << "" << tr("Name") << tr("Version") << tr("Repository"));
    m_proxyModelPackages->setSourceModel(m_modelPackages);
  }
  else
  {
    m_modelInstalledPackages->setHorizontalHeaderLabels(
          sl << "" << tr("Name") << tr("Version") << tr("Repository"));
    m_proxyModelPackages->setSourceModel(m_modelInstalledPackages);
  }

  if (ui->leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);

  changedTabIndex();
}

/*
 * Re-populates the HTML view with selected package's information (tab ONE)
 */
void MainWindow::refreshTabInfo(bool clearContents)
{
  static QString strSelectedPackage;

  if(ui->twProperties->currentIndex() != ctn_TABINDEX_INFORMATION) return;
  if (clearContents || ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTextBrowser *text = ui->twProperties->widget(0)->findChild<QTextBrowser*>("textBrowser");
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

  if (ui->actionNon_installed_pkgs->isChecked())
  {
    siIcon = m_modelPackages->item( mi.row(), ctn_COLUMN_PACKAGE_ICON);
    siName = m_modelPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
    siRepository = m_modelPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
    siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
  }
  else
  {
    siIcon = m_modelInstalledPackages->item( mi.row(), ctn_COLUMN_PACKAGE_ICON);
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
  QString description = StrConstants::getDescription();

  QTextBrowser *text = ui->twProperties->widget(0)->findChild<QTextBrowser*>("textBrowser");

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

    html += "<table border=\"0\">";

    html += "<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>";
    html += "<tr><td>" + description + "</td><td style=\"font-size:16px;\">" + pid.description + "</td></tr>";
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

  if(ui->twProperties->currentIndex() != ctn_TABINDEX_FILES) return;
  if (clearContents || ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0){
    QTreeView *tvPkgFileList = ui->twProperties->widget(1)->findChild<QTreeView*>("tvPkgFileList");
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

  if (ui->actionNon_installed_pkgs->isChecked())
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
  bool nonInstalled = (ui->actionNon_installed_pkgs->isChecked() &&
                       (m_modelPackages->item(mi.row(), ctn_COLUMN_PACKAGE_ICON)->text() == "_NonInstalled"));

  QTreeView *tvPkgFileList = ui->twProperties->widget(1)->findChild<QTreeView*>("tvPkgFileList");
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
      //if ( file.endsWith ( '/' ) ){
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

    //tabPkgFileList->setStatusTip(pkgName);
    QFileInfo info(pkgName);
    //QString tabName(info.fileName());

    root = fakeRoot;
    fakeModelPkgFileList->sort(0);
    modelPkgFileList = fakeModelPkgFileList;
    tvPkgFileList->setModel(modelPkgFileList);
    tvPkgFileList->header()->setDefaultAlignment( Qt::AlignCenter );
    modelPkgFileList->setHorizontalHeaderLabels( QStringList() <<
                                                 tr("Contents of \"%1\"").arg(pkgName));

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

//This method clears the current information showed on tab.
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

  /*if (numPkgs > 1)
    dockPackages->setWindowTitle(tr("%1 Packages in Directory").arg(QString::number(numPkgs)));
  else if (numPkgs == 1)
    dockPackages->setWindowTitle(tr("1 Package in Directory"));
  else
    dockPackages->setWindowTitle(tr("0 Packages in Directory"));*/

  if (isFilterPackageSelected) ui->leFilterPackage->setFocus();
  m_proxyModelPackages->sort(m_PackageListOrderedCol, m_PackageListSortOrder);

  /*disconnect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
             SLOT(changedTabIndex()));*/

  ui->tvPackages->selectionModel()->clear();
  QModelIndex mi = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  //changedTabIndex();
  invalidateTabs();

  /*connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
          SLOT(changedTabIndex()));*/
}

/*
 * Whenever a user clicks on the Sort indicator of the package treeview, we keep values to mantain his choices
 */
void MainWindow::headerViewPackageListSortIndicatorClicked( int col, Qt::SortOrder order )
{
  m_PackageListOrderedCol = col;
  m_PackageListSortOrder = order;
}

void MainWindow::initActions()
{
  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
          SLOT(invalidateTabs()));

  connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
  connect(ui->actionNon_installed_pkgs, SIGNAL(changed()), this, SLOT(changePackageListModel()));

  connect(ui->tvPackages, SIGNAL(activated(QModelIndex)), this, SLOT(changedTabIndex()));
          //ui->tvPackages, SIGNAL(clicked(QModelIndex)));
  connect(ui->tvPackages, SIGNAL(clicked(QModelIndex)), this, SLOT(changedTabIndex()));
  connect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
          SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));

  connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
}

/*
 * A helper method which writes the given string to the Output tab
 */
void MainWindow::writeToTabOutput(const QString &msg)
{
  QTextEdit *text = ui->twProperties->widget(2)->findChild<QTextEdit*>("textOutputEdit");
  if (text)
  {
    text->append(msg);
    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
    text->setFocus();
  }
}

void MainWindow::keyPressEvent(QKeyEvent* ke)
{
  if(ke->key() == Qt::Key_F5)
  {
    invalidateTabs();
    buildPackageList();
  }

  else if(ke->key() == Qt::Key_L && ke->modifiers() == Qt::ControlModifier)
  {
    ui->leFilterPackage->setFocus();
    ui->leFilterPackage->selectAll();
  }
}
