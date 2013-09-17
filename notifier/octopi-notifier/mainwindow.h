#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../../src/unixcommand.h"

#include <QProcess>
#include <QString>
#include <QMainWindow>
#include <QSystemTrayIcon>

class QIcon;
class QMenu;
class QAction;
class QFileSystemWatcher;
class PacmanHelperClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

  explicit MainWindow(QWidget *parent = 0);
    
private slots:

  void pacmanHelperTimerTimeout();
  void afterPacmanHelperSyncDatabase();
  void execSystemTrayActivated(QSystemTrayIcon::ActivationReason);
  void refreshAppIcon();
  void runOctopi(bool execApplication = false);
  void hideOctopi();
  void exitNotifier();
  void doSystemUpgrade();
  void doSystemUpgradeFinished(int, QProcess::ExitStatus);
  void toggleEnableInterface(bool state);

private:

  int m_numberOfOutdatedPackages;
  int m_numberOfOutdatedYaourtPackages;
  bool m_systemUpgradeDialog;
  CommandExecuting m_commandExecuting;
  UnixCommand *m_unixCommand;

  QAction *m_actionExit;
  QAction *m_actionOctopi;
  QIcon m_icon;
  QStringList *m_outdatedPackageList;
  QStringList *m_outdatedYaourtPackageList;
  QTimer *m_pacmanHelperTimer;
  QSystemTrayIcon *m_systemTrayIcon;
  QMenu *m_systemTrayIconMenu;
  QFileSystemWatcher *m_pacmanDatabaseSystemWatcher;
  PacmanHelperClient *m_pacmanClient;

  bool _isSUAvailable();
  void initSystemTrayIcon();
  void sendNotification(const QString &msg);
};

#endif // MAINWINDOW_H
