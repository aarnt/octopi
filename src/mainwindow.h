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

#include <memory>
#include "pacmanexec.h"
#include "uihelper.h"

#include <QApplication>
#include <QItemSelection>
#include <QSystemTrayIcon>
#include <QMainWindow>
#include <QToolButton>
#include <QList>
#include <QUrl>

class AurVote;
class QTreeView;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;
class QElapsedTimer;
class QLabel;
class QComboBox;
class QListView;
class QProgressBar;
class QTextBrowser;
class QMenu;
class SearchLineEdit;
class QAction;
class QTreeWidgetItem;
class QTime;
class QDropEvent;
class QDragEnterEvent;
class TermWidget;
class QTcpServer;
class QSharedMemory;
class OptionsDialog;

#include "src/model/packagemodel.h"
#include "src/packagerepository.h"

//Tab indices for Properties' tabview
const int ctn_TABINDEX_INFORMATION(0);
const int ctn_TABINDEX_FILES(1);
const int ctn_TABINDEX_ACTIONS(2);
const int ctn_TABINDEX_OUTPUT(3);
const int ctn_TABINDEX_NEWS(4);
const int ctn_TABINDEX_HELPUSAGE(5);
const int ctn_TABINDEX_TERMINAL(6);

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

signals:
  void buildPackageListDone();
  void buildAURPackageListDone();
  void buildPackagesFromGroupListDone();

protected:
  void closeEvent(QCloseEvent *event);
  void keyPressEvent(QKeyEvent* ke);
  void keyReleaseEvent(QKeyEvent* ke);
  void dropEvent(QDropEvent *ev);
  void dragEnterEvent(QDragEnterEvent *ev);

