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
#include "outputdialog.h"
#include "../src/strconstants.h"
#include "../src/uihelper.h"
#include "../src/package.h"
#include "../src/transactiondialog.h"
#include "../src/optionsdialog.h"

#include <QTimer>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QScreen>
#include <QtConcurrent/QtConcurrentRun>

#ifdef KSTATUS
  #include <kstatusnotifieritem.h>
#endif

/*
 * This is Octopi Notifier slim interface code :-)
 */

/*
 * The obligatory constructor...
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
  m_pacmanExec = nullptr;
  m_transactionDialog = nullptr;
  m_debugInfo = false;
  m_optionsDialog = nullptr;
  m_numberOfCheckUpdatesPackages = 0;
  m_checkUpdatesNameNewVersion=new QHash<QString, QString>();

  m_pacmanDatabaseSystemWatcher =
            new QFileSystemWatcher(QStringList() << ctn_PACMAN_DATABASE_DIR, this);

  connect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  initActions();
  initSystemTrayIcon();
}

MainWindow::~MainWindow()
{
#ifdef KSTATUS
  delete m_systemTrayIcon;
#endif
}

/*
 * Let's initialize all notifier's actions...
 */
void MainWindow::initActions()
{
  m_actionExit = new QAction(IconHelper::getIconExit(), tr("Exit"), this);
  m_actionExit->setIcon(QIcon::fromTheme("application-exit"));
  connect(m_actionExit, SIGNAL(triggered()), this, SLOT(exitNotifier()));

  m_actionAbout = new QAction(StrConstants::getHelpAbout(), this);
  m_actionAbout->setIcon(QIcon::fromTheme("help-about"));
  m_actionAbout->setIconVisibleInMenu(true);
  connect(m_actionAbout, SIGNAL(triggered()), this, SLOT(aboutOctopiNotifier()));

  m_actionOctopi = new QAction(this);
  m_actionOctopi->setText("Octopi...");
  m_actionOctopi->setIcon(QIcon::fromTheme("octopi"));
  connect(m_actionOctopi, SIGNAL(triggered()), this, SLOT(startOctopi()));

  m_actionOptions = new QAction(this);
  m_actionOptions->setText(StrConstants::getOptions());
  m_actionOptions->setIcon(QIcon::fromTheme("settings-configure"));
  connect(m_actionOptions, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));

  m_actionCheckUpdates = new QAction(this);
  m_actionCheckUpdates->setIconVisibleInMenu(true);
  m_actionCheckUpdates->setText(StrConstants::getCheckUpdates());
  m_actionCheckUpdates->setIcon(IconHelper::getIconCheckUpdates());
  connect(m_actionCheckUpdates, SIGNAL(triggered()), this, SLOT(checkUpdates()));

  m_actionSystemUpgrade = new QAction(this);
  m_actionSystemUpgrade->setIconVisibleInMenu(true);
  m_actionSystemUpgrade->setText(tr("System upgrade"));
  m_actionSystemUpgrade->setIcon(IconHelper::getIconSystemUpgrade());
  connect(m_actionSystemUpgrade, SIGNAL(triggered()), this, SLOT(runOctopiSysUpgrade()));

  m_actionAURUpgrade = new QAction(this);
  m_actionAURUpgrade->setIconVisibleInMenu(true);
  m_actionAURUpgrade->setText(tr("System upgrade"));
  m_actionAURUpgrade->setIcon(IconHelper::getIconForeignGreen());
  connect(m_actionAURUpgrade, SIGNAL(triggered()), this, SLOT(runOctopiAURUpgrade()));
}

/*
 * Let's initialize the system tray object...
 */
