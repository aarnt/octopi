/*
* This file is part of Octopi, an open-source GUI for pacman.
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
 * This is a MainWindow's initialization code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "uihelper.h"
#include "settingsmanager.h"
#include "searchlineedit.h"
#include "treeviewpackagesitemdelegate.h"
#include <QLabel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTextBrowser>
#include <QResource>
#include <QCryptographicHash>
#include <QTextStream>
#include <QDomDocument>
#include <QFile>
#include <QComboBox>
#include <QListView>
#include <QPalette>
#include <iostream>

/*
 * Loads various application settings configured in ~/.config/octopi/octopi.conf
 */
void MainWindow::loadSettings(){
  m_PackageListOrderedCol = SettingsManager::instance()->getPackageListOrderedCol();
  m_PackageListSortOrder = (Qt::SortOrder) SettingsManager::instance()->getPackageListSortOrder();
  //m_actionIconifyOnStart->setChecked(SettingsManager::instance()->getStartIconified());

  ui->tvPackages->header()->setSortIndicator( m_PackageListOrderedCol, m_PackageListSortOrder );
  ui->tvPackages->sortByColumn( m_PackageListOrderedCol, m_PackageListSortOrder );
}

/*
 * This method only retrieve the App saved panels settings
 */
void MainWindow::loadPanelSettings(){
  int panelOrganizing = SettingsManager::instance()->getPanelOrganizing();
  switch(panelOrganizing){
    case ectn_MAXIMIZE_PACKAGES:
      maximizePackagesTreeView(false);
      break;
    case ectn_MAXIMIZE_PROPERTIES:
      maximizePropertiesTabWidget(false);
      break;
    case ectn_NORMAL:
      ui->splitterHorizontal->restoreState(SettingsManager::instance()->getSplitterHorizontalState());
      break;
  }
}

/*
 * Saves all application settings to ~/.config/octopi/octopi.conf
 */
void MainWindow::saveSettings(int saveSettingsReason){
  switch(saveSettingsReason){
    case ectn_CurrentTabIndex:
      SettingsManager::instance()->setCurrentTabIndex(ui->twProperties->currentIndex());
      break;

    case ectn_MAXIMIZE_PACKAGES:
      SettingsManager::instance()->setPanelOrganizing(ectn_MAXIMIZE_PACKAGES);
      break;

    case ectn_MAXIMIZE_PROPERTIES:
      SettingsManager::instance()->setPanelOrganizing(ectn_MAXIMIZE_PROPERTIES);
      break;

    case ectn_NORMAL:
      SettingsManager::instance()->setPanelOrganizing(ectn_NORMAL);
      SettingsManager::instance()->setSplitterHorizontalState(ui->splitterHorizontal->saveState());
      break;

    case ectn_PackageList:
      SettingsManager::instance()->setPackageListOrderedCol(m_PackageListOrderedCol);
      SettingsManager::instance()->setPackageListSortOrder(m_PackageListSortOrder);
      break;

    case ectn_IconifyOnStart:
      //SettingsManager::instance()->setStartIconified(m_actionIconifyOnStart->isChecked());
      break;
  }
}

/*
 * If we have some outdated packages, let's put octopi in a red face/angry state ;-)
 */
void MainWindow::initAppIcon()
{
  m_outdatedPackageList = Package::getOutdatedPackageList();
  m_numberOfOutdatedPackages = m_outdatedPackageList->count();
  refreshAppIcon();
}

/*
 * Inserts the group names into the Groups comboBox
 */
void MainWindow::refreshComboBoxGroups()
{
  disconnect(m_cbGroups, SIGNAL(currentIndexChanged(QString)), this, SLOT(metaBuildPackageList()));

  m_cbGroups->clear();
  m_cbGroups->addItem("<" + StrConstants::getAll() + ">");
  m_cbGroups->addItems(*Package::getPackageGroups());

  connect(m_cbGroups, SIGNAL(currentIndexChanged(QString)), this, SLOT(metaBuildPackageList()));
}

/*
 * Inits the Groups combobox, so it can be added in app's toolBar
 */