private:
  Ui::MainWindow *ui;
  CPUIntensiveComputing *m_cic;
  PacmanExec *m_pacmanExec;
  UnixCommand *m_unixCommand;
  bool m_initializationCompleted;

  AurVote *m_aurVote;
  TermWidget *m_console;
  QTcpServer *m_tcpServer;

  SearchLineEdit *m_leFilterPackage;
  QList<QModelIndex> *m_foundFilesInPkgFileList;
  int m_indFoundFilesInPkgFileList;
  QFileSystemWatcher *m_pacmanDatabaseSystemWatcher;

  // Package Data
  PackageRepository m_packageRepo;
  // Package Model
  std::unique_ptr<PackageModel> m_packageModel;

  QSharedMemory *m_sharedMemory;

  //Controls if the dialog showing the packages to be upgraded is opened
  bool m_systemUpgradeDialog;

  //Searches /etc/pacman.conf to see if ILoveCandy is there
  bool m_iLoveCandy;

  //Controls the calling of System Upgrade action
  bool m_callSystemUpgrade;

  //Controls debugInfo output status
  bool m_debugInfo;

  //Controls the calling of System Upgrade NO CONFIRM action
  bool m_callSystemUpgradeNoConfirm;

  //Controls if this Linux box has an AUR tool installed
  bool m_hasAURTool;

  //Controls if this Linux box has slocate utility
  bool m_hasSLocate;

  //Controls if this Linux box has the mirror-check tool
  bool m_hasMirrorCheck;

  //Controls if the NewsTab must be showed
  bool m_gotoNewsTab;

  //Controls if some PackageLists must be refreshed while the main pkg list is being built
  bool m_refreshPackageLists;

  //Controls if foreign pkg list must be refreshed while the main pkg list is being built
  bool m_refreshForeignPackageList;

  //Controls if Group Widget needs to regain focus
  bool m_groupWidgetNeedsFocus;

  //Holds the remove command to be used: -Rcs/-R/-Rs or whichever the user has choosen
  QString m_removeCommand;

  //This model provides the list of pending actions of a transaction
  QStandardItemModel *m_modelTransaction;

  //This member holds the result list of AUR packages searched by the user
  QList<PackageListData> *m_listOfAURPackages;

  //This member holds the list of Pacman packages available
  std::unique_ptr<QList<PackageListData> > m_listOfPackages;

  //This member holds the list of Pacman packages from the selected group
  std::unique_ptr<QList<QString> > m_listOfPackagesFromGroup;

  //This member holds the target list retrieved by the pacman command which will be executed
  QStringList *m_targets;

  //This member holds the list of packages to install with "pacman -U" command
  QStringList m_packagesToInstallList;

  //This member holds the current command type being executed by Octopi
  CommandExecuting m_commandExecuting;
  CommandExecuting m_commandQueued;


  QStringList m_lastCommandList; //This member holds the last command string executed by Octopi
  QStringList *m_outdatedStringList;
  QStringList *m_checkupdatesStringList; //This is the outdated pkg list retrieved by checkupdates
  QHash<QString, QString> *m_checkUpdatesNameNewVersion;
  QStringList *m_outdatedAURStringList;

  QList<PackageListData> *m_foreignPackageList;
  QHash<QString, QString> *m_outdatedAURPackagesNameVersion;

  QLabel *m_lblSelCounter;    //Holds the number of selected packages
  QLabel *m_lblTotalCounters; //Holds the total number of packages
  QProgressBar *m_progressWidget;

  QToolButton *m_toolButtonPacman;
  QToolButton *m_toolButtonAUR;
  QMenu *m_menuToolButtonPacman;
  QMenu *m_menuToolButtonAUR;

  //This is a means for measuring the program's speed at some tasks
  QElapsedTimer *m_time;

  //This is the timer which controls the outdated AUR pkg list retrieval
  QTimer *m_outdatedAURTimer;

  QAction *m_dummyAction;
  QAction *m_actionLastSearchMethod;
  QAction *m_actionPackageInfo;
  QAction *m_actionInstallPacmanUpdates;
  QAction *m_actionInstallAURUpdates;
  QAction *m_actionShowGroups;
  QAction *m_actionMenuMirrorCheck;
  QAction *m_actionMenuRepository;
  QAction *m_actionMenuOptions;
  QAction *m_actionRepositoryAll;
  QAction *m_actionEditOctopiConf;
  QAction *m_actionCopyFullPath;
  QAction *m_actionSysInfo;
  QAction *m_actionStopTransaction;
  QAction *m_actionAURVote;
  QAction *m_actionAURUnvote;
  QToolButton *m_toolButtonStopTransaction;

  //Toggles use of AUR tool
  QAction *m_separatorForActionAUR;
  QAction *m_actionSwitchToAURTool;

  QByteArray m_horizontalSplit;
  QTreeWidgetItem *m_AllGroupsItem;

  int m_numberOfInstalledPackages;
  int m_numberOfOutdatedPackages;

  //Members that control the View menu settings
  ViewOptions m_selectedViewOption;
  QString m_selectedRepository;

  QString m_cachedPackageInInfo;  //Used in Info tab
  QString m_cachedPackageInFiles; //Used in Files tab

  QSet<QString> * m_unrequiredPackageList;
  QStringList m_listOfVisitedPackages;
  int m_indOfVisitedPackage;

  //Controls OptionsDialog exec
  OptionsDialog *m_optionsDialog;

  int selectTerminal(const int initialTerminalIndex);

  void loadSettings();
  void loadPanelSettings();
  void saveSettings(SaveSettingsReason saveSettingsReason);
  void savePackageColumnWidths();

  void initAppIcon();
  void refreshMenuTools();
  void refreshGroupsWidget();
  void refreshStatusBar();
  void refreshColumnSortSetup();

  void initMenuBar();
  void initPackageGroups();
  void initToolBar();
  void initStatusBar();
  void initTabFiles();
  void initActions();
  void initLineEditFilterPackages();
  void initPackageTreeView();
  void removePackageTreeViewConnections();
  void initTabWidgetPropertiesIndex();
  void initTabInfo();

  QString getSelectedGroup();
  bool isAllGroupsSelected();
  bool isAllGroups(const QString& group);
  bool hasPartialUpgrade(QStringList &pkgsToInstall);
  bool isPackageInstalled(const QString &pkgName);
  bool isPackageTreeViewVisible();

  void resizePackageView();
  void changeTabWidgetPropertiesIndex(const int newIndex);

  //Tab Files related methods
  void closeTabFilesSearchBar();
  void selectFirstItemOfPkgFileList();
  QString extractBaseFileName(const QString &fileName);
  QString getSelectedDirectory();

  void clearStatusBar();
  void showPackagesWithNoDescription();
  bool prepareSystemUpgrade();

  //Tab Transaction related methods
  bool areTherePendingActions();
  void tvTransactionAdjustItemText(QStandardItem *item);
  void tvTransactionRowsChanged(const QModelIndex& parent);
  QStandardItem * getRemoveTransactionParentItem();
  QStandardItem * getInstallTransactionParentItem();

  bool isPackageInInstallTransaction(const QString &pkgName);
  bool isPackageInRemoveTransaction(const QString &pkgName);
  void insertRemovePackageIntoTransaction(const QString &pkgName);
  void insertInstallPackageIntoTransaction(const QString &pkgName);
  void removePackagesFromRemoveTransaction();
  void removePackagesFromInstallTransaction();
  int getNumberOfTobeRemovedPackages();
  QString getTobeRemovedPackages();
  QString getTobeInstalledPackages();
  void initTabTransaction();

  //Tab Output related methods
  QTextBrowser *getOutputTextBrowser();
  void collapseItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex mi);
  void expandItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex* mi);
  void positionTextEditCursorAtEnd();
  bool textInTabOutput(const QString& findText);
  bool IsSyncingRepoInTabOutput();

  bool searchForKeyVerbs(const QString& msg);
  bool splitOutputStrings(const QString &output);
  void parsePacmanProcessOutput(const QString &pMsg);
  void ensureTabVisible(const int index);
  bool isPropertiesTabWidgetVisible();
  bool isSUAvailable();
  bool isInternetAvailable();
  void writeToTabOutput(const QString &msg, TreatURLLinks treatURLLinks = ectn_TREAT_URL_LINK);

  void initTabOutput();
  void clearTabOutput();

  QString retrieveDistroNews(bool searchForLatestNews = true);
  QString parseDistroNews();
  void showDistroNews(QString distroRSSXML, bool searchForLatestNews = true);
  void initTabNews();
  void initTabHelpUsage();
  void refreshToolBar();
  void refreshStatusBarToolButtons();

  void switchToViewAllPackages();
  void retrieveForeignPackageList();
  void retrieveUnrequiredPackageList();
  void retrieveOutdatedPackageList();

  bool isAURGroupSelected();
  bool isSearchByFileSelected();

  bool isNotifierBusy();

