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
#include "searchbar.h"
#include "utils.h"
#include "globals.h"
#include <iostream>
#include "optionsdialog.h"
#include "termwidget.h"
#include "aurvote.h"
#include "alpmbackend.h"

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
#include <QRandomGenerator>
#include <QTcpServer>
#include <QTcpSocket>
#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/*
 * MainWindow's constructor: basic UI init
 */
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent), ui(new Ui::MainWindow), m_packageModel(new PackageModel(m_packageRepo))
{
  m_hasForeignTool =
      UnixCommand::hasTheExecutable(Package::getForeignRepositoryToolName());

  m_packageRepo.registerDependency(*m_packageModel);
  m_foundFilesInPkgFileList = new QList<QModelIndex>();
  m_indFoundFilesInPkgFileList = 0;
  m_callSystemUpgrade = false;
  m_callSystemUpgradeNoConfirm = false;
  m_initializationCompleted=false;
  m_systemUpgradeDialog = false;
  m_refreshPackageLists = false;
  m_refreshForeignPackageList = false;
  m_cic = nullptr;
  m_ignoredStringList = Package::extractIgnorePkgList();
  m_outdatedStringList = new QStringList();
  m_checkupdatesStringList = new QStringList();
  m_checkUpdatesNameNewVersion = new QHash<QString, QString>();
  m_outdatedAURStringList = new QStringList();
  m_outdatedAURPackagesNameVersion = new QHash<QString, QString>();
  m_selectedViewOption = ectn_ALL_PKGS;
  m_selectedRepository = QLatin1String("");
  m_numberOfInstalledPackages = 0;
  m_debugInfo = false;
  m_console = nullptr;
  m_optionsDialog = nullptr;
  m_commandExecuting=ectn_NONE;
  m_commandQueued=ectn_NONE;

  m_sharedMemory = new QSharedMemory(QStringLiteral("org.arnt.octopi"), this);

  m_tcpServer = new QTcpServer(this);
  connect(m_tcpServer, &QTcpServer::newConnection, this, &MainWindow::onSendInfoToOctopiHelper);

  //Let's check if DistroRSSUrl is empty
  if (SettingsManager::isDistroRSSUrlEmpty())
  {
    SettingsManager::setDistroRSSUrl(SettingsManager::getDistroRSSUrl(UnixCommand::getLinuxDistro()));
  }

  m_time = new QElapsedTimer();
  m_unrequiredPackageList = nullptr;
  m_foreignPackageList = nullptr;
  m_groupWidgetNeedsFocus = false;
  m_outdatedAURTimer = new QTimer();
  m_outdatedAURTimer->setInterval(50);
  m_reconectSlotInvalidateTabs = false;
  connect(m_outdatedAURTimer, SIGNAL(timeout()), this, SLOT(postBuildPackageList()));

  //Here we try to speed up first pkg list build!
  m_time->start();
  ui->setupUi(this);
  switchToViewAllPackages();  

  m_pacmanDatabaseSystemWatcher =
            new QFileSystemWatcher(QStringList() << ctn_PACMAN_DATABASE_DIR, this);

  connect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(onPacmanDatabaseChanged()));

  setAcceptDrops(true);
  m_aurVote=nullptr;
}

/*
 * MainWindow's destructor
 */
MainWindow::~MainWindow()
{
  if (isAURGroupSelected())
  {
    if (m_savedSearchMethod->objectName() == QStringLiteral("actionSearchByName"))
      SettingsManager::setSearchMethod(SearchMethod::ectn_SEARCH_BY_NAME);
    else if (m_savedSearchMethod->objectName() == QStringLiteral("actionSearchByDescription"))
      SettingsManager::setSearchMethod(SearchMethod::ectn_SEARCH_BY_DESCRIPTION);
  }
  else
  {
    if (ui->actionSearchByName->isChecked())
      SettingsManager::setSearchMethod(SearchMethod::ectn_SEARCH_BY_NAME);
    else if (ui->actionSearchByDescription->isChecked())
      SettingsManager::setSearchMethod(SearchMethod::ectn_SEARCH_BY_DESCRIPTION);
  }

  savePackageColumnWidths();
  UnixCommand::removeTemporaryFiles();

  //Let's garbage collect transaction files...
  if (SettingsManager::getEnableAURVoting()) delete m_aurVote;
  delete ui;
}

/*
 * Checks if Notifier is running and busy
 */
bool MainWindow::isNotifierBusy()
{
  bool res=false;

  //Checks first if octopi is running...
  if (!UnixCommand::isOctoToolRunning(QStringLiteral("octopi-notifier"))) return res;

  QTcpSocket socket;
  socket.connectToHost(QStringLiteral("127.0.0.1"), 12702);

  if (!socket.waitForConnected(5000))
  {
    res=false;
  }

  QDataStream in(&socket);
  in.setVersion(QDataStream::Qt_5_15);
  QString octopiResponse;

  do
  {
    if (!socket.waitForReadyRead())
    {
      res=false;
    }

    in.startTransaction();
    in >> octopiResponse;
  } while (!in.commitTransaction());

  res = octopiResponse == QLatin1String("Octopi est occupatus");

  return res;
}

/*
 * Start listening for helper connections
 */
bool MainWindow::startServer()
{
  bool res=true;

  if (!m_tcpServer->listen(QHostAddress::LocalHost, 12701))
  {
    QMessageBox::critical(this, StrConstants::getApplicationName(),
                          QStringLiteral("Unable to start the server: %1.")
                          .arg(m_tcpServer->errorString()));
    res=false;
  }

  return res;
}

/*
 * The show() public SLOT, when the app is being drawn!!!
 * Init member variables and all UI widgets
 */
void MainWindow::show()
{
  if(m_debugInfo)
    std::cout << m_packageModel->getPackageCount() << " pkgs => " <<
               "Time elapsed at begining of show(): " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;

  if(!m_initializationCompleted)
  {
    restoreGeometry(SettingsManager::getWindowSize());
    m_leFilterPackage = new SearchLineEdit(this, m_hasSLocate);

    setWindowTitle(StrConstants::getApplicationName());
    setMinimumSize(QSize(820, 520));

    initTabOutput();
    initTabInfo();
    initTabFiles();
    initTabActions();
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
    onTerminalChanged();
    initTabWidgetPropertiesIndex();
    refreshDistroNews(false);

    if (Package::hasPacmanDatabase())
    {
      if(m_debugInfo)
        std::cout << m_packageModel->getPackageCount() << " pkgs => " <<
                   "Time elapsed before refreshGroupsWidget(): " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;

      refreshGroupsWidget();
    }        

    ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_POPULARITY_COLUMN, true);
    QMainWindow::show();

    m_listOfVisitedPackages.clear();
    m_indOfVisitedPackage = -1;

    if (Package::hasPacmanDatabase())
    {
      m_cic = new CPUIntensiveComputing;
      retrieveOutdatedPackageList();
      retrieveUnrequiredPackageList();
      retrieveForeignPackageList();

      metaBuildPackageList();
    }
    // Maybe this system has never synchronized pacman database!
    else
    {
      QMessageBox::critical(this, StrConstants::getError(), StrConstants::getMissingPacmanDBFile());
    }
  }
  else
    QMainWindow::show();
}