void MainWindow::initComboBoxGroups()
{
  m_cbGroups = new QComboBox(this);
  m_lvGroups = new QListView(m_cbGroups);
  m_cbGroups->setMinimumWidth(200);
  m_cbGroups->setAutoCompletion(true);
  m_cbGroups->setIconSize(QSize(0, 0));
  m_cbGroups->setView(m_lvGroups);
  m_cbGroups->setMaxVisibleItems(15);
  m_cbGroups->setStyleSheet(StrConstants::getMenuCSS());

  connect(m_cbGroups, SIGNAL(currentIndexChanged(QString)), this, SLOT(metaBuildPackageList()));
}

/*
 * Inits the toolbar, including taking that annoying default view action out of the game
 */
void MainWindow::initToolBar()
{
  initComboBoxGroups();
  qApp->setStyleSheet(StrConstants::getMenuCSS());

  ui->mainToolBar->addAction(ui->actionSyncPackages);
  ui->mainToolBar->addAction(ui->actionCommit);
  ui->mainToolBar->addAction(ui->actionRollback);

  m_leFilterPackage->setMinimumHeight(24);
  ui->mainToolBar->addWidget(m_leFilterPackage);
  ui->mainToolBar->addWidget(m_cbGroups);

  QWidget * hSpacer = new QWidget(this);
  hSpacer->setMinimumHeight(22);
  hSpacer->setMinimumWidth(6);
  hSpacer->setVisible(true);
  ui->mainToolBar->addWidget(hSpacer);

  //ui->mainToolBar->addAction(ui->actionExit);

  ui->mainToolBar->toggleViewAction()->setEnabled(false);
  ui->mainToolBar->toggleViewAction()->setVisible(false);
}

/*
 * The only thing needed here is to create a dynamic label which will contain the package counters
 */
void MainWindow::initStatusBar()
{
  m_lblSelCounter = new QLabel(this);
  m_lblTotalCounters = new QLabel(this);

  ui->statusBar->addWidget(m_lblSelCounter);
  connect(m_lblTotalCounters, SIGNAL(linkActivated(QString)), this, SLOT(outputOutdatedPackageList()));
}

/*
 * Sets the TabWidget Properties to the given index/tab and change app focus to its child widget
 */
void MainWindow::_changeTabWidgetPropertiesIndex(const int newIndex)
{
  int oldTabIndex = ui->twProperties->currentIndex();
  _ensureTabVisible(newIndex);

  if (newIndex == oldTabIndex)
  {
    if (oldTabIndex == ctn_TABINDEX_INFORMATION)
    {
      refreshTabInfo();
    }
    else if (oldTabIndex == ctn_TABINDEX_FILES)
    {
      refreshTabFiles();
    }

    ui->twProperties->currentWidget()->childAt(1,1)->setFocus();
  }
  /*else if (newIndex == ctn_TABINDEX_FILES)
  {
    QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
    if(tvPkgFileList)
    {
      tvPkgFileList->setFocus();
      tvPkgFileList->setCurrentIndex(tvPkgFileList->currentIndex());
    }
  }*/
  else
  {
    //For any other tab... just doing the following is enough
    ui->twProperties->currentWidget()->childAt(1,1)->setFocus();
  }
}

/*
 * Sets the current tabWidget index, based on last user session
 */
void MainWindow::initTabWidgetPropertiesIndex()
{
  ui->twProperties->setCurrentIndex(SettingsManager::getCurrentTabIndex());
}

/*
 * This is the 4th tab (Transaction).
 * It pops up whenever the user selects a remove/install action on a selected package
 */