void MainWindow::initSystemTrayIcon()
{
  if (m_debugInfo)
    qDebug() << "At initSystemTrayIcon()...";

  m_commandExecuting = ectn_NONE;
  m_outdatedStringList = new QStringList();

#ifdef KSTATUS
  m_systemTrayIcon = new KStatusNotifierItem(0);
#else
  m_systemTrayIcon = new QSystemTrayIcon(this);
#endif

  m_systemTrayIcon->setObjectName("systemTrayIcon");

#ifdef KSTATUS
  m_systemTrayIcon->setIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipIconByPixmap(m_icon);
  m_systemTrayIcon->setTitle("Octopi Notifier");
#else
  m_systemTrayIcon->setIcon(m_icon); 
#endif

  m_systemTrayIconMenu = new QMenu( this );

  if (UnixCommand::hasTheExecutable("octopi"))
    m_systemTrayIconMenu->addAction(m_actionOctopi);

  m_systemTrayIconMenu->addAction(m_actionCheckUpdates);
  m_systemTrayIconMenu->addAction(m_actionAURUpgrade);
  m_systemTrayIconMenu->addAction(m_actionSystemUpgrade);
  m_systemTrayIconMenu->addSeparator();
  m_systemTrayIconMenu->addAction(m_actionOptions);
  m_systemTrayIconMenu->addSeparator();
  m_systemTrayIconMenu->addAction(m_actionAbout);
  m_systemTrayIconMenu->addAction(m_actionExit);
  m_systemTrayIcon->setContextMenu(m_systemTrayIconMenu);

  // disable "standard" actions (restore & quit)
#ifdef KSTATUS
  m_systemTrayIcon->setStandardActionsEnabled(false);
  connect (m_systemTrayIcon, SIGNAL(activateRequested(bool,QPoint)),
           this, SLOT(execSystemTrayKF5()) );
#else
  connect ( m_systemTrayIcon , SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
            this, SLOT( execSystemTrayActivated ( QSystemTrayIcon::ActivationReason ) ) );
#endif

  m_pacmanHelperTimer = new QTimer();
  m_pacmanHelperTimer->setInterval(1000);
  m_pacmanHelperTimer->start();

  connect(m_pacmanHelperTimer, SIGNAL(timeout()), this, SLOT(pacmanHelperTimerTimeout()));
}

/*
 * Whenever this timer ticks, we need to call the PacmanHelper DBus interface to check package updates
 */
void MainWindow::pacmanHelperTimerTimeout()
{
  static bool firstTime=true;

  if (m_commandExecuting != ectn_NONE) return;

  if (firstTime)
  {
    refreshAppIcon();

#ifdef KSTATUS
    m_systemTrayIcon->setToolTipTitle("Octopi Notifier");
#else
    m_systemTrayIcon->show();
#endif

    //From now on, we verify if it's time to check for updates every 5 minutes
    m_pacmanHelperTimer->setInterval(60000 * 5);
    setWindowIcon(m_icon);
    firstTime=false;
  }

  //Is it time to check updates again?
  QDateTime lastCheckTime = SettingsManager::getLastCheckUpdatesTime();
  int checkUpdatesInterval = SettingsManager::getCheckUpdatesInterval();
  QDateTime now = QDateTime::currentDateTime();  
  int checkUpdatesHour = SettingsManager::getCheckUpdatesHour();

  //User did not set the check interval, so we assume it's once a day
  if (checkUpdatesInterval == -1)
  {
    bool checkUpdatesTime = false;

    if (checkUpdatesHour >= 0) //Once a day at a certain time?
    {      
      if (m_debugInfo)
        qDebug() << "CheckUpdates is scheduled once a day, at " << checkUpdatesHour << " hours";

      if (lastCheckTime.daysTo(now) >= 1 && now.time().hour() == checkUpdatesHour)
      {
        checkUpdatesTime = true;
      }
    }
    else
    {
      if (m_debugInfo)
        qDebug() << "CheckUpdates is scheduled once a day";
    }

    if ((checkUpdatesHour == -1 && (
           lastCheckTime.isNull() ||
           lastCheckTime.daysTo(now) >= 1)) || (checkUpdatesTime))
    {
      checkUpdates(ectn_AUTO_CHECK);
      //Then we set new LastCheckTime...
      SettingsManager::setLastCheckUpdatesTime(now);
    }
  }
  else if (checkUpdatesInterval != -2) //Because if it's "-2" user does NOT want any checkupdates!
  {
    if (lastCheckTime.isNull() || now.addSecs(-(checkUpdatesInterval * 60)) >= lastCheckTime)
    {
      checkUpdates(ectn_AUTO_CHECK);
      //Then we set new LastCheckTime...
      SettingsManager::setLastCheckUpdatesTime(now);
    }
    else
    {
      if (m_debugInfo)
        qDebug() << "CheckUpdates is scheduled once every " << checkUpdatesInterval << " minutes.";
    }

    m_pacmanHelperTimer->stop();
    m_pacmanHelperTimer->start();
  }
}

/*
 * Helper to a runOctopi with a call to SystemUpgrade
 */