/*
 * Answers Helper if Octopi is executing actions
 */
void MainWindow::onSendInfoToOctopiHelper()
{
  QString msg;
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_15);

  //Is octopi-helper running?
  bool isHelperExecuting=UnixCommand::isOctopiHelperRunning();

  if (isHelperExecuting && m_commandExecuting != ectn_NONE)
  {
    msg=QStringLiteral("Octopi est occupatus");
    out << msg;
  }
  else if (isHelperExecuting && m_commandExecuting == ectn_NONE)
  {
    msg=QStringLiteral("Octopi serenum est");
    out << msg;
  }
  else if (m_checkupdatesStringList->count() > 0 || m_outdatedStringList->count() > 0)
  {
    msg=QStringLiteral("Renovatio potest");
    out << msg;
  }
  else
  {
    msg=QStringLiteral("Atramento nigro");
    out << msg;
  }

  QTcpSocket *clientConnection = m_tcpServer->nextPendingConnection();
  if (clientConnection->isOpen())
  {
    m_sharedMemory->detach();
    connect(clientConnection, &QAbstractSocket::disconnected,
          clientConnection, &QObject::deleteLater);
    clientConnection->write(block);
    clientConnection->disconnectFromHost();
  }
}

/*
 * Whenever a user drops a package from any outside source, we try to install it
 */
void MainWindow::dropEvent(QDropEvent *ev)
{
  m_packagesToInstallList.clear();

  QList<QUrl> urls = ev->mimeData()->urls();

  for(const QUrl& url: urls)
  {
    QString str = url.fileName();
    QFileInfo f(str);
    if (f.completeSuffix().contains(QLatin1String("pkg.tar")))
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

  for(const QUrl& url: urls)
  {
    QString str = url.fileName();
    QFileInfo f(str);
    if (f.completeSuffix().contains(QLatin1String("pkg.tar")))
    {
      success=true;
      break;
    }
  }

  if (success) ev->accept();
  else ev->ignore();
}

/*
 * When user drags local pacman pkgs to Octopi's main window
 */
void MainWindow::droppedLocalPackages(const QStringList &packagesToInstallList)
{
  if (!isExecutingCommand())
  {
    setPackagesToInstallList(packagesToInstallList);
    doInstallLocalPackages();
  }
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
    if (m_commandExecuting == ectn_NONE && m_initializationCompleted) m_actionSwitchToForeignTool->setEnabled(true);
    m_actionLastSearchMethod->setChecked(true);

    if (m_savedSearchMethod != nullptr)
      tvPackagesSearchColumnChanged(m_savedSearchMethod);
    else
      tvPackagesSearchColumnChanged(ui->actionSearchByName);
  }
  else m_actionSwitchToForeignTool->setEnabled(false);
}

/*
 * Whenever we call Options menu
 */
void MainWindow::onOptions(OptionsDialogTab tabToOpen)
{
  if (m_optionsDialog != nullptr || m_commandExecuting != ectn_NONE) return;

  m_optionsDialog = new OptionsDialog(this);

  if (m_debugInfo) m_optionsDialog->turnDebugInfoOn();

  connect(m_optionsDialog, SIGNAL(AURToolChanged()), this, SLOT(onAURToolChanged()));
  connect(m_optionsDialog, SIGNAL(AURVotingChanged()), this, SLOT(onAURVotingChanged()));

  connect(m_optionsDialog, &OptionsDialog::alternateRowColorsChanged, [this] (){
    this->ui->tvPackages->setAlternatingRowColors(SettingsManager::getUseAlternateRowColor());
  });

  connect(m_optionsDialog, &OptionsDialog::columnsChanged, [this] (){
    ui->tvPackages->resizePackageView();
  });

  connect(m_optionsDialog, SIGNAL(terminalChanged()), this, SLOT(onTerminalChanged()));

  if (tabToOpen==ectn_TAB_AUR) m_optionsDialog->gotoAURTab();

  m_optionsDialog->exec();
  Options::result res = static_cast<unsigned>(m_optionsDialog->result());

  delete m_optionsDialog;
  m_optionsDialog=nullptr;

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
  QTextBrowser *ret=nullptr;
  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));

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
  if (link.toString().contains(QLatin1String("goto:")))
  {
    QString pkgName = link.toString().mid(5).remove(QStringLiteral("[installed]"), Qt::CaseInsensitive).trimmed();
    //Let's remove any "<" and "<=" symbol...
    pkgName.remove(QRegularExpression(QStringLiteral("%3C\\S*")));
    pkgName.remove(QRegularExpression(QStringLiteral("\\=\\S*")));

    if (pkgName == QLatin1String("sh")) pkgName = QStringLiteral("bash");
    QFuture<QString> f;
    disconnect(&g_fwToolTipInfo, SIGNAL(finished()), this, SLOT(execToolTip()));
    const PackageRepository::PackageData*const package = getFirstPackageFromRepo(pkgName);
    if (!package) return;

    PkgDesc pkgDesc;
    pkgDesc.name = pkgName;

    pkgDesc.description = package->description;
    pkgDesc.isForeign = (package->status == ectn_FOREIGN || package->status == ectn_FOREIGN_OUTDATED);

    if (SettingsManager::getEnablePackageTooltips())
    {
      f = QtConcurrent::run(showPackageDescriptionExt, pkgDesc);
      g_fwToolTipInfo.setFuture(f);
      connect(&g_fwToolTipInfo, SIGNAL(finished()), this, SLOT(execToolTip()));
    }
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
      ui->tvPackages->setFocus();
    }
  }
  if (foundItems.count() == 0 || !proxyIndex.isValid())
  {
    refreshTabInfo(pkgName);
    disconnect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
    ensureTabVisible(ctn_TABINDEX_INFORMATION);
    connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
    ui->tvPackages->setFocus();
  }
}

/*
 * This SLOT is called whenever user clicks a url inside output's textBrowser
 */