void MainWindow::initTabTransaction()
{
  m_modelTransaction = new QStandardItemModel(this);
  QWidget *tabTransaction = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabTransaction);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setMargin(0);

  QTreeView *tvTransaction = new QTreeView(tabTransaction);
  tvTransaction->setObjectName("tvTransaction");
  tvTransaction->setContextMenuPolicy(Qt::CustomContextMenu);
  tvTransaction->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvTransaction->setDropIndicatorShown(false);
  tvTransaction->setAcceptDrops(false);
  tvTransaction->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tvTransaction->setItemDelegate(new TreeViewPackagesItemDelegate(tvTransaction));
  tvTransaction->header()->setSortIndicatorShown(false);
  tvTransaction->header()->setClickable(false);
  tvTransaction->header()->setMovable(false);
  tvTransaction->setFrameShape(QFrame::NoFrame);
  tvTransaction->setFrameShadow(QFrame::Plain);
  tvTransaction->setStyleSheet(StrConstants::getTreeViewCSS()); //SettingsManager::getPkgListFontSize()));
  tvTransaction->expandAll();

  m_modelTransaction->setSortRole(0);
  m_modelTransaction->setColumnCount(0);

  QStringList sl;
  m_modelTransaction->setHorizontalHeaderLabels(sl << StrConstants::getPackages());

  QStandardItem *siToBeRemoved = new QStandardItem(IconHelper::getIconToRemove(),
                                                   StrConstants::getTransactionRemoveText());
  QStandardItem *siToBeInstalled = new QStandardItem(IconHelper::getIconToInstall(),
                                                     StrConstants::getTransactionInstallText());

  m_modelTransaction->appendRow(siToBeRemoved);
  m_modelTransaction->appendRow(siToBeInstalled);

  gridLayoutX->addWidget(tvTransaction, 0, 0, 1, 1);

  tvTransaction->setModel(m_modelTransaction);

  QString aux(StrConstants::getTabTransactionName());

  ui->twProperties->removeTab(ctn_TABINDEX_TRANSACTION);
  ui->twProperties->insertTab(ctn_TABINDEX_TRANSACTION, tabTransaction, QApplication::translate (
                                "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ));

  connect(tvTransaction, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(execContextMenuTransaction(QPoint)));
  connect(tvTransaction->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(tvTransactionSelectionChanged(QItemSelection,QItemSelection)));

  connect(tvTransaction->model(), SIGNAL(rowsInserted ( const QModelIndex , int, int )),
          this, SLOT(tvTransactionRowsInserted(QModelIndex,int,int)));
  connect(tvTransaction->model(), SIGNAL(rowsRemoved ( const QModelIndex , int, int )),
          this, SLOT(tvTransactionRowsRemoved(QModelIndex,int,int)));
}

/*
 * This is the LineEdit widget used to filter the package list
 */
void MainWindow::initLineEditFilterPackages(){
  connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
}

/*
 * This is the package treeview, it lists the installed [and not installed] packages in the system
 */
void MainWindow::initPackageTreeView()
{
  m_proxyModelPackages = new QSortFilterProxyModel(this);
  m_modelPackages = new QStandardItemModel(this);
  m_modelInstalledPackages = new QStandardItemModel(this);
  m_modelPackagesFromGroup = new QStandardItemModel(this);
  m_modelInstalledPackagesFromGroup = new QStandardItemModel(this);

  m_proxyModelPackages->setSourceModel(m_modelPackages);
  m_proxyModelPackages->setFilterKeyColumn(1);
  ui->tvPackages->setItemDelegate(new TreeViewPackagesItemDelegate(ui->tvPackages));
  ui->tvPackages->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->tvPackages->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
        StrConstants::getTreeViewCSS()); //SettingsManager::getPackagesInDirFontSize()));

  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(tvPackagesSelectionChanged(QItemSelection,QItemSelection)));
  connect(ui->tvPackages, SIGNAL(activated(QModelIndex)), this, SLOT(changedTabIndex()));
  connect(ui->tvPackages, SIGNAL(clicked(QModelIndex)), this, SLOT(changedTabIndex()));
  connect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
          SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));
  connect(ui->tvPackages, SIGNAL(customContextMenuRequested(QPoint)), this,
          SLOT(execContextMenuPackages(QPoint)));
  connect(ui->tvPackages, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickPackageList()));

  //Prepare it for drag operations
  //tvPackage->setDragEnabled(true);
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

  QString aux(StrConstants::getTabInfoName());
  //QString translated_about = QApplication::translate ( "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 );

  ui->twProperties->removeTab(ctn_TABINDEX_INFORMATION);
  /*int tindex =*/ ui->twProperties->insertTab(ctn_TABINDEX_INFORMATION, tabInfo, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  //ui->twProperties->setTabText(ui->twProperties->indexOf(tabInfo), QApplication::translate(
  //    "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_INFORMATION);
  text->show();
  text->setFocus();
}