void MainWindow::runOctopiSysUpgrade()
{
  runOctopi(ectn_SYSUPGRADE_EXEC_OPT);
}

/*
 * Helper to a runOctopi with a call to AUR upgrade
 */
void MainWindow::runOctopiAURUpgrade()
{
  runOctopi(ectn_AUR_UPGRADE_EXEC_OPT);
}

/*
 * Shows Octopi About Dialog...
 */
void MainWindow::aboutOctopiNotifier()
{
  m_actionAbout->setEnabled(false);

  //First we create a fake window to act as about dialog's parent
  //Otherwise the dialog appears at a random screen point!
  QMainWindow *fake = new QMainWindow();
  fake->setWindowIcon(m_icon);
  fake->setVisible(false);
  QScreen *sc = QGuiApplication::primaryScreen();
  fake->setGeometry(sc->geometry());

  QString aboutText = "<b>Octopi Notifier</b><br>";
  aboutText += StrConstants::getVersion() + ": " + StrConstants::getApplicationVersion() + "</b>" + " - " + StrConstants::getQtVersion() + "<br>";
  aboutText += StrConstants::getURL() + ": " + "<a href=\"http://octopiproject.wordpress.com/\">http://octopiproject.wordpress.com</a><br>";
  aboutText += StrConstants::getLicenses() + ": " + QString("<a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GPL v2</a><br>");
  aboutText += "&copy; Alexandre Albuquerque Arnt<br><br>";

  aboutText += "<b>Pacman</b><br>";
  QString pacmanV = UnixCommand::getPacmanVersion();
  if (pacmanV.at(0) == 'v') pacmanV.remove(0, 1);
  aboutText += StrConstants::getVersion() + ": " + pacmanV + "<br>";
  aboutText += StrConstants::getURL() + ": " + "<a href=\"https://www.archlinux.org/pacman/\">https://www.archlinux.org/pacman</a><br>";
  QDate d = QDate::currentDate();
  aboutText += "&copy; 2006-%1 Pacman Development Team<br>";
  aboutText += "&copy; 2002-2006 Judd Vinet";
  aboutText = aboutText.arg(d.year());
  QMessageBox::about(fake, StrConstants::getHelpAbout(), aboutText);

  delete fake;
  m_actionAbout->setEnabled(true);
}

/*
 * Hides Octopi
 */
void MainWindow::hideOctopi()
{  
  QProcess::startDetached("octopi -hide");
}

/*
 * Shows Octopi Window if it is hidden
 */
void MainWindow::showOctopi()
{
  QProcess::startDetached("octopi -show");
}

/*
 * Checks if some SU utility is available...
 * Returns false if not!
 */
bool MainWindow::_isSUAvailable()
{
  //If there are no means to run the actions, we must warn!
  if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND)
  {
    QMessageBox::critical( 0, StrConstants::getApplicationName(),
                           StrConstants::getErrorNoSuCommand() +
                           "\n" + StrConstants::getYoullNeedSuFrontend());
    return false;
  }
  else
    return true;
}

void MainWindow::setUpgradingTooltip()
{
#ifdef KSTATUS
  m_systemTrayIcon->setToolTipSubTitle(StrConstants::getUpgrading());
#else
  m_systemTrayIcon->setToolTip(StrConstants::getUpgrading());
#endif
}

/*
 * Calls only the Octopi system upgrade window
 */