void MainWindow::outputTextBrowserAnchorClicked(const QUrl &link)
{
  if (link.toString().contains(QLatin1String("goto:")))
  {
    QString pkgName = link.toString().mid(5).remove(QStringLiteral("[installed]"), Qt::CaseInsensitive).trimmed();
    if (pkgName == QLatin1String("sh")) pkgName = QStringLiteral("bash");

#ifdef ALPM_BACKEND
    if (AlpmBackend::getPackageVersion(pkgName).isEmpty())
    {
      return;
    }
#endif

    bool indIncremented = false;
    const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();

    if (selectionModel->selectedRows().count() <= 0) return;

    QModelIndexList selectedRows = selectionModel->selectedRows();
    QModelIndex item = selectedRows.at(0);
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

    if (!indIncremented) m_indOfVisitedPackage++;

    if (!m_leFilterPackage->text().isEmpty()) m_leFilterPackage->clear();

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
  if (m_commandExecuting != ectn_NONE || isAURGroupSelected())
    return;

  m_numberOfOutdatedPackages = m_outdatedStringList->count();

  if(m_numberOfOutdatedPackages > 0 && m_checkupdatesStringList->count() == 0)
  {
    QString html = QStringLiteral("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
    QString anchorBegin = QStringLiteral("anchorBegin");
    html += QLatin1String("<a id=\"") + anchorBegin + QLatin1String("\"></a>");

    clearTabOutput();

    if(m_outdatedStringList->count()==1){
      html += QLatin1String("<h3>") + StrConstants::getOneOutdatedPackage() + QLatin1String("</h3>");
    }
    else
    {
      html += QLatin1String("<h3>") +
          StrConstants::getOutdatedPackages(m_outdatedStringList->count()) + QLatin1String("</h3>");
    }

    if (SettingsManager::getMakeInterfaceLessCondensed())
      html += QLatin1String("<br><table cellpadding=\"4\" border=\"0\">");
    else
      html += QLatin1String("<br><table border=\"0\">");

    html += QLatin1String("<tr><th width=\"25%\" align=\"left\">") + StrConstants::getName() +
        QLatin1String("</th><th width=\"18%\" align=\"right\">") +
        StrConstants::getOutdatedVersion() +
        QLatin1String("</th><th width=\"18%\" align=\"right\">") +
        StrConstants::getAvailableVersion() + QLatin1String("</th></tr>");

    for (int c=0; c < m_outdatedStringList->count(); c++)
    {
      QString pkg = m_outdatedStringList->at(c);
      const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkg);

      if (package != nullptr) {
        html += QLatin1String("<tr><td><a href=\"goto:") + pkg + QLatin1String("\">") + pkg +
            QLatin1String("</td><td align=\"right\"><b><font color=\"#E55451\">") +
            package->outdatedVersion +
            QLatin1String("</b></font></td><td align=\"right\">") +
            package->version + QLatin1String("</td></tr>");
      }
    }

    html += QLatin1String("</table><br>");
    writeToTabOutput(html);

    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));

    if (text)
    {
      text->scrollToAnchor(anchorBegin);
    }

    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  }
  else {
    QString html = QStringLiteral("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
    QString anchorBegin = QStringLiteral("anchorBegin");
    html += QLatin1String("<a id=\"") + anchorBegin + QLatin1String("\"></a>");

    clearTabOutput();

    if(m_outdatedStringList->count()==1){
      html += QLatin1String("<h3>") + StrConstants::getOneOutdatedPackage() + QLatin1String("</h3>");
    }
    else
    {
      html += QLatin1String("<h3>") +
          StrConstants::getOutdatedPackages(m_outdatedStringList->count()) + QLatin1String("</h3>");
    }

    if (SettingsManager::getMakeInterfaceLessCondensed())
      html += QLatin1String("<br><table cellpadding=\"4\" border=\"0\">");
    else
      html += QLatin1String("<br><table border=\"0\">");

    html += QLatin1String("<tr><th width=\"25%\" align=\"left\">") + StrConstants::getName() +
        QLatin1String("</th><th width=\"18%\" align=\"right\">") +
        StrConstants::getOutdatedVersion() +
        QLatin1String("</th><th width=\"18%\" align=\"right\">") +
        StrConstants::getAvailableVersion() + QLatin1String("</th></tr>");

    for (int c=0; c < m_checkupdatesStringList->count(); c++)
    {
      QString pkg = m_checkupdatesStringList->at(c);
      const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkg);

      if (package != nullptr) {
        html += QLatin1String("<tr><td><a href=\"goto:") + pkg + QLatin1String("\">") + pkg +
            QLatin1String("</td><td align=\"right\"><b><font color=\"#E55451\">") +
            package->outdatedVersion +
            QLatin1String("</b></font></td><td align=\"right\">") +
            package->version + QLatin1String("</td></tr>");
      }
    }

    html += QLatin1String("</table><br>");
    writeToTabOutput(html);

    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));

    if (text)
    {
      text->scrollToAnchor(anchorBegin);
    }

    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  }
}

/*
 * Prints the list of voted AUR packages to the Output tab.
 */
void MainWindow::outputAURVotedPackageList()
{
  if (m_aurVote!=nullptr)
  {
    clearTabOutput();
    QString html = QStringLiteral("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
    QString anchorBegin = QStringLiteral("anchorBegin");
    html += QLatin1String("<a id=\"") + anchorBegin + QLatin1String("\"></a>");
    html += QLatin1String("<h3>") + StrConstants::getAURVotedPackageList() + QLatin1String("</h3>");
    writeToTabOutput(html);
    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);

    QStringList v=m_aurVote->getVotedPackages();
    v.sort();
    QString list=QStringLiteral("<ul>");
    for(const QString& vote: v)
    {
      list += QLatin1String("<li>") + vote + QLatin1String("</li>");
    }

    list += QLatin1String("</ul><br>");

    writeToTabOutput(list);

    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));

    if (text)
    {
      text->scrollToAnchor(anchorBegin);
    }
  }
}

/*
 * Prints the list of outdated AUR packages to the Output tab.
 */
void MainWindow::outputOutdatedAURPackageList()
{
  //We cannot output any list if there is a running transaction!
  if (m_commandExecuting != ectn_NONE) return;

  QString html = QStringLiteral("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
  QString anchorBegin = QStringLiteral("anchorBegin");
  html += QLatin1String("<a id=\"") + anchorBegin + QLatin1String("\"></a>");

  clearTabOutput();

  if(m_outdatedAURStringList->count()==1){
    html += QLatin1String("<h3>") + StrConstants::getOneOutdatedPackage() + QLatin1String("</h3>");
  }
  else
  {
    html += QLatin1String("<h3>") +
        StrConstants::getOutdatedPackages(m_outdatedAURStringList->count()) + QLatin1String("</h3>");
  }

  if (SettingsManager::getMakeInterfaceLessCondensed())
    html += QLatin1String("<br><table cellpadding=\"4\" border=\"0\">");
  else
    html += QLatin1String("<br><table border=\"0\">");

  html += QLatin1String("<tr><th width=\"25%\" align=\"left\">") + StrConstants::getName() +
      QLatin1String("</th><th width=\"18%\" align=\"right\">") +
      StrConstants::getOutdatedVersion() +
      QLatin1String("</th><th width=\"18%\" align=\"right\">") +
      StrConstants::getAvailableVersion() + QLatin1String("</th></tr>");

  for (int c=0; c < m_outdatedAURStringList->count(); c++)
  {
    QString pkg = m_outdatedAURStringList->at(c);
    const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkg);
    if (package != nullptr) {
      QString availableVersion = m_outdatedAURPackagesNameVersion->value(m_outdatedAURStringList->at(c));
  
      html += QLatin1String("<tr><td><a href=\"goto:") + pkg + QLatin1String("\">") + pkg +
          QLatin1String("</td><td align=\"right\"><b><font color=\"#E55451\">") +
          package->version +
          QLatin1String("</b></font></td><td align=\"right\">") +
          availableVersion + QLatin1String("</td></tr>");
    }
  }

  html += QLatin1String("</table><br>");
  writeToTabOutput(html);

  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));

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
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));
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
  return ((group == QLatin1String("<") + StrConstants::getDisplayAllGroups() + QLatin1String(">")) && !(m_actionSwitchToForeignTool->isChecked()));
}

