#include "mainwindow.h"
#include "../../src/strconstants.h"
#include "../../src/uihelper.h"
#include "../../src/package.h"
#include "../pacmanhelper/pacmanhelperclient.h"
#include "../../src/transactiondialog.h"

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
  qDebug() << "At MainWindow constructor...";

  m_pacmanDatabaseSystemWatcher =
            new QFileSystemWatcher(QStringList() << ctn_PACMAN_DATABASE_DIR, this);

  connect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  initSystemTrayIcon();
}

MainWindow::~MainWindow()
{
#ifdef KSTATUS
  delete m_systemTrayIcon;
#endif
}

/*
 * Let's initialize the system tray object...
 */
void MainWindow::initSystemTrayIcon()
{
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

  m_actionExit = new QAction(IconHelper::getIconExit(), tr("Exit"), this);
  connect(m_actionExit, SIGNAL(triggered()), this, SLOT(exitNotifier()));

  m_actionAbout = new QAction(StrConstants::getHelpAbout(), this);
  m_actionAbout->setIconVisibleInMenu(true);
  connect(m_actionAbout, SIGNAL(triggered()), this, SLOT(aboutOctopiNotifier()));

  m_actionOctopi = new QAction(this);
  m_actionOctopi->setText("Octopi...");
  connect(m_actionOctopi, SIGNAL(triggered()), this, SLOT(startOctopi()));

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

  m_systemTrayIconMenu = new QMenu( this );

  if (UnixCommand::hasTheExecutable("octopi"))
    m_systemTrayIconMenu->addAction(m_actionOctopi);

  m_systemTrayIconMenu->addAction(m_actionSyncDatabase);
  m_systemTrayIconMenu->addAction(m_actionSystemUpgrade);
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
  QDateTime now = QDateTime::currentDateTime();
  bool syncTime = false;

  int syncHour = SettingsManager::getSyncDbHour();
  if (syncHour >= 0)
  {
    if (lastCheckTime.daysTo(now) >= 1 && now.time().hour() == syncHour)
    {
      syncTime = true;
    }
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

/*
 * Helper to a runOctopi with a call to SystemUpgrade
 */
void MainWindow::runOctopiSysUpgrade()
{
  runOctopi(ectn_SYSUPGRADE_EXEC_OPT);
}

/*
 * Shows Octopi About Dialog...
 */
void MainWindow::aboutOctopiNotifier()
{
  QString aboutText = "<b>Octopi Notifier - " + StrConstants::getApplicationVersion() + "</b>" + " (" + StrConstants::getQtVersion() + ")<br>";
  aboutText += "<a href=\"http://octopiproject.wordpress.com/\">http://octopiproject.wordpress.com</a><br><br>";
  aboutText += "&copy; Alexandre Albuquerque Arnt";

  QMessageBox::about(this, StrConstants::getHelpAbout(), aboutText);
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

  totalDownloadSize = totalDownloadSize / 1024;
  QString ds = Package::kbytesToSize(totalDownloadSize);
  TransactionDialog question(this);

  //If we're in Chakra, there are no graphical system upgrades!
  if (UnixCommand::getLinuxDistro() == ectn_CHAKRA || !UnixCommand::hasTheExecutable("octopi"))
    question.removeYesButton();

  if(targets->count()==1)
    question.setText(StrConstants::getRetrievePackage() +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));
  else
    question.setText(StrConstants::getRetrievePackages(targets->count()) +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));

  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);

  m_systemUpgradeDialog = true;
  int result = question.exec();

  if (result == QDialogButtonBox::Yes)
  {
    runOctopi(ectn_SYSUPGRADE_NOCONFIRM_EXEC_OPT);
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
                     this, SLOT( doSystemUpgradeFinished(int, QProcess::ExitStatus) ));

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
 * When system upgrade process is finished...
 */
void MainWindow::doSystemUpgradeFinished(int, QProcess::ExitStatus)
{
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

  m_commandExecuting = ectn_NONE;
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
  m_actionExit->setEnabled(state);
}

/*
 * Called right after the PacmanHelper syncdb() method has finished!
 */