void MainWindow::doSystemUpgrade()
{
  if (!isInternetAvailable()) return;

  if(!SettingsManager::getEnableConfirmationDialogInSysUpgrade())
  {
    if( (m_checkUpdatesStringList.count() != 0 && m_checkUpdatesStringList.contains("pacman")) ||
        (m_outdatedStringList->count() != 0 && m_outdatedStringList->contains("pacman")) )
    {
      m_systemUpgradeDialog = false;

      //If there are no means to run the actions, we must warn!
      if (!_isSUAvailable()) return;

      OutputDialog *dlg = new OutputDialog(this);
      dlg->setViewAsTextBrowser(false);
      QObject::connect(dlg, SIGNAL( finished(int)),
                       this, SLOT( doSystemUpgradeFinished() ));

      m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
      m_actionSystemUpgrade->setEnabled(false);
      setUpgradingTooltip();
      dlg->show();
      dlg->doSystemUpgradeInTerminal();
    }
    else
    {
      m_commandExecuting = ectn_SYSTEM_UPGRADE;
      m_systemUpgradeDialog = false;
      toggleEnableInterface(false);
      m_actionSystemUpgrade->setEnabled(false);

      OutputDialog *dlg = new OutputDialog(this);
      dlg->setViewAsTextBrowser(true);

      if (m_debugInfo)
        dlg->setDebugMode(true);

      QObject::connect(dlg, SIGNAL( finished(int)),
                       this, SLOT( doSystemUpgradeFinished() ));
      setUpgradingTooltip();
      dlg->show();
      dlg->doSystemUpgrade();
    }
  }
  else
  {
    if (m_transactionDialog != nullptr)
    {
      if (m_transactionDialog->isMinimized())
        m_transactionDialog->setWindowState(Qt::WindowNoState);
      else
        m_transactionDialog->activateWindow();
      return;
    }

    QList<PackageListData> * targets = Package::getTargetUpgradeList();

    //There are no new updates to install!
    if (targets->count() == 0 && m_numberOfOutdatedPackages == 0)
    {
      return;
    }
    else if (targets->count() == 0 && m_checkUpdatesStringList.count() == 0)
    {
      return;
    }

    QString list;
    double totalDownloadSize = 0;

    if (m_checkUpdatesStringList.count() > m_outdatedStringList->count())
    {
      targets->clear();
      foreach(QString name, m_checkUpdatesStringList)
      {
        PackageListData aux;
        /*QString size;
        if (package)
        {
          size = size.number(package->downloadSize, 'f', 0);
        }*/

        aux = PackageListData(name, m_checkUpdatesNameNewVersion->value(name), "0");
        targets->append(aux);
      }
    }

    foreach(PackageListData target, *targets)
    {
      totalDownloadSize += target.downloadSize;
      list = list + target.name + "-" + target.version + "\n";
    }
    list.remove(list.size()-1, 1);

    QString ds = Package::kbytesToSize(totalDownloadSize);
    m_transactionDialog = new TransactionDialog(this);

    if(targets->count()==1)
      m_transactionDialog->setText(StrConstants::getRetrievePackage() +
                                   "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));
    else
      m_transactionDialog->setText(StrConstants::getRetrievePackages(targets->count()) +
                                   "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));

    m_transactionDialog->setWindowTitle(StrConstants::getConfirmation());
    m_transactionDialog->setInformativeText(StrConstants::getConfirmationQuestion());
    m_transactionDialog->setDetailedText(list);
    m_systemUpgradeDialog = true;
    int result = m_transactionDialog->exec();
    m_transactionDialog = nullptr;

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_SYSTEM_UPGRADE;
      m_systemUpgradeDialog = false;
      toggleEnableInterface(false);
      m_actionSystemUpgrade->setEnabled(false);

      OutputDialog *dlg = new OutputDialog(this);
      dlg->setViewAsTextBrowser(true);

      if (m_debugInfo)
        dlg->setDebugMode(true);

      QObject::connect(dlg, SIGNAL( finished(int)),
                       this, SLOT( doSystemUpgradeFinished() ));
      setUpgradingTooltip();
      dlg->show();
      dlg->doSystemUpgrade();
    }
    else if(result == QDialogButtonBox::AcceptRole)
    {
      m_systemUpgradeDialog = false;

      //If there are no means to run the actions, we must warn!
      if (!_isSUAvailable()) return;

      OutputDialog *dlg = new OutputDialog(this);
      dlg->setViewAsTextBrowser(false);
      QObject::connect(dlg, SIGNAL( finished(int)),
                       this, SLOT( doSystemUpgradeFinished() ));

      m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
      m_actionSystemUpgrade->setEnabled(false);
      setUpgradingTooltip();
      dlg->show();
      dlg->doSystemUpgradeInTerminal();
    }
    else if (result == QDialogButtonBox::No)
    {
      m_systemUpgradeDialog = false;
      toggleEnableInterface(true);
      refreshAppIcon();
    }
  }
}

/*
 * Calls the OutputDialog with TermWidget to execute the AUR upgrade commands
 */
void MainWindow::doAURUpgrade()
{
  QString listOfTargets;
  QString auxPkg;

  foreach(QString pkg, *m_outdatedAURStringList)
  {
    auxPkg = pkg;
    auxPkg.remove("[1;39m");
    auxPkg.remove("[0m");
    auxPkg.remove("");
    listOfTargets += auxPkg + " ";
  }

  OutputDialog *dlg = new OutputDialog(this);
  dlg->setViewAsTextBrowser(false);
  dlg->setListOfAURPackagesToUpgrade(listOfTargets);

  QObject::connect(dlg, SIGNAL( finished(int)),
                   this, SLOT( doSystemUpgradeFinished() ));
  dlg->show();
  dlg->doAURUpgrade();
}

/*
 * When system upgrade process has finished...
 */
void MainWindow::doSystemUpgradeFinished()
{
  m_commandExecuting = ectn_NONE;
  m_checkUpdatesStringList.clear();
  m_checkUpdatesNameNewVersion->clear();
  m_numberOfCheckUpdatesPackages=0;
  refreshAppIcon();

  //Does it still need to upgrade another packages due to any issues???
  /*if (m_outdatedStringList->count() > 0)
  {
    m_commandExecuting = ectn_NONE;
    doSystemUpgrade();
    return;
  }*/

  m_unixCommand->removeTemporaryFile();
  toggleEnableInterface(true);
}

/*
 * Enables and Disables some UI elements of Octopi-notifier
 */
void MainWindow::toggleEnableInterface(bool state)
{
  m_actionOctopi->setEnabled(state);
  m_actionCheckUpdates->setEnabled(state);
  m_actionOptions->setEnabled(state);
  m_actionSystemUpgrade->setEnabled(state);
  m_actionExit->setEnabled(state);
}

/*
 * Called right after "checkupdates" has finished!
 */
void MainWindow::afterCheckUpdates(int exitCode, QProcess::ExitStatus)
{
  if (m_debugInfo)
    qDebug() << "At afterCheckUpdates()...";
  toggleEnableInterface(true);

  if (exitCode != 0) return;

  SettingsManager::setLastCheckUpdatesTime(QDateTime::currentDateTime());

#ifndef KSTATUS
  m_systemTrayIcon->setContextMenu(m_systemTrayIconMenu);
  m_systemTrayIconMenu->close();
#endif

  QStringList checkUpdatesList = m_pacmanExec->getOutdatedPackages();
  m_checkUpdatesStringList.clear();
  m_checkUpdatesNameNewVersion->clear();

  m_commandExecuting = ectn_NONE;

  foreach(QString line, checkUpdatesList)
  {
    QStringList aux = line.split(" ", QString::SkipEmptyParts);

    m_checkUpdatesStringList.append(aux.at(0));
    m_checkUpdatesNameNewVersion->insert(aux.at(0), aux.at(3));
  }

  m_numberOfCheckUpdatesPackages = m_checkUpdatesStringList.count();
  int numberOfOutdatedPackages = m_numberOfOutdatedPackages;
  refreshAppIcon();

  if (numberOfOutdatedPackages != m_numberOfOutdatedPackages)
  {
    if (m_numberOfOutdatedPackages > 0)
    {
      QString notification;

      if (m_numberOfOutdatedPackages == 1)
      {
        notification = StrConstants::getOneNewUpdate();

        #ifdef KSTATUS
          m_systemTrayIcon->setToolTipSubTitle(notification);
          m_systemTrayIcon->showMessage("Octopi",
                                        notification, m_systemTrayIcon->iconName());
        #else
          m_systemTrayIcon->setToolTip(notification);
        #endif
      }
      else if (m_numberOfOutdatedPackages > 1)
      {
        notification = StrConstants::getNewUpdates(m_numberOfOutdatedPackages);

        #ifdef KSTATUS
          m_systemTrayIcon->setToolTipSubTitle(notification);
          m_systemTrayIcon->showMessage("Octopi",
                                        notification, m_systemTrayIcon->iconName());
        #else
          m_systemTrayIcon->setToolTip(notification);
        #endif
      }
    }
  }
  else
  {
    QString notification;

    if (numberOfOutdatedPackages == 1)
    {
      notification = StrConstants::getOneNewUpdate();

      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(notification);
        m_systemTrayIcon->showMessage("Octopi",
                                      notification, m_systemTrayIcon->iconName());
      #else
        m_systemTrayIcon->setToolTip(notification);
      #endif
    }
    else if (numberOfOutdatedPackages > 1)
    {
      notification = StrConstants::getNewUpdates(numberOfOutdatedPackages);

      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(notification);
        m_systemTrayIcon->showMessage("Octopi",
                                      notification, m_systemTrayIcon->iconName());
      #else
        m_systemTrayIcon->setToolTip(notification);
      #endif
    }
  }
}

/*
 * Checks if Internet connection is up/down
 */
bool MainWindow::isInternetAvailable()
{
  bool res=true;

  //First we create a fake window to act as about dialog's parent
  //Otherwise the dialog appears at a random screen point!
  QMainWindow *fake = new QMainWindow();
  fake->setWindowIcon(m_icon);
  fake->setVisible(false);
  QScreen *sc = QGuiApplication::primaryScreen();
  fake->setGeometry(sc->geometry());

  //Test if Internet access exists
  if (!UnixCommand::hasInternetConnection())
  {
    QMessageBox::critical(fake, StrConstants::getError(), StrConstants::getInternetUnavailableError());
    res=false;
    delete fake;
  }

  return res;
}

/*
 * Called every time user selects "Check updates..." menu option
 */
void MainWindow::checkUpdates(CheckUpdate check)
{
  if (check == ectn_AUTO_CHECK)
  {
    if (!UnixCommand::hasInternetConnection()) return;
  }
  else if (check == ectn_USER_CHECK)
  {
    if (!isInternetAvailable()) return;
  }

  disconnect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  QTime now;
  if (m_debugInfo)
    qDebug() << now.currentTime().toString("HH:mm").toLatin1().data() <<  ": At checkUpdates()...";
  toggleEnableInterface(false);
  m_icon = IconHelper::getIconOctopiBusy();

#ifdef KSTATUS
  m_systemTrayIcon->setIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipSubTitle(StrConstants::getCheckingForUpdates());
#else
  m_systemTrayIcon->setIcon(m_icon);
  m_systemTrayIcon->setToolTip(StrConstants::getCheckingForUpdates());
#endif

  qApp->processEvents();

  m_systemTrayIconMenu->close();

#ifndef KSTATUS
  m_systemTrayIcon->setContextMenu(0);
#endif

  m_commandExecuting = ectn_CHECK_UPDATES;

  //Let's synchronize kcp database...
  if (UnixCommand::getLinuxDistro() == ectn_KAOS && UnixCommand::hasTheExecutable("kcp"))
  {
    if (m_debugInfo)
      qDebug() << "Synchronizing kcp database...";
    UnixCommand::execCommandAsNormalUser("kcp -u");
  }

  m_pacmanExec = new PacmanExec(this);
  m_commandExecuting = ectn_CHECK_UPDATES;
  m_pacmanExec->doCheckUpdates();
  connect(m_pacmanExec, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(afterCheckUpdates(int, QProcess::ExitStatus)));
}

/*
 * Uses notify-send to send a notification to the systray area
 */
void MainWindow::sendNotification(const QString &msg)
{
  QString processToExec("notify-send");

  if (UnixCommand::hasTheExecutable(processToExec))
  {
    processToExec += " -i /usr/share/icons/octopi_red.png -t 5000 \"" +
        StrConstants::getApplicationName() + "\"  \"" + msg + "\"";
    QProcess::startDetached(processToExec);
  }
}

/*
 * If we have some outdated packages, let's put an angry red face icon in this app!
 */
void MainWindow::refreshAppIcon()
{
  if (m_commandExecuting != ectn_NONE) return;

  if (m_pacmanExec != nullptr)
  {
    delete m_pacmanExec;
    m_pacmanExec = nullptr;
  }

  disconnect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  if (m_debugInfo)
    qDebug() << "At refreshAppIcon()...";
  m_outdatedStringList = Package::getOutdatedStringList();

  //We only need to check for outdated AUR pkgs IF we do NOT have outdated standard ones!
  if (m_outdatedStringList->count() == 0)
  {
    bool hasAURTool = UnixCommand::hasTheExecutable(Package::getForeignRepositoryToolName());

    if (hasAURTool && SettingsManager::getSearchOutdatedAURPackages())
    {
      m_outdatedAURStringList = Package::getOutdatedAURStringList();

      for(int c=0; c<m_outdatedAURStringList->count(); ++c)
      {
        //If we find an outdated AUR pkg in the official pkg list, let's remove it
        if (UnixCommand::hasPackage(m_outdatedAURStringList->at(c)))
        {
          m_outdatedAURStringList->removeAt(c);
        }
      }
    }
    else
    {
      m_outdatedAURStringList = new QStringList();
    }
  }
  else
  {
    m_outdatedAURStringList = new QStringList();
  }

  m_numberOfOutdatedPackages = m_outdatedStringList->count();
  m_numberOfOutdatedAURPackages = m_outdatedAURStringList->count();

  if (m_numberOfOutdatedPackages < m_numberOfCheckUpdatesPackages)
    m_numberOfOutdatedPackages=m_numberOfCheckUpdatesPackages;

  if (m_numberOfOutdatedPackages == 0 && m_numberOfOutdatedAURPackages == 0)
  {
    #ifdef KSTATUS
      m_systemTrayIcon->setToolTipSubTitle("Octopi Notifier");
    #else
      m_systemTrayIcon->setToolTip("Octopi Notifier");
    #endif
  }
  else if (m_numberOfOutdatedPackages > 0)
  {
    if (m_numberOfOutdatedPackages == 1)
    {
      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(StrConstants::getOneNewUpdate());
      #else
        m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate());
      #endif
    }
    else if (m_numberOfOutdatedPackages > 1)
    {
      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(
              StrConstants::getNewUpdates(m_numberOfOutdatedPackages));
      #else
        m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates(m_numberOfOutdatedPackages));
      #endif
    }
  }
  else if (m_numberOfOutdatedAURPackages > 0)
  {
    if (m_numberOfOutdatedAURPackages == 1)
    {
      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(StrConstants::getOneNewUpdate() +
                                             " (" + StrConstants::getForeignRepositoryName() + ")");
      #else
        m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate() +
                                     " (" + StrConstants::getForeignRepositoryName() + ")");
      #endif
    }
    else if (m_numberOfOutdatedAURPackages > 1)
    {
      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(
              StrConstants::getNewUpdates(m_numberOfOutdatedAURPackages) +
              " (" + StrConstants::getForeignRepositoryName() + ")");
      #else
        m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates(m_numberOfOutdatedAURPackages) +
                                     " (" + StrConstants::getForeignRepositoryName() + ")");
      #endif
    }
  }

  if(m_numberOfOutdatedPackages > 0) //RED ICON!
  {
    if(m_commandExecuting == ectn_NONE)
    {
      m_actionAURUpgrade->setVisible(false);
      m_actionSystemUpgrade->setEnabled(true);
      m_actionSystemUpgrade->setVisible(true);
    }

    if (m_debugInfo)
      qDebug() << "Got a RED icon!";

    m_icon = IconHelper::getIconOctopiRed();


#ifdef KSTATUS
    m_systemTrayIcon->setAttentionIconByPixmap(m_icon);
    m_systemTrayIcon->setStatus(KStatusNotifierItem::NeedsAttention);
#endif
  }
  else if(m_outdatedAURStringList->count() > 0) //YELLOW ICON!
  {
    m_actionAURUpgrade->setEnabled(true);
    m_actionAURUpgrade->setVisible(true);
    m_actionSystemUpgrade->setVisible(false);
    m_icon = IconHelper::getIconOctopiYellow();
    if (m_debugInfo)
      qDebug() << "Got a YELLOW icon!";

#ifdef KSTATUS
    m_systemTrayIcon->setAttentionIconByPixmap(m_icon);
    m_systemTrayIcon->setStatus(KStatusNotifierItem::NeedsAttention);
#endif
  }
  else //YEAHHH... GREEN ICON!
  {
    m_actionAURUpgrade->setVisible(false);
    m_actionSystemUpgrade->setVisible(false);
    m_icon = IconHelper::getIconOctopiGreen();
    if (m_debugInfo)
      qDebug() << "Got a GREEN icon!";

#ifdef KSTATUS
    m_systemTrayIcon->setStatus(KStatusNotifierItem::Passive);
#endif
  }

  setWindowIcon(m_icon);

