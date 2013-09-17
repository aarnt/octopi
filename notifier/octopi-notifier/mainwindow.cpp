#include "mainwindow.h"
#include "../../src/strconstants.h"
#include "../../src/uihelper.h"
#include "../../src/package.h"
#include "../../src/pacmanhelperclient.h"

#include <QTimer>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QProcess>

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
  m_systemTrayIcon = new QSystemTrayIcon(this);
  m_systemTrayIcon->setObjectName("systemTrayIcon");

  m_actionExit = new QAction(IconHelper::getIconExit(), tr("Exit"), this);
  connect(m_actionExit, SIGNAL(triggered()), this, SLOT(exitNotifier()));
  m_actionOctopi = new QAction(this);
  m_actionOctopi->setText("Octopi...");
  connect(m_actionOctopi, SIGNAL(triggered()), this, SLOT(runOctopi()));

  refreshAppIcon();

  if (m_numberOfOutdatedPackages == 0 && m_numberOfOutdatedYaourtPackages == 0)
  {
    m_systemTrayIcon->setToolTip("");
  }
  else if (m_numberOfOutdatedPackages > 0)
  {
    QString notification;

    if (m_numberOfOutdatedPackages == 1)
    {
      notification = StrConstants::getOneNewUpdate();
      m_systemTrayIcon->setToolTip(notification);
    }
    else if (m_numberOfOutdatedPackages > 1)
    {
      notification = StrConstants::getNewUpdates().arg(m_numberOfOutdatedPackages);
      m_systemTrayIcon->setToolTip(notification);
    }
  }
  else if (m_numberOfOutdatedYaourtPackages > 0)
  {
    m_systemTrayIcon->setToolTip("");

    if (m_numberOfOutdatedYaourtPackages == 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate());
    }
    else if (m_numberOfOutdatedYaourtPackages > 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates().arg(m_numberOfOutdatedYaourtPackages));
    }
  }

  m_systemTrayIcon->show();
  m_systemTrayIconMenu = new QMenu( this );
  m_systemTrayIconMenu->addAction(m_actionOctopi);
  m_systemTrayIconMenu->addSeparator();
  m_systemTrayIconMenu->addAction(m_actionExit);
  m_systemTrayIcon->setContextMenu(m_systemTrayIconMenu);

  connect ( m_systemTrayIcon , SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
            this, SLOT( execSystemTrayActivated ( QSystemTrayIcon::ActivationReason ) ) );

  m_pacmanClient = new PacmanHelperClient("org.octopi.pacmanhelper", "/", QDBusConnection::systemBus(), 0);
  connect(m_pacmanClient, SIGNAL(syncdbcompleted()), this, SLOT(afterPacmanHelperSyncDatabase()));

  m_pacmanHelperTimer = new QTimer();
  m_pacmanHelperTimer->setInterval(1000 * 5);
  m_pacmanHelperTimer->start();

  connect(m_pacmanHelperTimer, SIGNAL(timeout()), this, SLOT(pacmanHelperTimerTimeout()));

  //connect(m_pacmanDatabaseSystemWatcher,
  //        SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));
}

/*
 * Exec Octopi
 */
void MainWindow::runOctopi()
{
  QProcess proc;

  if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX &&
     (!WMHelper::isKDERunning() && (!WMHelper::isRazorQtRunning())))
  {
    if (m_icon.pixmap(QSize(22,22)).toImage() ==
        IconHelper::getIconOctopiRed().pixmap(QSize(22,22)).toImage())
    {
      proc.startDetached("octopi -sysupgrade -style gtk");
    }
    else
    {
      proc.startDetached("octopi -style gtk");
    }
  }
  else
  {
    if (m_icon.pixmap(QSize(22,22)).toImage() ==
        IconHelper::getIconOctopiRed().pixmap(QSize(22,22)).toImage())
    {
      proc.startDetached("octopi -sysupgrade");
    }
    else
    {
      proc.startDetached("octopi");
    }
  }
}

void MainWindow::hideOctopi()
{
  QProcess proc;
  proc.startDetached("octopi -hide");
}