/*
 * Helper to analyse if < AUR > is selected
 */
bool MainWindow::isAURGroupSelected()
{
  return (m_actionSwitchToForeignTool->isChecked());
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
const PackageRepository::PackageData* MainWindow::getFirstPackageFromRepo(const QString &pkgName)
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
  m_removeCommand.remove(QStringLiteral("="));
  m_removeCommand.remove(QStringLiteral("-"));
}

/*
 * Searchs model modelInstalledPackages by a package name and returns if it is already installed
 */
bool MainWindow::isPackageInstalled(const QString &pkgName)
{
  const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkgName);
  return (package != nullptr && package->installed());
}

/*
 * User changed the search column used by the SortFilterProxyModel in tvPackages
 */
void MainWindow::tvPackagesSearchColumnChanged(QAction *actionSelected)
{
  if (!isAURGroupSelected())
  {
    m_actionLastSearchMethod = actionSelected;
    m_savedSearchMethod = m_actionLastSearchMethod;
  }

  //We are in the realm of tradictional NAME search
  if (actionSelected->objectName() == ui->actionSearchByName->objectName())
  {
    if (ui->actionUseInstantSearch->isChecked())
    {
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    }

    ui->menuView->setEnabled(true);
    if (!m_actionSwitchToForeignTool->isChecked()) ui->twGroups->setEnabled(true);

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
    if (!m_actionSwitchToForeignTool->isChecked()) ui->twGroups->setEnabled(true);

    if (isAURGroupSelected())
      m_leFilterPackage->setRefreshValidator(ectn_AUR_VALIDATOR);
    else
      m_leFilterPackage->setRefreshValidator(ectn_DEFAULT_VALIDATOR);

    m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN);
  }
  else if (actionSelected->objectName() == ui->actionSearchByFile->objectName())
  {
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));

    //m_leFilterPackage->clear();
    //m_packageModel->applyFilter("");
    ui->actionViewAllPackages->trigger();
    m_actionRepositoryAll->trigger();
    ui->menuView->setEnabled(false);

    ui->twGroups->setEnabled(false);
    m_leFilterPackage->setRefreshValidator(ectn_FILE_VALIDATOR);
  }

  /*if (!isSearchByFileSelected() && m_packageModel->getPackageCount() <= 1)
  {
    m_leFilterPackage->clear();
  }*/

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
  if (m_actionSwitchToForeignTool->isChecked())
    m_packageModel->applyFilter(viewOptions, QLatin1String(""), StrConstants::getForeignToolGroup());
  else
    m_packageModel->applyFilter(viewOptions, selectedRepo, isAllGroupsSelected() ? QLatin1String("") : getSelectedGroup());

  if (m_leFilterPackage->text() != QLatin1String("")) reapplyPackageFilter();

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
  if (selectionModel != nullptr && selectionModel->selectedRows().count() > 0)
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
          //if (package->repository == StrConstants::getForeignRepositoryName())
          {           
            insertIntoRemovePackage();
          }
        }
        else if (command == ectn_INSTALL)
        {
          if (UnixCommand::getLinuxDistro() != ectn_KAOS && package->repository != StrConstants::getForeignRepositoryName())
          {
            insertIntoInstallPackage();
          }
          else if (UnixCommand::getLinuxDistro() == ectn_KAOS && !hasInstallActions())
          {
            insertIntoInstallPackage();
          }
        }
      }
    }
    else //There are more than 1 package selected
    {
      int countToInstall = 0;
      int countToRemove = 0;

      for(QModelIndex item: selectedRows)
      {
        const PackageRepository::PackageData*const package = m_packageModel->getData(item);

        if (command == ectn_INSTALL && package->repository != StrConstants::getForeignRepositoryName())
        {
          countToInstall++;
          //insertIntoInstallPackage();
        }

        else if (package->installed() && command == ectn_REMOVE && !package->required)
        {
          countToRemove++;
          //insertIntoRemovePackage();
        }

        else if (package->repository == StrConstants::getForeignRepositoryName())
        {
          if (!isAURGroupSelected() && package->installed() && command == ectn_REMOVE)
          {
            countToRemove++; //insertIntoRemovePackage();
          }
          else if (isAURGroupSelected())
          {
            if (command == ectn_INSTALL)
              countToInstall++; //insertIntoInstallPackage();
            else if (package->installed() && command == ectn_REMOVE)
              countToRemove++; //insertIntoRemovePackage();
          }
        }
      }

      if (countToRemove == selectedRows.count())
        insertIntoRemovePackage();
      else if (countToInstall == selectedRows.count())
        insertIntoInstallPackage();
    } // End of 'more than 1 package selected'
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
  if (selectionModel != nullptr && selectionModel->selectedRows().count() > 0)
  {
    QMenu* menu = new QMenu(this);
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.count() == 1) // enable entry "browse installed files" ?
    {
      QModelIndex item = selectedRows.at(0);
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      if (package)
      {
        menu->addAction(m_actionPackageInfo);
      }
      if (package){
        menu->addAction(ui->actionFindFileInPackage);
      }
      if (!menu->actions().isEmpty()) menu->addSeparator();

      if (package->status == ectn_IGNORED)
      {
        menu->addSeparator();
        menu->addAction(ui->actionRemoveFromIgnored);
        menu->addAction(m_actionChangeInstallReason);
      }
    }

    bool allInstallable = true;
    bool allRemovable = true;
    int numberOfSelPkgs = selectedRows.count();
    int numberOfAUR = 0;
    int numberOfIgnored = 0;
    int numberOfOutdated = 0;
    int numberOfIntalled = 0;

    for(QModelIndex item: selectedRows)
    {
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);

      if (package->repository == StrConstants::getForeignRepositoryName())
      {
        allInstallable = false;
        numberOfAUR++;
      }
      if (package->outdated()) numberOfOutdated ++;

      if (!package->installed())
      {
        allRemovable = false;
      }

      if (package->status == ectn_IGNORED)
      {
        numberOfIgnored++;
        allInstallable = false;
        allRemovable = false;
      }
    }

    if (allInstallable) // implicitly foreign packages == 0
    {
      if (!isAllGroupsSelected() && !isAURGroupSelected()) menu->addAction(ui->actionInstallGroup);
      if (!allRemovable && numberOfOutdated != numberOfSelPkgs)
      {
        ui->actionInstall->setText(StrConstants::getInstall());
      }
      else if (allRemovable && numberOfOutdated == numberOfSelPkgs)
      {
        menu->addAction(ui->actionAddToIgnored);
        ui->actionInstall->setText(StrConstants::getUpdate());
      }
      else if (allRemovable)
      {
        menu->addAction(ui->actionAddToIgnored);
        ui->actionInstall->setText(StrConstants::getReinstall());
      }

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
    else if (!allInstallable && numberOfIgnored == numberOfSelPkgs)
    {
      //There are only ignored packages here
      menu->addAction(ui->actionRemoveFromIgnored);
      menu->addAction(m_actionChangeInstallReason);
    }
    else if (!allInstallable && numberOfAUR == numberOfSelPkgs)
    {
      QString aurPkg;
      bool allInstalled=false;
      int numOutdated=0;
      for(QModelIndex item: selectedRows)
      {
        const PackageRepository::PackageData*const package = m_packageModel->getData(item);
        aurPkg = package->name;

        if (package->installed() || package->outdated())
        {
          numberOfIntalled++;
          if (package->outdated()) ++numOutdated;
          allInstalled = true;
        }
        else
        {
          allInstalled = false;
        }
      }

      if (allInstalled && (numOutdated==numberOfSelPkgs))
      {
        menu->addAction(ui->actionAddToIgnored);
        ui->actionInstall->setText(StrConstants::getUpdate());
      }
      else if (numberOfIntalled==numberOfSelPkgs)
      {
        menu->addAction(ui->actionAddToIgnored);
        ui->actionInstall->setText(StrConstants::getReinstall());
      }
      else
      {
        ui->actionInstall->setText(StrConstants::getInstall());
      }

      if (selectedRows.count() == 1 && m_aurVote != nullptr)
      {
        if (m_aurVote->isPkgVoted(aurPkg) == 0)
          menu->addAction(m_actionAURUnvote);
        else if (m_aurVote->isPkgVoted(aurPkg) == 1)
          menu->addAction(m_actionAURVote);
      }

      if (selectedRows.count() >= 1 && Package::isAURBased())
      {
        if (numOutdated == numberOfSelPkgs)
        {
          menu->addAction(m_actionUpdateAURPackage);
        }

        if (selectedRows.count() == 1)
        {
          menu->addAction(m_actionAUROpenPKGBUILD);
          menu->addAction(m_actionAURShowPKGBUILDDiff);
        }
      }

      if (isAURGroupSelected())
      {
        if (UnixCommand::getLinuxDistro() != ectn_KAOS)
          menu->addAction(ui->actionInstall); //AUR);
        else
        {
          if (!hasInstallActions()) menu->addAction(ui->actionInstall);
        }
      }
    }

    if (allRemovable)
    {
      menu->addAction(ui->actionRemove);
      if (!isAURGroupSelected()) menu->addAction(m_actionChangeInstallReason);

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
 * Given the selected AUR package we vote on it
 */
void MainWindow::onAURVote()
{
  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (selectionModel != nullptr && selectionModel->selectedRows().count() > 0)
  {
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.count() == 1)
    {
      QModelIndex item = selectedRows.at(0);
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      m_aurVote->voteForPkg(package->name);
      if (isAURGroupSelected()) metaBuildPackageList();
    }
  }
}

/*
 * Given the selected AUR package we remove its vote
 */
void MainWindow::onAURUnvote()
{
  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (selectionModel != nullptr && selectionModel->selectedRows().count() > 0)
  {
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.count() == 1)
    {
      QModelIndex item = selectedRows.at(0);
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      m_aurVote->unvoteForPkg(package->name);
      if (isAURGroupSelected()) metaBuildPackageList();
    }
  }
}