private slots:

  void initTabTerminal();
  void removeTabTerminal();
  void onTerminalChanged();
  void onExecCommandInTabTerminal(QString command);
  void onPressAnyKeyToContinue();
  void onCancelControlKey();
  void initToolButtonPacman();
  void initToolButtonAUR();
  void showToolButtonAUR();

  //TreeView methods
  void copyFullPathToClipboard();
  void collapseAllContentItems();
  void collapseThisContentItems();
  void expandAllContentItems();
  void expandThisContentItems();
  void openFile();
  void editFile();
  void editOctopiConf();
  void openTerminal();
  void openDirectory();
  void installLocalPackage();
  void showPackageInfo();
  void findFileInPackage();
  void incrementPercentage(int);
  void outputText(const QString&);
  void tvPackagesSearchColumnChanged(QAction*);
  void tvPackagesSelectionChanged(const QItemSelection&, const QItemSelection&);
  void tvTransactionSelectionChanged (const QItemSelection&, const QItemSelection&);
  void tvTransactionRowsInserted(const QModelIndex& parent, int, int);
  void tvTransactionRowsRemoved(const QModelIndex& parent, int, int);

  void buildPackagesFromGroupList(const QString &group);
  void buildPackageList();
  void horizontalSplitterMoved(int pos, int index);
  void metaBuildPackageList();
  void onPackageGroupChanged();

  void onAURVote();
  void onAURUnvote();
  void AURToolSelected();
  void groupItemSelected();

  void preBuildForeignPackageList();
  void preBuildUnrequiredPackageList();
  void preBuildPackageList();
  void postBuildPackageList();
  void refreshOutdatedAURStringList();
  void preBuildPackagesFromGroupList();
  void preBuildAURPackageList();
  void preBuildAURPackageListMeta();
  void buildAURPackageList();

  void headerViewPackageListSortIndicatorClicked(int col, Qt::SortOrder order);
  void changePackageListModel(ViewOptions viewOptions, QString selectedRepo);

  void execKeyActionOnPackage(CommandExecuting command);

  void execContextMenuPackages(QPoint point);
  void execContextMenuPkgFileList(QPoint point);
  void execContextMenuTransaction(QPoint point);
  void execSystemTrayActivated(QSystemTrayIcon::ActivationReason);

  //SearchLineEdit methods
  void reapplyPackageFilter();
  void lightPackageFilter();

  //TabWidget methods
  void refreshTabInfo(QString pkgName);
  void refreshTabInfo(bool clearContents=false, bool neverQuit=false);
  void refreshTabFiles(bool clearContents=false, bool neverQuit=false);
  void onDoubleClickPackageList();
  void refreshInfoAndFileTabs();
  void changedTabIndex();
  void invalidateTabs(); //This method clears the current information showed on tab.

  //Pacman transaction methods
  void doRemove();
  void doRemoveAndInstall();
  void doInstall();
  void doCheckUpdates();
  void doMirrorCheck();
  void doInstallAURPackage();
  void doPreDownloadTempYay();
  void doInstallYayPackage();
  void doRemoveAURPackage();
  void onAURToolChanged();
  void onAURVotingChanged();
  void disableTransactionActions();
  void enableTransactionActions();
  void toggleInstantSearch();
  void toggleTransactionActions(const bool value);
  void toggleSystemActions(const bool value);
  void commitTransaction();
  void cancelTransaction();
  bool stopTransaction();
  void onCanStopTransaction(bool yesNo);
  void pacmanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  //Tab transaction methods
  void changeTransactionActionsState();
  void clearTransactionTreeView();
  void insertIntoRemovePackage(QModelIndex *indexToInclude = nullptr);
  void insertIntoInstallPackage(QModelIndex *indexToInclude = nullptr);
  void insertIntoInstallPackageOptDeps(const QString &packageName);
  bool insertIntoRemovePackageDeps(const QStringList &dependencies);
  void insertGroupIntoRemovePackage();
  void insertGroupIntoInstallPackage();

  //View menu and submenu Repository actions...
  void selectedAllPackagesMenu();
  void selectedInstalledPackagesMenu();
  void selectedNonInstalledPackagesMenu();
  void selectedRepositoryMenu(QAction *actionRepoSelected);

  void hideGroupsWidget(bool pSaveSettings = true);
  void maximizePackagesTreeView();
  void maxDemaxPackagesTreeView(bool pSaveSettings = true);
  void maximizeTerminalTab();
  void maximizePropertiesTabWidget();
  void maxDemaxPropertiesTabWidget(bool pSaveSettings = true);
  void outputOutdatedPackageList();
  void outputAURVotedPackageList();
  void outputOutdatedAURPackageList();

  void onTabNewsSourceChanged(QUrl newSource);
  void refreshDistroNews(bool searchForLatestNews = true, bool gotoNewsTab = true);
  void postRefreshDistroNews();

  void onPacmanDatabaseChanged();
  void onHelpUsage();
  QString generateHelpUsageHtmlText();
  void refreshHelpUsageText();
  void onHelpDonate();
  void onHelpAbout();
  void onPressDelete();

  //SearchBar methods
  void positionInPkgListSearchByFile();
  void positionInFirstMatch();
  void searchBarTextChangedInTextBrowser(const QString &textToSearch);
  void searchBarFindNextInTextBrowser();
  void searchBarFindPreviousInTextBrowser();
  void searchBarClosedInTextBrowser();
  void searchBarTextChangedInTreeView(const QString &textToSearch);
  void searchBarFindNextInTreeView();
  void searchBarFindPreviousInTreeView();
  void searchBarClosedInTreeView();

  void showAnchorDescription(const QUrl & link);
  void positionInPackageList(const QString &pkgName);
  void outputTextBrowserAnchorClicked(const QUrl & link);
  void execToolTip();
  void launchPLV();
  void launchRepoEditor();
  void launchCacheCleaner();
  void doSysInfo();
  void refreshAppIcon();
  void onSendInfoToOctopiHelper();

public slots:
  bool startServer();
  void show();
  void doSystemUpgrade(SystemUpgradeOptions sysUpgradeOption = ectn_NO_OPT);
  void doAURUpgrade();
  void doInstallLocalPackages();
  void onOptions(OptionsDialogTab tabToOpen=ectn_TAB_GENERAL);

public:
  explicit MainWindow(QWidget *parent = nullptr);
  virtual ~MainWindow();

  static MainWindow* returnMainWindow()
  {
    static MainWindow *w=nullptr;
    if (w != nullptr) return w;
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
      if (widget->objectName() == QLatin1String("MainWindow"))
        w = dynamic_cast<MainWindow*>(widget);
    }
    return w;
  }

  const PackageRepository::PackageData* getFirstPackageFromRepo(const QString &pkgName);
  void turnDebugInfoOn();
  void setCallSystemUpgrade();
  void setCallSystemUpgradeNoConfirm();
  void setRemoveCommand(const QString &removeCommand);
  void setPackagesToInstallList(const QStringList &pkgList){ m_packagesToInstallList = pkgList; }
  bool isExecutingCommand(){ return m_commandExecuting != ectn_NONE; }
};

#endif // MAINWINDOW_H