/*
 * Whenever this timer ticks, we need to call the PacmanHelper DBus interface to sync Pacman's dbs
 */
void MainWindow::pacmanHelperTimerTimeout()
{
  static bool firstTime=true;

  if (firstTime)
  {
    m_pacmanHelperTimer->setInterval(1000 * 60 * 60);
    firstTime=false;
  }

  m_pacmanClient->syncdb();
}

/*
 * Called right after the PacmanHelper syncdb() method has finished!
 */
void MainWindow::afterPacmanHelperSyncDatabase()
{
  m_systemTrayIconMenu->close();

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
        sendNotification(notification);
      }
      else if (m_numberOfOutdatedPackages > 1)
      {
        notification = StrConstants::getNewUpdates().arg(m_numberOfOutdatedPackages);
        m_systemTrayIcon->setToolTip(notification);
        sendNotification(notification);
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
      sendNotification(notification);
    }
    else if (numberOfOutdatedPackages > 1)
    {
      notification = StrConstants::getNewUpdates().arg(numberOfOutdatedPackages);
      m_systemTrayIcon->setToolTip(notification);
      sendNotification(notification);
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
  if (WMHelper::isXFCERunning() || WMHelper::isLXDERunning() || WMHelper::isOPENBOXRunning())
  {
    QString processToExec("notify-send");
    if (UnixCommand::hasTheExecutable(processToExec))
    {
      QProcess *send = new QProcess();
      processToExec += " -i /usr/share/icons/octopi_red.png -t 30000 \"" + StrConstants::getApplicationName() +
          "\"  \"" + msg + "\"";
      send->startDetached(processToExec);
    }
  }
}

/*
 * If we have some outdated packages, let's put an angry red face icon in this app!
 */
void MainWindow::refreshAppIcon()
{
  //disconnect(m_pacmanDatabaseSystemWatcher,
  //        SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));

  m_outdatedPackageList = Package::getOutdatedPackageList();
  m_outdatedYaourtPackageList = Package::getOutdatedYaourtPackageList();
  m_numberOfOutdatedPackages = m_outdatedPackageList->count();
  m_numberOfOutdatedYaourtPackages = m_outdatedYaourtPackageList->count();

  if (m_numberOfOutdatedPackages == 0 && m_numberOfOutdatedYaourtPackages == 0)
  {
    //m_systemTrayIcon->setToolTip(StrConstants::getApplicationName());
    m_actionOctopi->setText("Octopi...");
    m_systemTrayIcon->setToolTip("");
  }
  else if (m_numberOfOutdatedPackages > 0)
  {
    m_actionOctopi->setText(tr("System upgrade"));

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
    m_actionOctopi->setText("Octopi...");

    if (m_numberOfOutdatedYaourtPackages == 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate());
    }
    else if (m_numberOfOutdatedYaourtPackages > 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates().arg(m_numberOfOutdatedYaourtPackages));
    }
  }

  if(m_outdatedPackageList->count() > 0)
  {
    m_icon = (IconHelper::getIconOctopiRed());
  }
  else if(m_outdatedYaourtPackageList->count() > 0)
  {
    m_icon = (IconHelper::getIconOctopiYellow());
  }
  else
  {
    m_icon = (IconHelper::getIconOctopiGreen());
  }

  m_systemTrayIcon->setIcon(m_icon);
  m_systemTrayIcon->show();

  //connect(m_pacmanDatabaseSystemWatcher,
  //        SIGNAL(directoryChanged(QString)), this, SLOT(refreshAppIcon()));
}

/*
 * Whenever the user clicks on the systemTray icon...
 */
void MainWindow::execSystemTrayActivated(QSystemTrayIcon::ActivationReason ar)
{
  switch (ar)
  {
  case QSystemTrayIcon::DoubleClick:
  {
    runOctopi();
    break;
  }
  case QSystemTrayIcon::Trigger:
  {
    if (UnixCommand::isAppRunning("octopi", true)) hideOctopi();
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
    QProcess proc;
    proc.startDetached("octopi -close");
  }

  close();
}