/*
 * Given the package name, retrieve "Package Base" value from AUR site
 */
QString MainWindow::getAURPackageBase(const QString& packageName)
{
  QString res;
  QString pkgAUR(QStringLiteral("https://aur.archlinux.org/packages/%1/"));
  QNetworkAccessManager manager;
  manager.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
  QNetworkReply *response = manager.get(QNetworkRequest(QUrl(pkgAUR.arg(packageName))));
  QEventLoop event;
  connect(response, SIGNAL(finished()), &event, SLOT(quit()));
  event.exec();

  QString contents = QString::fromUtf8(response->readAll());
  //QRegularExpression re(QStringLiteral("a href=\"/pkgbase/(?<packageBase>\\S+)/\""));
  QRegularExpression re(QStringLiteral("a href=\"/pkgbase/(?<packageBase>[\\w\\-_]+)\\\">"));
  QRegularExpressionMatch rem;
  if (contents.contains(re, &rem))
  {
    res=rem.captured(QStringLiteral("packageBase"));
  }

  return res;
}

/*
 * Given the selected AUR package we download its PKGBUILD from
 * https://aur.archlinux.org/cgit/aur.git/tree/PKGBUILD?h=AUR-PKG-NAME
 */
void MainWindow::onAUROpenPKGBUILD()
{
  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (selectionModel != nullptr && selectionModel->selectedRows().count() > 0)
  {
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.count() == 1)
    {
      CPUIntensiveComputing cic;
      QModelIndex item = selectedRows.at(0);
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      QString pkgbuildSite(QStringLiteral("https://aur.archlinux.org/cgit/aur.git/plain/PKGBUILD?h=%1"));
      QString tempFileName(QStringLiteral(".temp_octopi_") + package->name + QStringLiteral("_PKGBUILD"));

      QProcess curl;
      QStringList params;

      QString pkgBase = getAURPackageBase(package->name);
      if (pkgBase.isEmpty()) return;

      params << pkgbuildSite.arg(pkgBase);
      params << QStringLiteral("-o");
      params << QDir::tempPath() + QDir::separator() + tempFileName;
      curl.start(QStringLiteral("/usr/bin/curl"), params);
      curl.waitForFinished(-1);

      if (curl.exitCode() == 0)
        WMHelper::editFile(QDir::tempPath() + QDir::separator() + tempFileName, ectn_EDIT_AS_NORMAL_USER);
    }
  }
}

/*
 * Given the selected AUR package we download its PKGBUILD diff and show in a text editor
 */
