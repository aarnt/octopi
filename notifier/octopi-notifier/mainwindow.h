#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

class QIcon;
class QMenu;
class QAction;
class QFileSystemWatcher;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    
signals:
    
private slots:

  void pacmanHelperTimerTimeout();
  void afterPacmanHelperSyncDatabase();
  void execSystemTrayActivated(QSystemTrayIcon::ActivationReason);
  void refreshAppIcon();
  void runOctopi();

private:

  QAction *m_actionExit;
  QAction *m_actionOctopi;
  QIcon m_icon;
  QStringList *m_outdatedPackageList;
  QTimer *m_pacmanHelperTimer;
  QSystemTrayIcon *m_systemTrayIcon;
  QMenu *m_systemTrayIconMenu;
  QFileSystemWatcher *m_pacmanDatabaseSystemWatcher;
  int m_numberOfOutdatedPackages;

  void initSystemTrayIcon();
};

#endif // MAINWINDOW_H
