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
#include "../pacmanhelper/pacmanhelperclient.h"
#include "../../src/strconstants.h"
#include "../../src/uihelper.h"
#include "../../src/package.h"
#include "../../src/transactiondialog.h"
#include "../../src/optionsdialog.h"

#include <QTimer>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>

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
  connect(m_actionExit, SIGNAL(triggered()), this, SLOT(exitNotifier()));

  m_actionAbout = new QAction(StrConstants::getHelpAbout(), this);
  m_actionAbout->setIconVisibleInMenu(true);
  connect(m_actionAbout, SIGNAL(triggered()), this, SLOT(aboutOctopiNotifier()));

  m_actionOctopi = new QAction(this);
  m_actionOctopi->setText("Octopi...");
  connect(m_actionOctopi, SIGNAL(triggered()), this, SLOT(startOctopi()));

  m_actionOptions = new QAction(this);
  m_actionOptions->setText(StrConstants::getOptions());
  connect(m_actionOptions, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));

  m_actionSyncDatabase = new QAction(this);
  m_actionSyncDatabase->setIconVisibleInMenu(true);
  m_actionSyncDatabase->setText(StrConstants::getSyncDatabase());
  m_actionSyncDatabase->setIcon(IconHelper::getIconSyncDatabase());
  connect(m_actionSyncDatabase, SIGNAL(triggered()), this, SLOT(syncDatabase()));

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

  m_systemTrayIconMenu->addAction(m_actionSyncDatabase);
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

  m_pacmanHelperClient = new PacmanHelperClient("org.octopi.pacmanhelper", "/", QDBusConnection::systemBus(), 0);
  connect(m_pacmanHelperClient, SIGNAL(syncdbcompleted()), this, SLOT(afterPacmanHelperSyncDatabase()));

  m_pacmanHelperTimer = new QTimer();
  m_pacmanHelperTimer->setInterval(1000);
  m_pacmanHelperTimer->start();

  connect(m_pacmanHelperTimer, SIGNAL(timeout()), this, SLOT(pacmanHelperTimerTimeout()));
}

/*
 * Whenever this timer ticks, we need to call the PacmanHelper DBus interface to sync Pacman's dbs
 */
void MainWindow::pacmanHelperTimerTimeout()
{
  static bool firstTime=true;

  if (!UnixCommand::hasInternetConnection() || m_commandExecuting != ectn_NONE) return;

  if (firstTime)
  {
    refreshAppIcon();

#ifdef KSTATUS
    m_systemTrayIcon->setToolTipTitle("Octopi");
#else
    m_systemTrayIcon->show();
#endif

    //From now on, we verify if it's time to check for updates every 5 minutes
    m_pacmanHelperTimer->setInterval(60000 * 5);
    setWindowIcon(m_icon);
    firstTime=false;
  }

  //Is it time to syncdb again?
  QDateTime lastCheckTime = SettingsManager::getLastSyncDbTime();
  int syncDbInterval = SettingsManager::getSyncDbInterval();
  QDateTime now = QDateTime::currentDateTime();
  bool syncTime = false;
  int syncHour = SettingsManager::getSyncDbHour();

  //User did not set the check interval, so we assume it's once a day
  if (syncDbInterval == -1)
  {
    if (syncHour >= 0) //Once a day at a certain time?
    {
      if (m_debugInfo)
        qDebug() << "SyncDb is scheduled once a day, at " << syncHour << " hours";

      if (lastCheckTime.daysTo(now) >= 1 && now.time().hour() == syncHour)
      {
        syncTime = true;
      }
    }
    else
    {
      if (m_debugInfo)
        qDebug() << "SyncDb is scheduled once a day";
    }

    if ((syncHour == -1 && (
           lastCheckTime.isNull() ||
           lastCheckTime.daysTo(now) >= 1)) || (syncTime))
    {
      syncDatabase();
      //Then we set new LastCheckTime...
      SettingsManager::setLastSyncDbTime(now);
    }
  }
  else
  {
    if (lastCheckTime.isNull() || now.addSecs(-(syncDbInterval * 60)) >= lastCheckTime)
    {
      syncDatabase();
      //Then we set new LastCheckTime...
      SettingsManager::setLastSyncDbTime(now);
    }
    else
    {
      if (m_debugInfo)
        qDebug() << "SyncDb is scheduled once every " << syncDbInterval << " minutes.";
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

  QString aboutText = "<b>Octopi Notifier - " +
      StrConstants::getApplicationVersion() + "</b>" + " (" + StrConstants::getQtVersion() + ")<br>";
  aboutText += "<a href=\"http://octopiproject.wordpress.com/\">http://octopiproject.wordpress.com</a><br>";
  aboutText += "&copy; Alexandre Albuquerque Arnt<br><br>";
  aboutText += "<b>Pacman - " + UnixCommand::getPacmanVersion() + "</b><br>";
  aboutText += "<a href=\"https://www.archlinux.org/pacman/\">https://www.archlinux.org/pacman</a><br>";
  QDate d = QDate::currentDate();
  aboutText += "&copy; 2006-%1 Pacman Development Team<br>";
  aboutText += "&copy; 2002-2006 Judd Vinet";
  aboutText = aboutText.arg(d.year());
  QMessageBox::about(this, StrConstants::getHelpAbout(), aboutText);

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

/*
 * Calls only the Octopi system upgrade window
 */
void MainWindow::doSystemUpgrade()
{
  if (m_transactionDialog != nullptr)
  {
    if (m_transactionDialog->isMinimized())
      m_transactionDialog->setWindowState(Qt::WindowNoState);
    else
      m_transactionDialog->activateWindow();
    return;
  }

  //Shows a dialog indicating the targets needed to be retrieved and asks for the user's permission.
  QList<PackageListData> * targets = Package::getTargetUpgradeList();

  //There are no new updates to install!
  if (targets->count() == 0 && m_outdatedStringList->count() == 0)
  {
    return;
  }
  else if (targets->count() == 0 && m_outdatedStringList->count() > 0)
  {
    return;
  }

  QString list;
  double totalDownloadSize = 0;

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

    if (m_debugInfo)
      dlg->setDebugMode(true);

    QObject::connect(dlg, SIGNAL( finished(int)),
                     this, SLOT( doSystemUpgradeFinished() ));
    dlg->show();
  }
  else if(result == QDialogButtonBox::AcceptRole)
  {
    m_systemUpgradeDialog = false;

    //If there are no means to run the actions, we must warn!
    if (!_isSUAvailable()) return;

    QStringList lastCommandList;
    lastCommandList.append("pacman -Su;");
    lastCommandList.append("echo -e;");
    lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( doSystemUpgradeFinished() ));

    toggleEnableInterface(false);
    m_actionSystemUpgrade->setEnabled(false);

    if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
      m_unixCommand->runCommandInTerminal(lastCommandList);
    }
  }
  else if (result == QDialogButtonBox::No)
  {   
    m_systemUpgradeDialog = false;
    toggleEnableInterface(true);
    refreshAppIcon();
  }
}

