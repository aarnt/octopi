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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../src/unixcommand.h"
#include "../src/pacmanexec.h"

#include <QDialog>
#include <QProcess>
#include <QString>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QRunnable>

class QIcon;
class QMenu;
class QAction;
class QFileSystemWatcher;
class OptionsDialog;
class TransactionDialog;
class QTcpServer;
class OutputDialog;

class ExecCommandAsNormalUserExtTask : public QRunnable
{
  void run() override
  {
    UnixCommand::execCommandAsNormalUserExt(ctn_PACMAN_SUP_COMMAND);
  }
};

enum CheckUpdate { ectn_AUTO_CHECK, ectn_USER_CHECK};

#ifdef KSTATUS
  class KStatusNotifierItem;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  virtual ~MainWindow();

  inline void turnDebugInfoOn() { m_debugInfo = true;}
  bool startServer();

public slots:
  void doCheckUpdates(CheckUpdate check = ectn_USER_CHECK);

private slots:
  void pacmanHelperTimerTimeout();
  void afterCheckUpdates(int exitCode, QProcess::ExitStatus);

  void execSystemTrayActivated(QSystemTrayIcon::ActivationReason);
  void execSystemTrayKF5();

  void refreshOutdatedPkgsTooltip();
  void refreshAppIcon();
  void runOctopi(ExecOpt execOptions = ectn_SYSUPGRADE_EXEC_OPT);
  void runOctopiCheckUpdates();
  void runOctopiSysUpgrade();
  void runOctopiAURUpgrade();

  inline void startOctopi() { runOctopi(ectn_NORMAL_EXEC_OPT); }
  void aboutOctopiNotifier();
  void hideOctopi();
  void showOctopi();
  void exitNotifier();
  void doSystemUpgrade();
  void doAURUpgrade();
  void doSystemUpgradeFinished(int exitCode);
  void toggleEnableInterface(bool state);
  void showOptionsDialog();
  void onSendInfoToOctopiHelper();
  void onCallRefreshAppIcon();

private:
  bool m_debugInfo;
  int m_numberOfCheckUpdatesPackages;
  int m_numberOfOutdatedPackages;
  int m_numberOfOutdatedAURPackages;
  bool m_systemUpgradeDialog;
  bool isOctopiBusy();
  bool canOctopiUpgrade();

  CommandExecuting m_commandExecuting;
  UnixCommand *m_unixCommand;
  PacmanExec *m_pacmanExec;
  TransactionDialog *m_transactionDialog;
  OptionsDialog *m_optionsDialog;
  QTcpServer *m_tcpServer;
  QAction *m_actionOctopi;
  QAction *m_actionOptions;
  QAction *m_actionCheckUpdates;
  QAction *m_actionSystemUpgrade;
  QAction *m_actionAURUpgrade;
  QAction *m_actionAbout;
  QAction *m_actionExit;
  QIcon m_icon;
  QHash<QString, QString> *m_checkUpdatesNameCurrentVersion;
  QHash<QString, QString> *m_checkUpdatesNameNewVersion;
  QStringList m_checkUpdatesStringList;
  QStringList *m_outdatedStringList;
  QStringList *m_outdatedAURStringList;
  QTimer *m_pacmanHelperTimer;
  QTimer *m_callRefreshAppIcon;
  QMenu *m_systemTrayIconMenu;
  QFileSystemWatcher *m_pacmanDatabaseSystemWatcher;
  OutputDialog *m_outputDialog;

#ifdef KSTATUS
  KStatusNotifierItem * m_systemTrayIcon;
#else
  QSystemTrayIcon *m_systemTrayIcon;
#endif

  bool _isSUAvailable();
  bool isInternetAvailable();
  void initActions();
  void initSystemTrayIcon();
  void setUpgradingTooltip();
};

#endif // MAINWINDOW_H