void MainWindow::onAURShowPKGBUILDDiff()
{
  //https://aur.archlinux.org/cgit/aur.git/log/PKGBUILD?h=AUR_PKG_NAME
  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (selectionModel != nullptr && selectionModel->selectedRows().count() > 0)
  {
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.count() == 1)
    {
      CPUIntensiveComputing *cic=new CPUIntensiveComputing();
      QModelIndex item = selectedRows.at(0);
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      QString pkglogSite(QStringLiteral("https://aur.archlinux.org/cgit/aur.git/log/PKGBUILD?h=%1"));
      //QString latestVersion, previousVersion;

      //Let's download LOG html page and find the two newest version commit hashes

      QString pkgBase = getAURPackageBase(package->name);
      if (pkgBase.isEmpty())
      {
        delete cic;
        return;
      }

      QNetworkAccessManager manager;
      QNetworkReply *response = manager.get(QNetworkRequest(QUrl(pkglogSite.arg(pkgBase))));
      QEventLoop event;
      connect(response, SIGNAL(finished()), &event, SLOT(quit()));
      event.exec();

      QString res = QString::fromUtf8(response->readAll());
      QRegularExpression re(QStringLiteral(";id=(?<commit>[a-f0-9]+)"));
      QRegularExpressionMatchIterator i = re.globalMatch(res);
      QStringList commits;

      while (i.hasNext())
      {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch())
        {
          commits << match.captured(QStringLiteral("commit"));
          if (commits.count() == 2) break;
        }
      }

      if (commits.count() != 2)
      {
        delete cic;
        QMessageBox::information(this, StrConstants::getApplicationName(),
                                 StrConstants::getThereIsOnlyOneVersionAvailable().arg(package->name));
        return;
      }

      QString pkgCommit(QStringLiteral("https://aur.archlinux.org/cgit/aur.git/plain/PKGBUILD?h=%1"));
      QString tempPkgLatest(QStringLiteral(".temp_octopi_commit_latest"));
      QString tempPkgPrevious(QStringLiteral(".temp_octopi_commit_previous"));

      QProcess p;
      QStringList params;
      params << pkgCommit.arg(commits.at(0));
      params << QStringLiteral("-o");
      params << QDir::tempPath() + QDir::separator() + tempPkgLatest;
      p.start(QStringLiteral("/usr/bin/curl"), params);
      p.waitForFinished(-1);

      params.clear();
      params << pkgCommit.arg(commits.at(1));
      params << QStringLiteral("-o");
      params << QDir::tempPath() + QDir::separator() + tempPkgPrevious;
      p.start(QStringLiteral("/usr/bin/curl"), params);
      p.waitForFinished(-1);

      params.clear();
      QString tempPkgDiff(QStringLiteral(".temp_octopi_commit_diff"));
      p.setStandardOutputFile(QDir::tempPath() + QDir::separator() + tempPkgDiff);
      params << QStringLiteral("-u");
      params << QDir::tempPath() + QDir::separator() + tempPkgPrevious;
      params << QDir::tempPath() + QDir::separator() + tempPkgLatest;
      p.start(QStringLiteral("/usr/bin/diff"), params);
      p.waitForFinished(-1);

      delete cic;

      //Diff returns a "1" exit code if files are different!
      if (p.exitCode() == 1)
        WMHelper::editFile(QDir::tempPath() + QDir::separator() + tempPkgDiff, ectn_EDIT_AS_NORMAL_USER);
    }
  }
}

/*
 * This SLOT collapses all treeview items
 */
void MainWindow::collapseAllContentItems(){
  QTreeView *tv = ui->twProperties->getTvPkgFileList();
  if (tv != nullptr)
    tv->collapseAll();
}

/*
 * This SLOT collapses only the currently selected item
 */
void MainWindow::collapseThisContentItems(){
  QTreeView *tv = ui->twProperties->getTvPkgFileList();
  if (tv != nullptr)
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
  QTreeView *tv = ui->twProperties->getTvPkgFileList();
  if (tv != nullptr)
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
  QTreeView *tv = ui->twProperties->getTvPkgFileList();
  if (tv != nullptr)
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
      tv->collapse(mi);
      QModelIndex mi2 = sim->index(i, 0, mi);
      collapseItem(tv, sim, mi2);
    }
  }
}

/*
 * This method does the job of expanding the given item and its children
 */
void MainWindow::expandItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex* mi){
  for (int i=0; i<sim->rowCount(*mi); i++){
    if (sim->hasChildren(*mi))
    {
      tv->expand(*mi);
      QModelIndex mi2 = sim->index(i, 0, *mi);
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
      ui->twProperties->getTvPkgFileList();

  if (tvPkgFileList == nullptr)
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

    if (si == nullptr) return;
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
    if (f.exists() && UnixCommand::isTextFile(selectedPath))
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
  QTreeView *tvTransaction = ui->twProperties->getTvTransaction();
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
  QTreeView *tvPkgFileList = ui->twProperties->getTvPkgFileList();
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
  SearchBar *searchBar = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<SearchBar*>(QStringLiteral("searchbar"));
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

  if (fileName.endsWith(QLatin1Char('/')))
  {
    baseFileName.remove(baseFileName.size()-1, 1);
  }

  return baseFileName.right(baseFileName.size() - baseFileName.lastIndexOf(QLatin1Char('/')) -1);
}

/*
 * Whenever user double clicks the package list items, app shows the contents of the selected package
 */
void MainWindow::onDoubleClickPackageList()
{
  const PackageRepository::PackageData* package = m_packageModel->getData(ui->tvPackages->currentIndex());
  if (package != nullptr && package->installed())
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
  disconnect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(invalidateTabs()));

  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION)
    refreshTabInfo();
  else if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    refreshTabFiles();

  if(m_initializationCompleted)
    saveSettings(ectn_CURRENTTABINDEX);

  //connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
  //        this, SLOT(invalidateTabs()));
}

/*
 * When the user changes the current selected tab, we must take care of data refresh.
 */
void MainWindow::changedTabIndex()
{
  disconnect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(invalidateTabs()));

  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION)
    refreshTabInfo();
  else if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    refreshTabFiles();
  else if (ui->twProperties->currentIndex() == ctn_TABINDEX_TERMINAL)
  {
    m_console->setFocus();
  }

  if(m_initializationCompleted)
    saveSettings(ectn_CURRENTTABINDEX);
}

/*
 * Clears the contents of Info or Files tabs depending which one is selected
 */
void MainWindow::clearTabsInfoOrFiles()
{
  //disconnect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
  //        this, SLOT(invalidateTabs()));

  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION) //This is TabInfo
  {
    refreshTabInfo(false, false); //(true, false);
    return;
  }
  else if(ui->twProperties->currentIndex() == ctn_TABINDEX_FILES) //This is TabFiles
  {
    refreshTabFiles(); //(true, false);
    return;
  }

  //connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
  //        this, SLOT(invalidateTabs()));
}

/*
 * Clears the contents of Info or Files tabs depending which one is selected
 */
void MainWindow::clearTabsInfoOrFilesExt()
{
  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION) //This is TabInfo
  {
    refreshTabInfo(true, false);
    return;
  }
  else if(ui->twProperties->currentIndex() == ctn_TABINDEX_FILES) //This is TabFiles
  {
    refreshTabFiles(true);
    return;
  }
}

/*
 * Clears the information showed on the current tab (Info or Files).
 */