/*
 * Calls the chosen terminal and run a system upgrade of the outdated AUR targets
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

  m_pacmanExec = new PacmanExec();

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( refreshAppIcon()) );

  m_pacmanExec->doAURUpgrade(listOfTargets);
}

/*
 * When system upgrade process has finished...
 */
void MainWindow::doSystemUpgradeFinished()
{
  m_commandExecuting = ectn_NONE;
  refreshAppIcon();

  //Does it still need to upgrade another packages due to SyncFirst issues???
  if ((m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
      && m_outdatedStringList->count() > 0)
  {
    m_commandExecuting = ectn_NONE;
    m_unixCommand->removeTemporaryFile();

    doSystemUpgrade();

    return;
  }

  m_unixCommand->removeTemporaryFile();
  toggleEnableInterface(true);
}

/*
 * Enables and Disables some UI elements of Octopi-notifier
 */
void MainWindow::toggleEnableInterface(bool state)
{
  m_actionOctopi->setEnabled(state);
  m_actionSyncDatabase->setEnabled(state);
  m_actionOptions->setEnabled(state);
  m_actionSystemUpgrade->setEnabled(state);
  m_actionExit->setEnabled(state);
}

/*
 * Called right after the PacmanHelper syncdb() method has finished!
 */
void MainWindow::afterPacmanHelperSyncDatabase()
{
  if (m_debugInfo)
    qDebug() << "At afterPacmanHelperSyncDatabase()...";
  toggleEnableInterface(true);

#ifndef KSTATUS
  m_systemTrayIcon->setContextMenu(m_systemTrayIconMenu);
  m_systemTrayIconMenu->close();
#endif

  m_commandExecuting = ectn_NONE;

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
 * Called every time user selects "Sync databases..." menu option
 */
void MainWindow::syncDatabase()
{
  disconnect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  QTime now;
  if (m_debugInfo)
    qDebug() << now.currentTime().toString("HH:mm").toLatin1().data() <<  ": At syncDatabase()...";
  toggleEnableInterface(false);
  m_icon = IconHelper::getIconOctopiBusy();

#ifdef KSTATUS
  m_systemTrayIcon->setIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipIconByPixmap(m_icon);
  m_systemTrayIcon->setToolTipSubTitle(StrConstants::getSyncDatabases());
#else
  m_systemTrayIcon->setIcon(m_icon);
  m_systemTrayIcon->setToolTip(StrConstants::getSyncDatabases());
#endif

  qApp->processEvents();

  m_systemTrayIconMenu->close();

#ifndef KSTATUS
  m_systemTrayIcon->setContextMenu(0);
#endif

  m_commandExecuting = ectn_SYNC_DATABASE;

  //Let's synchronize kcp database too...
  if (UnixCommand::getLinuxDistro() == ectn_KAOS && UnixCommand::hasTheExecutable("kcp"))
  {
    if (m_debugInfo)
      qDebug() << "Synchronizing kcp database...";
    UnixCommand::execCommandAsNormalUser("kcp -u");
  }

  m_pacmanHelperClient->syncdb();
  SettingsManager::setLastSyncDbTime(QDateTime::currentDateTime());
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
  if (m_pacmanExec != nullptr) delete m_pacmanExec;

  disconnect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  if (m_debugInfo)
    qDebug() << "At refreshAppIcon()...";
  m_outdatedStringList = Package::getOutdatedStringList();

  bool hasAURTool = UnixCommand::hasTheExecutable(Package::getForeignRepositoryToolName());

  if (hasAURTool)
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

  m_numberOfOutdatedPackages = m_outdatedStringList->count();
  m_numberOfOutdatedAURPackages = m_outdatedAURStringList->count();

  if (m_numberOfOutdatedPackages == 0 && m_numberOfOutdatedAURPackages == 0)
  {
    #ifdef KSTATUS
      m_systemTrayIcon->setToolTipSubTitle("");
    #else
      m_systemTrayIcon->setToolTip("");
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

  if(m_outdatedStringList->count() > 0) //RED ICON!
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
  static bool hidingOctopi = true;

  if (UnixCommand::isAppRunning("octopi", true))
  {
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
      !UnixCommand::isAppRunning("octopi", true) && m_outdatedStringList->count() > 0)
  {
    doSystemUpgrade();
  }
  else if (execOptions == ectn_SYSUPGRADE_EXEC_OPT &&
      UnixCommand::isAppRunning("octopi", true) && m_outdatedStringList->count() > 0)
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