/*
 * This is the files treeview, which shows the directory structure of ONLY installed packages's files.
 */
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
  tvPkgFileList->setStyleSheet(StrConstants::getTreeViewCSS()); //SettingsManager::getPkgListFontSize()));

  modelPkgFileList->setSortRole(0);
  modelPkgFileList->setColumnCount(0);
  gridLayoutX->addWidget(tvPkgFileList, 0, 0, 1, 1);
  tvPkgFileList->setModel(modelPkgFileList);

  QString aux(StrConstants::getTabFilesName());
  ui->twProperties->removeTab(ctn_TABINDEX_FILES);
  ui->twProperties->insertTab(ctn_TABINDEX_FILES, tabPkgFileList, QApplication::translate (
                                                  "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );

  /*twTODO->setTabText(twTODO->indexOf(tabPkgFileList), QApplication::translate(
      "MainWindow", tabName.toUtf8(), 0, QApplication::UnicodeUTF8));*/

  tvPkgFileList->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(tvPkgFileList, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(execContextMenuPkgFileList(QPoint)));
  //connect(tvPkgFileList, SIGNAL(clicked (const QModelIndex&)),
  //        this, SLOT(showFullPathOfObject(const QModelIndex&)));

  connect(tvPkgFileList, SIGNAL(doubleClicked (const QModelIndex&)),
          this, SLOT(openFile()));
  //connect(tvPkgFileList, SIGNAL(activated(const QModelIndex)),
  //        this, SLOT(openFileOrDirectory(QModelIndex)));

  //connect(tvPkgFileList, SIGNAL(pressed (const QModelIndex&)),
  //        tvPkgFileList, SIGNAL(clicked (const QModelIndex&)));
}

/*
 * Retrieves the distro RSS news feed from its respective site
 * If it fails to connect to the internet, uses the available "./.config/octopi/distro_rss.xml"
 * The result is a QString containing the RSS News Feed XML code
 */
QString MainWindow::retrieveDistroNews(bool searchForLatestNews)
{
  const QString ctn_ARCH_LINUX_RSS = "https://www.archlinux.org/feeds/news/";
  const QString ctn_MANJARO_LINUX_RSS = "http://manjaro.org/feed/";
  LinuxDistro distro = UnixCommand::getLinuxDistro();

  QString res;
  QString tmpRssPath = QDir::homePath() + QDir::separator() + ".config/octopi/.tmp_distro_rss.xml";
  QString rssPath = QDir::homePath() + QDir::separator() + ".config/octopi/distro_rss.xml";
  QString contentsRss;

  QFile fileRss(rssPath);
  if (fileRss.exists())
  {
    if (!fileRss.open(QIODevice::ReadOnly | QIODevice::Text)) res = "";
    QTextStream in2(&fileRss);
    contentsRss = in2.readAll();
    fileRss.close();
  }

  if(searchForLatestNews && UnixCommand::hasInternetConnection() && distro != ectn_UNKNOWN)
  {
    QString curlCommand = "curl %1 -o %2";

    if (distro == ectn_ARCHLINUX)
    {
      curlCommand = curlCommand.arg(ctn_ARCH_LINUX_RSS).arg(tmpRssPath);
    }
    else if (distro == ectn_MANJAROLINUX)
    {
      curlCommand = curlCommand.arg(ctn_MANJARO_LINUX_RSS).arg(tmpRssPath);
    }

    if (UnixCommand::runCurlCommand(curlCommand) == 0)
    {
      QFile fileTmpRss(tmpRssPath);
      QFile fileRss(rssPath);

      if (!fileRss.exists())
      {
        fileTmpRss.rename(tmpRssPath, rssPath);

        if (!fileRss.open(QIODevice::ReadOnly | QIODevice::Text)) res = "";
        QTextStream in2(&fileRss);
        contentsRss = in2.readAll();
        fileRss.close();

        res = contentsRss;
      }
      else
      {
        //A rss file already exists. We have to make a SHA1 hash to compare the contents
        QString tmpRssSHA1;
        QString rssSHA1;
        QString contentsTmpRss;

        QFile fileTmpRss(tmpRssPath);
        if (!fileTmpRss.open(QIODevice::ReadOnly | QIODevice::Text)) res = "";
        QTextStream in(&fileTmpRss);
        contentsTmpRss = in.readAll();
        fileTmpRss.close();

        tmpRssSHA1 = QCryptographicHash::hash(contentsTmpRss.toAscii(), QCryptographicHash::Sha1);
        rssSHA1 = QCryptographicHash::hash(contentsRss.toAscii(), QCryptographicHash::Sha1);

        if (tmpRssSHA1 != rssSHA1){
          fileRss.remove();
          fileTmpRss.rename(tmpRssPath, rssPath);

          res = "*" + contentsTmpRss; //The asterisk indicates there is a MORE updated rss!
        }
        else
        {
          fileTmpRss.remove();
          res = contentsRss;
        }
      }
    }
  }
  //Either we don't have internet or we weren't asked to retrieve the latest news
  else
  {
    QFile fileRss(rssPath);

    //Maybe we have a file in "./.config/octopi/distro_rss.xml"
    if (fileRss.exists())
    {
      res = contentsRss;
    }
    else if (searchForLatestNews)
    {
      res = "<h3><font color=\"#E55451\">" + StrConstants::getInternetUnavailableError() + "</font></h3>";
    }
    else if (distro != ectn_UNKNOWN)
    {
      res = "<h3><font color=\"#E55451\">" + StrConstants::getNewsErrorMessage() + "</font></h3>";
    }
    else
    {
      res = "<h3><font color=\"#E55451\">" + StrConstants::getIncompatibleLinuxDistroError() + "</font></h3>";
    }
  }

  return res;
}

/*
 * Parses the raw XML contents from the Distro RSS news feed
 * Creates and returns a string containing a HTML code with latest 10 news
 */
QString MainWindow::parseDistroNews()
{
  QString html;

  LinuxDistro distro = UnixCommand::getLinuxDistro();
  if (distro == ectn_ARCHLINUX)
  {
    html = "<p align=\"center\"><h2>" + StrConstants::getArchLinuxNews() + "</h2></p><ul>";
  }
  else if (distro == ectn_MANJAROLINUX)
  {
    html = "<p align=\"center\"><h2>" + StrConstants::getManjaroLinuxNews() + "</h2></p><ul>";
  }

  QString lastBuildDate;
  QString rssPath = QDir::homePath() + QDir::separator() + ".config/octopi/distro_rss.xml";
  QDomDocument doc("rss");
  int itemCounter=0;

  QFile file(rssPath);
  if (!file.open(QIODevice::ReadOnly)) return "";
  if (!doc.setContent(&file)) {
      file.close();
      return "";
  }
  file.close();

  QDomElement docElem = doc.documentElement(); //This is rss
  QDomNode n = docElem.firstChild(); //This is channel
  n = n.firstChild();

  while(!n.isNull()) {
    QDomElement e = n.toElement();

    if(!e.isNull())
    {
      if (e.tagName() == "lastBuildDate")
      {
        lastBuildDate = e.text();
      }
      else if(e.tagName() == "item")
      {
        //Let's iterate over the 10 lastest "item" news
        if (itemCounter == 10) break;

        QDomNode text = e.firstChild();

        QString itemTitle;
        QString itemLink;
        QString itemDescription;
        QString itemPubDate;

        while(!text.isNull())
        {
          QDomElement eText = text.toElement();

          if(!eText.isNull())
          {
            if (eText.tagName() == "title")
            {
              itemTitle = "<h3>" + eText.text() + "</h3>";
            }
            else if (eText.tagName() == "link")
            {
              itemLink = Package::makeURLClickable(eText.text());
              if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX) itemLink += "<br>";
            }
            else if (eText.tagName() == "description")
            {
              itemDescription = eText.text();
              itemDescription = itemDescription.remove(QRegExp("\\n"));
              itemDescription += "<br>";
            }
            else if (eText.tagName() == "pubDate")
            {
              itemPubDate = eText.text();
              itemPubDate = itemPubDate.remove(QRegExp("\\n"));
              int pos = itemPubDate.indexOf("+");

              if (pos > -1)
              {
                itemPubDate = itemPubDate.mid(0, pos-1).trimmed() + "<br>";
              }
            }
          }

          text = text.nextSibling();
        }

        html += "<li><p>" + itemTitle + " " + itemPubDate + "<br>" + itemLink + itemDescription + "</p></li>";
        itemCounter++;
      }
    }

    n = n.nextSibling();
  }

  html += "</ul>";
  return html;
}