void MainWindow::invalidateTabs()
{
  disconnect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(invalidateTabs()));

  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION) //This is TabInfo
  {
    refreshTabInfo(false, false);
    return;
  }
  else if(ui->twProperties->currentIndex() == ctn_TABINDEX_FILES) //This is TabFiles
  {
    refreshTabFiles();
    return;
  }

  //connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
  //        this, SLOT(invalidateTabs()));
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
 * Maximizes the upper pane (tvPackages)
 */
void MainWindow::maximizePackagesTreeView()
{
  QList<int> l;
  ui->splitterHorizontal->setSizes( l << ui->tvPackages->maximumHeight() << 0);
}

/*
 * Maximizes/de-maximizes the upper pane (tvPackages)
 */
void MainWindow::maxDemaxPackagesTreeView(bool pSaveSettings)
{
  QList<int> savedSizes;
  savedSizes << 200 << 235;

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[1] != 0 ) //Maximizes
  {
    ui->splitterHorizontal->setSizes( l << ui->tvPackages->maximumHeight() << 0);
    if(!ui->tvPackages->hasFocus())
      ui->tvPackages->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_MAXIMIZE_PACKAGES);
  }
  else //Demaximizes
  {
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_NORMAL);
  }
}

/*
 * Maximizes/de-maximizes terminal tab
 */
void MainWindow::maximizeTerminalTab()
{
  maxDemaxPropertiesTabWidget(false);
}

/*
 * Maximizes the lower pane (tabwidget)
 */
void MainWindow::maximizePropertiesTabWidget()
{
  QList<int> l;
  ui->splitterHorizontal->setSizes( l << 0 << ui->twProperties->maximumHeight());
  ui->twProperties->currentWidget()->childAt(1,1)->setFocus();
  ui->twProperties->tabBar()->hide();
}

/*
 * Maximizes/de-maximizes the lower pane (tabwidget)
 */
