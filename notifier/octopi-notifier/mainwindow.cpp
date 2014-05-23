#include "mainwindow.h"
#include "../../src/strconstants.h"
#include "../../src/uihelper.h"
#include "../../src/package.h"
#include "../../src/pacmanhelperclient.h"
#include "../../src/transactiondialog.h"

#include <QTimer>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QProcess>
#include <QMessageBox>

/*
 * This is Octopi Notifier slim interface code :-)
 */

/*
 * The obligatory constructor...
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
  m_pacmanDatabaseSystemWatcher =
            new QFileSystemWatcher(QStringList() << ctn_PACMAN_DATABASE_DIR, this);
  initSystemTrayIcon();
}

/*
 * Let's initialize the system tray object...
 */
void MainWindow::initSystemTrayIcon()
{
  m_commandExecuting = ectn_NONE;
  m_outdatedPackageList = new QStringList();
  m_systemTrayIcon = new QSystemTrayIcon(this);
  m_systemTrayIcon->setObjectName("systemTrayIcon");

  m_icon = IconHelper::getIconOctopiTransparent();
  m_systemTrayIcon->setIcon(m_icon); 
  setWindowIcon(m_icon);
  m_systemTrayIcon->show();
  m_systemTrayIcon->setToolTip(StrConstants::getSyncDatabases());
  qApp->processEvents();

  m_actionExit = new QAction(IconHelper::getIconExit(), tr("Exit"), this);
  connect(m_actionExit, SIGNAL(triggered()), this, SLOT(exitNotifier()));

  m_actionAbout = new QAction(StrConstants::getHelpAbout(), this);
  m_actionAbout->setIconVisibleInMenu(true);
  connect(m_actionAbout, SIGNAL(triggered()), this, SLOT(aboutOctopiNotifier()));

  m_actionOctopi = new QAction(this);
  m_actionOctopi->setText("Octopi...");
  connect(m_actionOctopi, SIGNAL(triggered()), this, SLOT(startOctopi()));

  m_actionSystemUpgrade = new QAction(this);
  m_actionSystemUpgrade->setIconVisibleInMenu(true);
  m_actionSystemUpgrade->setText(tr("System upgrade"));
  m_actionSystemUpgrade->setIcon(IconHelper::getIconSystemUpgrade());
  connect(m_actionSystemUpgrade, SIGNAL(triggered()), this, SLOT(runOctopiSysUpgrade()));

  m_systemTrayIconMenu = new QMenu( this );
  m_systemTrayIconMenu->addAction(m_actionOctopi);
  m_systemTrayIconMenu->addAction(m_actionSystemUpgrade);
  m_systemTrayIconMenu->addSeparator();
  m_systemTrayIconMenu->addAction(m_actionAbout);
  m_systemTrayIconMenu->addAction(m_actionExit);
  m_systemTrayIcon->setContextMenu(m_systemTrayIconMenu);

  connect ( m_systemTrayIcon , SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
            this, SLOT( execSystemTrayActivated ( QSystemTrayIcon::ActivationReason ) ) );

  m_pacmanHelperClient = new PacmanHelperClient("org.octopi.pacmanhelper", "/", QDBusConnection::systemBus(), 0);
  connect(m_pacmanHelperClient, SIGNAL(syncdbcompleted()), this, SLOT(afterPacmanHelperSyncDatabase()));

  m_pacmanHelperTimer = new QTimer();
  m_pacmanHelperTimer->setInterval(100);
  m_pacmanHelperTimer->start();

  connect(m_pacmanHelperTimer, SIGNAL(timeout()), this, SLOT(pacmanHelperTimerTimeout()));
}

/*
 * Execs Octopi
 */