#ifdef KSTATUS
  m_systemTrayIcon->setIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipIconByPixmap(m_icon);
#else
  m_systemTrayIcon->setIcon(m_icon);
#endif

  connect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  if (m_outdatedStringList->count() > 0)
    QtConcurrent::run(UnixCommand::execCommandAsNormalUserExt, ctn_PACMAN_SUP_COMMAND);
}

/*
 * Whenever the user clicks on the systemTray icon...
 */
void MainWindow::execSystemTrayActivated(QSystemTrayIcon::ActivationReason ar)
{
  if (m_commandExecuting != ectn_NONE) return;

  switch (ar)
  {
  case QSystemTrayIcon::DoubleClick:
  {
    if (m_outdatedStringList->count() > 0)
    {
      runOctopi(ectn_SYSUPGRADE_EXEC_OPT);
    }
    else
    {
      runOctopi(ectn_NORMAL_EXEC_OPT);
    }

    break;
  }
  case QSystemTrayIcon::Trigger:
  {
    if (UnixCommand::isAppRunning("octopi", true))
    {
      hideOctopi();
    }
    else
    {
      runOctopi(ectn_NORMAL_EXEC_OPT);
    }

    break;
  }
  default: break;
  }
}

/*
 * This slot is called only when we're using Knotifications from KF5
 */