void MainWindow::maxDemaxPropertiesTabWidget(bool pSaveSettings)
{
  QList<int> savedSizes;
  savedSizes << 200 << 235;

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[0] != 0 ) //Maximize here
  {
    ui->splitterHorizontal->setSizes( l << 0 << ui->twProperties->maximumHeight());

    if (ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION)
    {
      QTextBrowser *tb = ui->twProperties->currentWidget()->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));
      if (tb) tb->setFocus();
    }
    else
      ui->twProperties->currentWidget()->childAt(1,1)->setFocus();

    ui->twProperties->tabBar()->hide();

    if(pSaveSettings)
      saveSettings(ectn_MAXIMIZE_PROPERTIES);
  }
  else //DeMaximize here
  {
    ui->twProperties->tabBar()->show();
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();

    if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    {
      QTreeView *tv = ui->twProperties->getTvPkgFileList();
      if (tv)
        tv->scrollTo(tv->currentIndex());
    }
    else if(ui->twProperties->currentIndex() == ctn_TABINDEX_TERMINAL)
    {
      m_console->setFocus();
    }

    if(pSaveSettings)
    {
      bool hideGroups=false;
      if (!m_actionShowGroups->isChecked()) hideGroups=true;
      saveSettings(ectn_NORMAL);
      if (hideGroups) SettingsManager::setShowGroupsPanel(false);
    }
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
    saveSettings(ectn_AUR_PACKAGELIST);
  else
    saveSettings(ectn_PACKAGELIST);
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void MainWindow::positionTextEditCursorAtEnd()
{
  QTextBrowser *textEdit =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));
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
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));
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
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));
  if (text)
  {
    positionTextEditCursorAtEnd();
    //We have to find at least two times, as "Synching" string will always be the first text in output
    res = text->find(StrConstants::getSyncing(), QTextDocument::FindBackward | QTextDocument::FindWholeWords);
    res = text->find(StrConstants::getSyncing(), QTextDocument::FindBackward | QTextDocument::FindWholeWords);
    res = text->find(StrConstants::getCheckingForUpdates(), QTextDocument::FindBackward | QTextDocument::FindWholeWords);

    if (!res)
    {
      positionTextEditCursorAtEnd();
      res = text->find(QStringLiteral("downloading"), QTextDocument::FindBackward | QTextDocument::FindWholeWords);
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
  QTreeView *tv = ui->twProperties->getTvPkgFileList();
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
  QTreeView *tv = ui->twProperties->getTvPkgFileList();
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
    m_console->execute(QLatin1String("cd ") + dir);
    ensureTabVisible(ctn_TABINDEX_TERMINAL);
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
 * Open a file chooser dialog for the user to select local packages to install (pacman -U)
 */
void MainWindow::installLocalPackage()
{
  if (!isSUAvailable()) return;

  m_packagesToInstallList =
      QFileDialog::getOpenFileNames(this,
                                    StrConstants::getFileChooserTitle(),
                                    QDir::homePath(),
                                    StrConstants::getPackages() + QLatin1String(" (*.pkg.tar*)"));

  if (m_packagesToInstallList.count() > 0)
    doInstallLocalPackages();
}

/*
 * Brings the user to the Info tab
 */
void MainWindow::showPackageInfo()
{
  refreshTabInfo(false, true);
}

/*
 * Brings the user to the Files tab and position cursor inside searchBar
 * so he can find any file the selected package may have
 */
void MainWindow::findFileInPackage()
{
  refreshTabFiles(false, true);

  QTreeView *tb = ui->twProperties->getTvPkgFileList();
  SearchBar *searchBar = ui->twProperties->currentWidget()->findChild<SearchBar*>(QStringLiteral("searchbar"));

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
    QTreeView *t = ui->twProperties->getTvPkgFileList();
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
  const int selected = selection != nullptr ? selection->selectedRows().count() : 0;

  QString newMessage = StrConstants::getTotalPackages(m_packageModel->getPackageCount()) +
      QLatin1String(" (") + StrConstants::getSelectedPackages(selected) + QLatin1String(") ");

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
  QProcess *proc = new QProcess(this);
  proc->startDetached(QStringLiteral("plv"), QStringList());
}

/*
 * Launch Repo Editor
 */
void MainWindow::launchRepoEditor()
{
  m_unixCommand = new UnixCommand(this);
  m_unixCommand->execCommandAsNormalUser(QStringLiteral("octopi-repoeditor"), QStringList());
}

/*
 * Launch Cache Cleaner
 */
void MainWindow::launchCacheCleaner()
{
  m_unixCommand = new UnixCommand(this);
  m_unixCommand->execCommandAsNormalUser(QStringLiteral("octopi-cachecleaner"), QStringList());
}

/*
 * Creates "octopi-sysinfo.log" file in user's home path as a result of a bunch of commands.
 */
void MainWindow::doSysInfo()
{
  if (!UnixCommand::hasTheExecutable(QStringLiteral("curl")) ||
      m_commandExecuting != ectn_NONE) return;

  if (!isInternetAvailable()) return;

  //Asks user if he/she is sure about doing this
  int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                  StrConstants::getDoYouAgreeToGenerateSysInfo(),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);
  if (res == QMessageBox::No) return;

  disableTransactionActions();
  m_commandExecuting = ectn_SYSINFO;
  clearTabOutput();
  writeToTabOutput(QStringLiteral("<b>SysInfo...</b><br>"));

  QString command;
  QEventLoop el;
  QFuture<QByteArray> f;
  command=QStringLiteral("hostname");
  f = QtConcurrent::run(execCommand, command);
  connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwCommandToExecute.setFuture(f);
  el.exec();
  QString hostname = QString::fromUtf8(g_fwCommandToExecute.result());

  hostname.remove(QStringLiteral("\n"));
  QString homePath = QDir::homePath();
  QByteArray out;

  if (UnixCommand::getLinuxDistro() == ectn_KAOS)
  {
    out += "----------------------------------------------------------------------------------------------------------\n";
    out += "cat /etc/KaOS-release\n";
    out += "----------------------------------------------------------------------------------------------------------\n\n";

    command = QStringLiteral("cat /etc/KaOS-release");
    f = QtConcurrent::run(execCommand, command);
    connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
    g_fwCommandToExecute.setFuture(f);
    el.exec();
    out += g_fwCommandToExecute.result();
    out += "\n\n";
  }
  else
  {
    out += "----------------------------------------------------------------------------------------------------------\n";
    out += "cat /etc/lsb-release\n";
    out += "----------------------------------------------------------------------------------------------------------\n\n";

    command = QStringLiteral("cat /etc/lsb-release");
    f = QtConcurrent::run(execCommand, command);
    connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
    g_fwCommandToExecute.setFuture(f);
    el.exec();
    out += g_fwCommandToExecute.result();
    out += "\n\n";
  }

  if (UnixCommand::hasTheExecutable(QStringLiteral("inxi")))
  {
    out += "----------------------------------------------------------------------------------------------------------\n";
    out += "inxi -Fxz\n";
    out += "----------------------------------------------------------------------------------------------------------\n\n";

    command = QStringLiteral("inxi -Fxz -c 0");
    f = QtConcurrent::run(execCommand, command);
    connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
    g_fwCommandToExecute.setFuture(f);
    el.exec();
    out += g_fwCommandToExecute.result();
    out += "\n\n";
  }
  else
  {
    out += "----------------------------------------------------------------------------------------------------------\n";
    out += "uname -a\n";
    out += "----------------------------------------------------------------------------------------------------------\n\n";

    command = QStringLiteral("uname -a");
    f = QtConcurrent::run(execCommand, command);
    connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
    g_fwCommandToExecute.setFuture(f);
    el.exec();
    out += g_fwCommandToExecute.result();
    out += "\n\n";
  }

  if (UnixCommand::hasTheExecutable(QStringLiteral("mhwd")))
  {
    out += "----------------------------------------------------------------------------------------------------------\n";
    out += "mhwd -li -d\n";
    out += "----------------------------------------------------------------------------------------------------------\n\n";

    command = QStringLiteral("mhwd -li -d");
    f = QtConcurrent::run(execCommand, command);
    connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
    g_fwCommandToExecute.setFuture(f);
    el.exec();
    out += g_fwCommandToExecute.result();
    out += "\n\n";
  }

  out += "----------------------------------------------------------------------------------------------------------\n";
  out += "journalctl -b -p err\n";
  out += "----------------------------------------------------------------------------------------------------------\n\n";

  command = QStringLiteral("journalctl -b -p err");
  f = QtConcurrent::run(execCommand, command);
  connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwCommandToExecute.setFuture(f);
  el.exec();
  out += g_fwCommandToExecute.result();
  out += "\n\n";

  out += "----------------------------------------------------------------------------------------------------------\n";
  out += "cat /etc/pacman.conf\n";
  out += "----------------------------------------------------------------------------------------------------------\n\n";

  command = QStringLiteral("cat /etc/pacman.conf");
  f = QtConcurrent::run(execCommand, command);
  connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwCommandToExecute.setFuture(f);
  el.exec();
  out += g_fwCommandToExecute.result();
  out += "\n\n";

  out += "----------------------------------------------------------------------------------------------------------\n";
  out += "pacman -Qm\n";
  out += "----------------------------------------------------------------------------------------------------------\n\n";

  command = QStringLiteral("pacman -Qm");
  f = QtConcurrent::run(execCommand, command);
  connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwCommandToExecute.setFuture(f);
  el.exec();
  out += g_fwCommandToExecute.result();
  out += "\n\n";

  if (UnixCommand::getLinuxDistro() == ectn_KAOS)
  {
    out += "----------------------------------------------------------------------------------------------------------\n";
    out += "cat /var/log/pacman.log\n";
    out += "----------------------------------------------------------------------------------------------------------\n\n";

    command = QStringLiteral("cat /var/log/pacman.log");
    f = QtConcurrent::run(execCommand, command);
    connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
    g_fwCommandToExecute.setFuture(f);
    el.exec();
    out += g_fwCommandToExecute.result();
    out += "\n\n";

    out += "----------------------------------------------------------------------------------------------------------\n";
    out += "cat /var/log/installation.log\n";
    out += "----------------------------------------------------------------------------------------------------------\n\n";

    command = QStringLiteral("cat /var/log/installation.log");
    f = QtConcurrent::run(execCommand, command);
    connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
    g_fwCommandToExecute.setFuture(f);
    el.exec();
    out += g_fwCommandToExecute.result();

    QByteArray aba;
    aba += out;
  }
  else
  {
    out += "----------------------------------------------------------------------------------------------------------\n";
    out += "head --bytes=256K /var/log/pacman.log\n";
    out += "----------------------------------------------------------------------------------------------------------\n\n";

    command = QStringLiteral("head --bytes=256K /var/log/pacman.log");
    f = QtConcurrent::run(execCommand, command);
    connect(&g_fwCommandToExecute, SIGNAL(finished()), &el, SLOT(quit()));
    g_fwCommandToExecute.setFuture(f);
    el.exec();
    out += g_fwCommandToExecute.result();
  }

  m_commandExecuting = ectn_NONE;
  quint32 gen = QRandomGenerator::global()->generate();
  QString fileName = QStringLiteral("%1%2octopi-sysinfo-%3.log").arg(homePath, QDir::separator()).arg(gen);
  QFile *outFile = new QFile(fileName);
  outFile->open(QIODevice::ReadWrite|QIODevice::Text);
  outFile->setPermissions(QFile::Permissions(QFile::ExeOwner|QFile::ReadOwner));
  outFile->write(out);
  outFile->flush();
  outFile->close();

  writeToTabOutput(QLatin1String("<br>") + StrConstants::getSysInfoGenerated().arg(fileName) + QLatin1String("<br>"));
  writeToTabOutput(QLatin1String("<br><b>") + StrConstants::getCommandFinishedOK() + QLatin1String("</b><br>"));
  enableTransactionActions();
}