/*
 * This is the high level method that orquestrates the Distro RSS News printing in tabNews
 *
 * boolean parameter searchForLatestNews:
 *    controls whether this method tries to connect to the remote RSS News Feed (default is true)
 * boolean parameter gotoNewsTab:
 *    controls whether this method must set the mainwindow focus to NewsTab (default is true)
 */
void MainWindow::refreshDistroNews(bool searchForLatestNews, bool gotoNewsTab)
{
  qApp->processEvents();

  if (searchForLatestNews)
  {
    LinuxDistro distro = UnixCommand::getLinuxDistro();

    clearTabOutput();

    if (distro == ectn_ARCHLINUX)
    {
      writeToTabOutput("<b>" + StrConstants::getSearchingForArchLinuxNews() + "</b>");
    }
    else if (distro == ectn_MANJAROLINUX)
    {
      writeToTabOutput("<b>" + StrConstants::getSearchingForManjaroLinuxNews() + "</b>");
    }

    qApp->processEvents();
  }

  CPUIntensiveComputing cic;
  QString distroRSSXML = retrieveDistroNews(searchForLatestNews);
  QString html;

  if (distroRSSXML.count() >= 200)
  {
    if (distroRSSXML.at(0)=='*' && gotoNewsTab)
    {
      //If this is an updated RSS, we must warn the user!
      ui->twProperties->setTabText(ctn_TABINDEX_NEWS, "** " + StrConstants::getTabNewsName() + " **");
      ui->twProperties->setCurrentIndex(ctn_TABINDEX_NEWS);
    }
    else
    {
      if(searchForLatestNews)
        ui->twProperties->setTabText(ctn_TABINDEX_NEWS, StrConstants::getTabNewsName());
    }

    //First, we have to parse the raw RSS XML...
    html = parseDistroNews();
  }
  else
  {
    if(searchForLatestNews)
      ui->twProperties->setTabText(ctn_TABINDEX_NEWS, StrConstants::getTabNewsName());

    html = distroRSSXML;
  }

  //Now that we have the html table code, let's put it into TextBrowser's News tab
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_NEWS)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    text->clear();
    text->setHtml(html);
  }

  clearTabOutput();
  qApp->processEvents();

  if (searchForLatestNews && gotoNewsTab)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_NEWS);
  }
}