void MainWindow::execSystemTrayKF5()
{
  if (UnixCommand::isAppRunning("octopi", true))
  {
    static bool hidingOctopi = true;

    if (!hidingOctopi)
      runOctopi(ectn_NORMAL_EXEC_OPT);
    else
      hideOctopi();

    hidingOctopi = !hidingOctopi;
  }
}

/*
 * When the users quit this notifier...
 */
void MainWindow::exitNotifier()
{
  if (m_debugInfo)
    qDebug() << "At exitNotifier()...";

  //If Octopi was hidden, let's show its window again...
  if (UnixCommand::isAppRunning("octopi", true)) showOctopi();
  qApp->quit();
}

/*
 * Execs Octopi
 */
void MainWindow::runOctopi(ExecOpt execOptions)
{
  if (execOptions == ectn_SYSUPGRADE_NOCONFIRM_EXEC_OPT)
  {
    if (!WMHelper::isKDERunning() && (!WMHelper::isLXQTRunning()))
    {
      QProcess::startDetached("octopi -sysupgrade-noconfirm -style gtk");
    }
    else
    {
      QProcess::startDetached("octopi -sysupgrade-noconfirm");
    }
  }
  else if (execOptions == ectn_SYSUPGRADE_EXEC_OPT &&
      !UnixCommand::isAppRunning("octopi", true) && (m_outdatedStringList->count() > 0 || m_checkUpdatesStringList.count() > 0))
  {
    doSystemUpgrade();
  }
  else if (execOptions == ectn_SYSUPGRADE_EXEC_OPT &&
      UnixCommand::isAppRunning("octopi", true) && (m_outdatedStringList->count() > 0 || m_checkUpdatesStringList.count() > 0))
  {
    if (!WMHelper::isKDERunning() && (!WMHelper::isLXQTRunning()))
    {
      QProcess::startDetached("octopi -sysupgrade -style gtk");
    }
    else
    {
      QProcess::startDetached("octopi -sysupgrade");
    }
  }
  else if (execOptions == ectn_AUR_UPGRADE_EXEC_OPT &&
      !UnixCommand::isAppRunning("octopi", true) && m_outdatedAURStringList->count() > 0)
  {
    doAURUpgrade();
  }
  else if (execOptions == ectn_AUR_UPGRADE_EXEC_OPT &&
      UnixCommand::isAppRunning("octopi", true) && m_outdatedAURStringList->count() > 0)
  {
    QProcess::startDetached("octopi -aurupgrade");
  }
  else if (execOptions == ectn_NORMAL_EXEC_OPT)
  {
    if (!WMHelper::isKDERunning() && (!WMHelper::isLXQTRunning()))
    {
      QProcess::startDetached("octopi -style gtk");
    }
    else
    {
      QProcess::startDetached("octopi");
    }
  }
}

/*
 * Calls the QDialog to set notifier interval
 */
void MainWindow::showOptionsDialog()
{
  if (m_optionsDialog == nullptr)
  {
    m_optionsDialog = new OptionsDialog(this);
    connect(m_optionsDialog, SIGNAL(AURToolChanged()), this, SLOT(refreshAppIcon()));
    utils::positionWindowAtScreenCenter(m_optionsDialog);
    m_optionsDialog->exec();

    Options::result res = m_optionsDialog->result();
    if (res & Options::ectn_ICON)
    {
      refreshAppIcon();
    }

    delete m_optionsDialog;
    m_optionsDialog = nullptr;
  }
}