void MainWindow::runOctopi(ExecOpt execOptions)
{
  if (execOptions == ectn_SYSUPGRADE_NOCONFIRM_EXEC_OPT)
  {
    if (!WMHelper::isKDERunning() && (!WMHelper::isRazorQtRunning()))
    {
      QProcess::startDetached("octopi -sysupgrade-noconfirm -style gtk");
    }
    else
    {
      QProcess::startDetached("octopi -sysupgrade-noconfirm");
    }
  }
  else if (execOptions == ectn_SYSUPGRADE_EXEC_OPT &&
      !UnixCommand::isAppRunning("octopi", true) && m_outdatedPackageList->count() > 0)
  {
    doSystemUpgrade();
  }
  else if (execOptions == ectn_SYSUPGRADE_EXEC_OPT &&
      UnixCommand::isAppRunning("octopi", true) && m_outdatedPackageList->count() > 0)
  {
    if (!WMHelper::isKDERunning() && (!WMHelper::isRazorQtRunning()))
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
    if (!WMHelper::isKDERunning() && (!WMHelper::isRazorQtRunning()))
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
 * Helper to a runOctopi with a call to SystemUpgrade
 */
void MainWindow::runOctopiSysUpgrade()
{
  runOctopi(ectn_SYSUPGRADE_EXEC_OPT);
}

/*
 * Shows Octopi Notifier About Dialog...
 */
void MainWindow::aboutOctopiNotifier()
{
  QString aboutText = "<b>Octopi Notifier - " + StrConstants::getApplicationVersion() + "</b><br>";
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
  if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
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
  if (targets->count() == 0 && m_outdatedPackageList->count() == 0)
  {
    return;
  }
  else if (targets->count() == 0 && m_outdatedPackageList->count() > 0)
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
  QString ds = QString::number(totalDownloadSize, 'f', 2);

  TransactionDialog question(this);

  //If we're in Chakra, there are no graphical system upgrades!
  if (UnixCommand::getLinuxDistro() == ectn_CHAKRA) question.removeYesButton();

  if(targets->count()==1)
    question.setText(StrConstants::getRetrieveTarget() +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds));
  else
    question.setText(StrConstants::getRetrieveTargets().arg(targets->count()) +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds));

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
      && m_outdatedPackageList->count() > 0)
  {
    m_commandExecuting = ectn_NONE;
    m_unixCommand->removeTemporaryActionFile();
    doSystemUpgrade();

    return;
  }

  m_commandExecuting = ectn_NONE;
  m_unixCommand->removeTemporaryActionFile();
  toggleEnableInterface(true);
}

/*
 * Enables and Disables some UI elements of Octopi-notifier
 */
void MainWindow::toggleEnableInterface(bool state)
{
  m_actionOctopi->setEnabled(state);
  m_actionExit->setEnabled(state);
}

/*
 * Whenever this timer ticks, we need to call the PacmanHelper DBus interface to sync Pacman's dbs
 */
void MainWindow::pacmanHelperTimerTimeout()
{
  static bool firstTime=true;

  if (m_commandExecuting != ectn_NONE) return;

  if (firstTime)
  {
    m_pacmanHelperTimer->setInterval(1000 * 60 * 60);
    firstTime=false;
  }

  m_actionOctopi->setEnabled(false);

  if (m_outdatedPackageList->count() > 0)
  {
    m_actionSystemUpgrade->setEnabled(false);
  }
  else
  {
    m_actionSystemUpgrade->setVisible(false);
  }

  m_icon = IconHelper::getIconOctopiTransparent();
  m_systemTrayIcon->setIcon(m_icon);
  m_systemTrayIcon->setToolTip(StrConstants::getSyncDatabases());
  qApp->processEvents();

  m_systemTrayIconMenu->close();
  m_systemTrayIcon->setContextMenu(0);
  m_commandExecuting = ectn_SYNC_DATABASE;
  m_pacmanHelperClient->syncdb();
}

/*
 * Called right after the PacmanHelper syncdb() method has finished!
 */