void MainWindow::afterPacmanHelperSyncDatabase()
{
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
          if (!UnixCommand::isAppRunning("spun", true)) sendNotification(notification);
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
          if (!UnixCommand::isAppRunning("spun", true)) sendNotification(notification);
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
        if (!UnixCommand::isAppRunning("spun", true)) sendNotification(notification);
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
        if (!UnixCommand::isAppRunning("spun", true)) sendNotification(notification);
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

  qDebug() << "At syncDatabase()...";
  toggleEnableInterface(false);
  m_icon = IconHelper::getIconOctopiTransparent();

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
    UnixCommand::execCommandAsNormalUser("kcp -u");

  m_pacmanHelperClient->syncdb();
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
  disconnect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  qDebug() << "At refreshAppIcon()...";
  m_outdatedStringList = Package::getOutdatedStringList();
  bool hasAURTool = UnixCommand::hasTheExecutable(StrConstants::getForeignRepositoryToolName());

  if (hasAURTool)
  {
    m_outdatedAURStringList = Package::getOutdatedAURStringList();
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
        m_systemTrayIcon->setToolTipSubTitle(StrConstants::getOneNewUpdate());
      #else
        m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate());
      #endif
    }
    else if (m_numberOfOutdatedAURPackages > 1)
    {
      #ifdef KSTATUS
        m_systemTrayIcon->setToolTipSubTitle(
              StrConstants::getNewUpdates(m_numberOfOutdatedAURPackages));
      #else
        m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates(m_numberOfOutdatedAURPackages));
      #endif
    }
  }

  if(m_outdatedStringList->count() > 0) //RED ICON!
  {
    if(m_commandExecuting == ectn_NONE)
    {
      m_actionSystemUpgrade->setEnabled(true);
      m_actionSystemUpgrade->setVisible(true);
    }

    qDebug() << "Got a RED icon!";
    m_icon = IconHelper::getIconOctopiRed();


#ifdef KSTATUS
    m_systemTrayIcon->setAttentionIconByPixmap(m_icon);
    m_systemTrayIcon->setStatus(KStatusNotifierItem::NeedsAttention);
#endif
  }
  else if(m_outdatedAURStringList->count() > 0) //YELLOW ICON!
  {
    m_actionSystemUpgrade->setVisible(false);
    m_icon = IconHelper::getIconOctopiYellow();
    qDebug() << "Got a YELLOW icon!";

#ifdef KSTATUS
    m_systemTrayIcon->setAttentionIconByPixmap(m_icon);
    m_systemTrayIcon->setStatus(KStatusNotifierItem::NeedsAttention);
#endif
  }
  else //YEAHHH... GREEN ICON!
  {
    m_actionSystemUpgrade->setVisible(false);
    m_icon = IconHelper::getIconOctopiGreen();
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
  qDebug() << "At exitNotifier()...";

  /*if (UnixCommand::isAppRunning("octopi", true))
  {    
    QProcess::startDetached("octopi -close");
    qDebug() << "Closing Octopi too...";
  }*/

  qApp->quit();
}

/*
 * Execs Octopi
 */
void MainWindow::runOctopi(ExecOpt execOptions)
{
  if (execOptions == ectn_SYSUPGRADE_NOCONFIRM_EXEC_OPT)
  {
    if (!WMHelper::isKDERunning() && (!WMHelper::isRazorQtRunning()) && (!WMHelper::isLXQTRunning()))
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
    if (!WMHelper::isKDERunning() && (!WMHelper::isRazorQtRunning()) && (!WMHelper::isLXQTRunning()))
    {
      QProcess::startDetached("octopi -sysupgrade -style gtk");
    }
    else
    {
      QProcess::startDetached("octopi -sysupgrade");
    }
  }
  else if (execOptions == ectn_NORMAL_EXEC_OPT)
  {
    if (!WMHelper::isKDERunning() && (!WMHelper::isRazorQtRunning()) && (!WMHelper::isLXQTRunning()))
    {
      QProcess::startDetached("octopi -style gtk");
    }
    else
    {
      QProcess::startDetached("octopi");
    }
  }
}