/*
 * This SLOT is called every time the user clicks a url inside newsTab textBrowser
 */
void MainWindow::onTabNewsSourceChanged(QUrl newSource)
{
  if(newSource.isRelative())
  {
    //If the user clicked a relative and impossible to display link...
    QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_NEWS)->findChild<QTextBrowser*>("textBrowser");
    if (text)
    {
      disconnect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));
      text->setHtml(parseDistroNews());
      connect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));
    }
  }
}

/*
 * This is the TextBrowser News tab, which shows the latest news from Distro news feed
 */
void MainWindow::initTabNews()
{
  QString aux(StrConstants::getTabNewsName());
  QWidget *tabNews = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabNews);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setMargin(0);

  QTextBrowser *text = new QTextBrowser(tabNews);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget(text, 0, 0, 1, 1);

  text->show();

  int tindex = ui->twProperties->insertTab(ctn_TABINDEX_NEWS, tabNews, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  ui->twProperties->setTabText(ui->twProperties->indexOf(tabNews), QApplication::translate(
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  //QWidget *w = m_tabBar->tabButton(tindex, QTabBar::RightSide);
  //w->setToolTip(tr("Close tab"));
  //w->setObjectName("toolButton");

  //SearchBar *searchBar = new SearchBar(this);
  //MyHighlighter *highlighter = new MyHighlighter(text, "");

  /*
  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findNextButtonClicked()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPreviousButtonClicked()), this, SLOT(searchBarFindPrevious()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);
  gridLayoutX->addWidget(new SyntaxHighlighterWidget(this, highlighter));
  */

  connect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));

  text->show();
  ui->twProperties->setCurrentIndex(tindex);
  text->setFocus();
}

/*
 * This is the TextEdit output tab, which shows the output of pacman commands.
 */
