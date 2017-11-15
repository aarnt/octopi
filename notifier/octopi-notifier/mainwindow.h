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

#include "../../src/unixcommand.h"
#include "../../src/pacmanexec.h"

#include <QDialog>
#include <QProcess>
#include <QString>
#include <QMainWindow>
#include <QSystemTrayIcon>

class QIcon;
class QMenu;
class QAction;
class QFileSystemWatcher;
class PacmanHelperClient;
class OptionsDialog;
class TransactionDialog;

#ifdef KSTATUS
  class KStatusNotifierItem;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

  explicit MainWindow(QWidget *parent = 0);
  virtual ~MainWindow();
  inline void turnDebugInfoOn() { m_debugInfo = true;}

private slots:

  void pacmanHelperTimerTimeout();
  void afterPacmanHelperSyncDatabase();

  void execSystemTrayActivated(QSystemTrayIcon::ActivationReason);
  void execSystemTrayKF5();

  void syncDatabase();
  void refreshAppIcon();
  void runOctopi(ExecOpt execOptions = ectn_SYSUPGRADE_EXEC_OPT);
  void runOctopiSysUpgrade();  

  inline void startOctopi() { runOctopi(ectn_NORMAL_EXEC_OPT); }
  void aboutOctopiNotifier();
  void hideOctopi();
  void exitNotifier();
  void doSystemUpgrade();
  void doSystemUpgradeFinished();
  void toggleEnableInterface(bool state);
  void showOptionsDialog();

private:

  TransactionDialog *m_transactionDialog;
  OptionsDialog *m_optionsDialog;
  int m_numberOfOutdatedPackages;
  int m_numberOfOutdatedAURPackages;
  bool m_systemUpgradeDialog;
  CommandExecuting m_commandExecuting;
  UnixCommand *m_unixCommand;

  QAction *m_actionOctopi;
  QAction *m_actionOptions;
  QAction *m_actionSyncDatabase;
  QAction *m_actionSystemUpgrade;
  QAction *m_actionAbout;
  QAction *m_actionExit;

  QIcon m_icon;
  QStringList *m_outdatedStringList;
  QStringList *m_outdatedAURStringList;
  QTimer *m_pacmanHelperTimer;
  bool m_debugInfo;

#ifdef KSTATUS
  KStatusNotifierItem * m_systemTrayIcon;
#else
  QSystemTrayIcon *m_systemTrayIcon;
#endif

  QMenu *m_systemTrayIconMenu;
  QFileSystemWatcher *m_pacmanDatabaseSystemWatcher;
  PacmanHelperClient *m_pacmanHelperClient;

  bool _isSUAvailable();
  void initSystemTrayIcon();
  void sendNotification(const QString &msg);
};

#endif // MAINWINDOW_H