void MainWindow::afterPacmanHelperSyncDatabase()
{
  m_actionOctopi->setEnabled(true);
  m_systemTrayIcon->setContextMenu(m_systemTrayIconMenu);
  m_systemTrayIconMenu->close();
  m_commandExecuting = ectn_NONE;

  disconnect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

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
        m_systemTrayIcon->setToolTip(notification);
        if (!UnixCommand::isAppRunning("spun", true)) sendNotification(notification);
      }
      else if (m_numberOfOutdatedPackages > 1)
      {
        notification = StrConstants::getNewUpdates().arg(m_numberOfOutdatedPackages);
        m_systemTrayIcon->setToolTip(notification);
        if (!UnixCommand::isAppRunning("spun", true)) sendNotification(notification);
      }
    }
  }
  else
  {
    QString notification;

    if (numberOfOutdatedPackages == 1)
    {
      notification = StrConstants::getOneNewUpdate();
      m_systemTrayIcon->setToolTip(notification);
      if (!UnixCommand::isAppRunning("spun", true)) sendNotification(notification);
    }
    else if (numberOfOutdatedPackages > 1)
    {
      notification = StrConstants::getNewUpdates().arg(numberOfOutdatedPackages);
      m_systemTrayIcon->setToolTip(notification);
      if (!UnixCommand::isAppRunning("spun", true)) sendNotification(notification);
    }
  }

  connect(m_pacmanDatabaseSystemWatcher,
          SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));
}

/*
 * Uses notify-send to send a notification to the systray area
 */
void MainWindow::sendNotification(const QString &msg)
{
  QString processToExec("notify-send");
  if (UnixCommand::hasTheExecutable(processToExec))
  {
    processToExec += " -i /usr/share/icons/octopi_red.png -t 5000 \"" + StrConstants::getApplicationName() +
        "\"  \"" + msg + "\"";
    QProcess::startDetached(processToExec);
  }
}

/*
 * If we have some outdated packages, let's put an angry red face icon in this app!
 */
void MainWindow::refreshAppIcon()
{
  m_outdatedPackageList = Package::getOutdatedPackageList();

  bool hasYaourt = UnixCommand::hasTheExecutable(StrConstants::getForeignRepositoryToolName());
  if (hasYaourt)
  {
    m_outdatedYaourtPackageList = Package::getOutdatedYaourtPackageList();
  }
  else
  {
    m_outdatedYaourtPackageList = new QStringList();
  }

  m_numberOfOutdatedPackages = m_outdatedPackageList->count();
  m_numberOfOutdatedYaourtPackages = m_outdatedYaourtPackageList->count();

  if (m_numberOfOutdatedPackages == 0 && m_numberOfOutdatedYaourtPackages == 0)
  {
    m_systemTrayIcon->setToolTip("");
  }
  else if (m_numberOfOutdatedPackages > 0)
  {
    if (m_numberOfOutdatedPackages == 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate());
    }
    else if (m_numberOfOutdatedPackages > 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates().arg(m_numberOfOutdatedPackages));
    }
  }
  else if (m_numberOfOutdatedYaourtPackages > 0)
  {
    if (m_numberOfOutdatedYaourtPackages == 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate());
    }
    else if (m_numberOfOutdatedYaourtPackages > 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates().arg(m_numberOfOutdatedYaourtPackages));
    }
  }

  if(m_outdatedPackageList->count() > 0) //RED ICON!
  {
    if(m_commandExecuting == ectn_NONE)
    {
      m_actionSystemUpgrade->setEnabled(true);
      m_actionSystemUpgrade->setVisible(true);
    }

    m_icon = IconHelper::getIconOctopiRed();
  }
  else if(m_outdatedYaourtPackageList->count() > 0) //YELLOW ICON!
  {
    m_actionSystemUpgrade->setVisible(false);
    m_icon = IconHelper::getIconOctopiYellow();
  }
  else //YEAHHH... GREEN ICON!
  {
    m_actionSystemUpgrade->setVisible(false);
    m_icon = IconHelper::getIconOctopiGreen();
  }

  setWindowIcon(m_icon);
  m_systemTrayIcon->setIcon(m_icon);
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
    if (m_outdatedPackageList->count() > 0)
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
 * When the users quit this notifier...
 */
void MainWindow::exitNotifier()
{
  if (UnixCommand::isAppRunning("octopi", true))
  {    
    QProcess::startDetached("octopi -close");
  }

  qApp->quit();
}