void MainWindow::initTabOutput()
{
  QWidget *tabOutput = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabOutput);
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTextBrowser *text = new QTextBrowser(tabOutput);
  text->setObjectName("textOutputEdit");
  text->setReadOnly(true);
  text->setOpenExternalLinks(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  gridLayoutX->addWidget (text, 0, 0, 1, 1);

  QString aux(StrConstants::getTabOutputName());
  //QString translated_about = QApplication::translate ( "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 );

  ui->twProperties->removeTab(ctn_TABINDEX_OUTPUT);
  ui->twProperties->insertTab(ctn_TABINDEX_OUTPUT, tabOutput, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  //ui->twProperties->setTabText(ui->twProperties->indexOf(tabInfo), QApplication::translate(
  //    "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  text->show();
  text->setFocus();
}

/*
 * Initialize the Help tab with basic information about using Octopi
 */
void MainWindow::initTabHelpAbout()
{
  QString aux(StrConstants::getHelp());
  QWidget *tabHelpAbout = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabHelpAbout);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setMargin(0);

  QTextBrowser *text = new QTextBrowser(tabHelpAbout);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget(text, 0, 0, 1, 1);

  QString url = "qrc:/resources/help/help_" + QLocale::system().name() + ".html";
  QResource resource(url);

  if(resource.isValid())
  {
    text->setSource(QUrl(url));
  }
  else
  {
    url = "qrc:/resources/help/help_en_US.html";
    text->setSource(QUrl(url));
  }

  text->show();

  int tindex = ui->twProperties->addTab(tabHelpAbout, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  ui->twProperties->setTabText(ui->twProperties->indexOf(tabHelpAbout), QApplication::translate(
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  //QWidget *w = m_tabBar->tabButton(tindex, QTabBar::RightSide);
  //w->setToolTip(tr("Close tab"));
  //w->setObjectName("toolButton");

  //SearchBar *searchBar = new SearchBar(this);
  //MyHighlighter *highlighter = new MyHighlighter(text, "");

  /*
  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findNextButtonClicked()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPreviousButtonClicked()), this, SLOT(searchBarFindPrevious()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);
  gridLayoutX->addWidget(new SyntaxHighlighterWidget(this, highlighter));
  */

  text->show();
  ui->twProperties->setCurrentIndex(tindex);
  text->setFocus();
}

/*
 * Slot to position twProperties at Help About tab
 */
void MainWindow::onHelpAbout()
{
  //if (_isPropertiesTabWidgetVisible())
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_HELPABOUT);
  }
}

/*
 * Initialize QAction objects
 */
void MainWindow::initActions()
{
  ui->actionCommit->setEnabled(false);
  ui->actionRollback->setEnabled(false);

  ui->actionOpenDirectory->setIcon(IconHelper::getIconFolder());

  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(invalidateTabs()));

  connect(ui->actionRemoveTransactionItem, SIGNAL(triggered()), this, SLOT(onPressDelete()));
  connect(ui->actionRemoveTransactionItems, SIGNAL(triggered()), this, SLOT(onPressDelete()));
  connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
  connect(ui->actionNonInstalledPackages, SIGNAL(changed()), this, SLOT(changePackageListModel()));
  connect(ui->actionSyncPackages, SIGNAL(triggered()), this, SLOT(doSyncDatabase()));
  connect(ui->actionSystemUpgrade, SIGNAL(triggered()), this, SLOT(doSystemUpgrade()));
  connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(insertIntoRemovePackage()));
  connect(ui->actionInstall, SIGNAL(triggered()), this, SLOT(insertIntoInstallPackage()));
  connect(ui->actionRemoveGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoRemovePackage()));
  connect(ui->actionInstallGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoInstallPackage()));
  connect(ui->actionCommit, SIGNAL(triggered()), this, SLOT(doCommitTransaction()));
  connect(ui->actionRollback, SIGNAL(triggered()), this, SLOT(doRollbackTransaction()));
  connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(onHelpAbout()));
  connect(ui->actionGetNews, SIGNAL(triggered()), this, SLOT(refreshDistroNews()));

  connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));

  //Actions from tvPkgFileList context menu
  connect(ui->actionCollapseAllItems, SIGNAL(triggered()), this, SLOT(collapseAllContentItems()));
  connect(ui->actionExpandAllItems, SIGNAL(triggered()), this, SLOT(expandAllContentItems()));
  connect(ui->actionCollapseItem, SIGNAL(triggered()), this, SLOT(collapseThisContentItems()));
  connect(ui->actionExpandItem, SIGNAL(triggered()), this, SLOT(expandThisContentItems()));
  connect(ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
  connect(ui->actionEditFile, SIGNAL(triggered()), this, SLOT(editFile()));
  connect(ui->actionOpenDirectory, SIGNAL(triggered()), this, SLOT(openDirectory()));
  connect(ui->actionOpenTerminal, SIGNAL(triggered()), this, SLOT(openTerminal()));
}
