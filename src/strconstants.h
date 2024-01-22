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

#ifndef STRCONSTANTS_H
#define STRCONSTANTS_H

#include <QString>

class StrConstants{
public:
  static QString getApplicationName();
  static QString getQtVersion();
  static QString getApplicationCliHelp();
  static QString getMissingPacmanDBFile();
  static QString getAll();
  static QString getForeignRepositoryName();
  static QString getForeignPkgRepositoryName();
  static QString getForeignRepositoryGroupName();
  static QString getForeignRepositoryTargetPrefix();
  static QString getArchLinuxNews();
  static QString getArtixLinuxNews();
  static QString getChakraNews();
  static QString getCondresOSNews();
  static QString getEndeavourOSNews();
  static QString getKaOSNews();
  static QString getManjaroLinuxNews();
  static QString getObarunLinuxNews();
  static QString getParabolaNews();
  static QString getInformation();
  static QString getError();
  static QString getNewsErrorMessage();
  static QString getShowingCachedNews();
  static QString getIncompatibleLinuxDistroError();
  static QString getInternetUnavailableError();
  static QString getDisplayAllGroups();
  static QString getForeignToolGroup();
  static QString getHelpUsage();
  static QString getHelpAbout();
  static QString getHelp();
  static QString getOptions();
  static QString getName();
  static QString getVersion();
  static QString getOutdated();
  static QString getOutdatedVersion();
  static QString getAvailableVersion();
  static QString getNoDescriptionAvailabe();
  static QString getURL();
  static QString getRepository();
  static QString getLicenses();
  static QString getMaintainer();
  static QString getLastModified();
  static QString getOutOfDate();
  static QString getGroup();
  static QString getGroups();
  static QString getProvides();
  static QString getDependsOn();
  static QString getMakeDeps();
  static QString getRequiredBy();
  static QString getOptionalFor();
  static QString getOptionalDeps();
  static QString getConflictsWith();
  static QString getReplaces();
  static QString getVotesHeader();
  static QString getVotes();
  static QString getVote();
  static QString getAURVotedPackageList();
  static QString getUnvote();
  static QString getOpenPKGBUILD();  
  static QString getShowPKGBUILDDiff();
  static QString getOpenNewsInBrowser();
  static QString getThereIsOnlyOneVersionAvailable();
  static QString getDownloadSize();
  static QString getInstalledSize();
  static QString getInstallReason();
  static QString getExplicitly();
  static QString getAsDependency();
  static QString getPackager();
  static QString getPackage();
  static QString getPackages();
  static QString getArchitecture();
  static QString getBuildDate();
  static QString getInstallDate();
  static QString getDescription();
  static QString getAttention();
  static QString getPartialUpdatesNotSupported();
  static QString getAutomaticSuCommand();
  static QString getPassword();
  static QString getTabInfoName();
  static QString getTabFilesName();
  static QString getTabTransactionName();
  static QString getActions();
  static QString getTabOutputName();
  static QString getTabNewsName();
  static QString getTabTerminal();
  static QString getContentsOf();
  static QString getFind();
  static QString getClear();
  static QString getOutdatedInstalledVersion();
  static QString getNewerInstalledVersion();
  static QString getBuildingPackageList();
  static QString getSearchingForDistroNews();
  static QString getOneOutdatedPackage();
  static QString getOutdatedPackages(int outdatedPackagesCount);
  static QString getNewVersionAvailable();
  static QString getTotalPackages(int packageCount);
  static QString getSelectedPackages(int selectedCount);
  static QString getNumberInstalledPackages(int installedPackagesCount);
  static QString getNumberOutdatedPackages(int outdatedPackagesCount);
  static QString getNumberAvailablePackages(int availablePackagesCount);
  static QString getCleaningPackageCache();
  static QString getRemovingPacmanTransactionLockFile();
  static QString getSyncing();
  static QString getPressAnyKey();
  static QString getCouldNotAttachToParent();
  static QString getSuspiciousExecutionDetected();
  static QString getSuspiciousTransactionDetected();
  static QString getCheckUpdates();
  static QString getSystemUpgrade();
  static QString getCheckingForUpdates();
  static QString getNoUpdatesAvailable();
  static QString getSyncMirror();
  static QString getSyncDatabase();
  static QString getSyncDatabases();
  static QString getIsUpToDate();
  static QString getSysInfoGenerated();
  static QString getSystemUpgradeMsg();
  static QString getChangingInstallReason();
  static QString getInstallingPackages();
  static QString getRemovingPackages();
  static QString getRemovingAndInstallingPackages();
  static QString getChooseATerminal();
  static QString getRunningCommandInTerminal();
  static QString getCommandFinishedOK();
  static QString getCommandFinishedWithErrors();  
  static QString getTempYayDownloaded();
  static QString getDownloadingTempYay();
  static QString getErrorCouldNotDownloadTempYay();
  static QString getErrorPacmanProcessExecuting();
  static QString geRetrievingPackage();
  static QString getTotalDownloadSize();
  static QString getRetrievePackage();
  static QString getRemovePackage();
  static QString getRetrievePackages(int retrievePackagesCount);
  static QString getRemovePackages(int removePackagesCount);
  static QString getWarnHoldPkgFound();
  static QString getNoNewUpdatesAvailable();
  static QString getOneNewUpdate();
  static QString getNewUpdates(int newUpdatesCount);
  static QString getConfirmationQuestion();
  static QString getWarning();
  static QString getConfirmation();
  static QString getThereHasBeenATransactionError();
  static QString getConfirmExecuteTransactionInTerminal();
  static QString getCleanCacheConfirmation();
  static QString getRemovePacmanTransactionLockFileConfirmation();
  static QString getCancelActionsConfirmation();
  static QString getEnterAdministratorsPassword();
  static QString getErrorNoSuCommand();
  static QString getErrorDbLock();
  static QString getYoullNeedSuFrontend();
  static QString getYoullNeedToInstallAURTool();
  static QString getDoYouWantToInstallYayTool();
  static QString getAURConnectionIsOK();
  static QString getWouldYouLikeToHelpThisProject();
  static QString getThankYouForVoting();
  static QString getAURUserNameOrPasswordIsIncorrect();
  static QString getErrorAURUserNameIsNotSet();
  static QString getErrorAURPasswordIsNotSet();
  static QString getSelectAURBuildDir();
  static QString getErrorIconPathInfoIsNotSet();
  static QString getErrorRunOctopiAsUsrBin();
  static QString getErrorRunOctopiNotifierAsUsrBin();
  static QString getErrorRunOctopiCacheCleanerAsUsrBin();
  static QString getErrorRunningWithRoot();
  static QString getThereIsARunningTransaction();
  static QString getThereArePendingActions();
  static QString getDoYouReallyWantToQuit();
  static QString getDoYouAgreeToGenerateSysInfo();
  static QString getExecutingCommand();
  static QString getRunInTerminal();
  static QString getNeedsAppRestart();
  static QString getWarnNeedsAppRestart();
  static QString getFileChooserTitle();
  static QString getThisIsNotATextFile();
  static QString getTransactionInstallText();
  static QString getTransactionRemoveText();
  static QString getRemove();
  static QString getInstall();
  static QString getReinstall();
  static QString getChangeInstallReason();
  static QString getUpdate();
  static QString getRemoveItem();
  static QString getRemoveItems();
  static QString getPressCtrlAToSelectAll();
  static QString getUseForeignTool();
  static QString getCopyFullPath();
  static QString getNotifierSetupDialogTitle();
  static QString getNotiferSetupDialogGroupBoxTitle();
  static QString getOnceADay();
  static QString getOnceADayAt();
  static QString getOnceADayAtDesc();
  static QString getOnceEvery();
  static QString getOnceEveryDesc();
  static QString getSetInterval();
  static QString getStop();
  static QString getExecutableCouldNotBeFound();
  static QString getZoomIn();
  static QString getZoomOut();
  static QString getMaximize();
  static QString getCopy();
  static QString getPaste();
  static QString getNever();
  static QString getUpgrading();
  static QString getExit();

  //Style Sheets ---------------------------------
  static QString getToolBarCSS();
  static QString getFilterPackageNotFoundCSS();
  static QString getFilterPackageFoundCSS();
  static QString getDockWidgetTitleCSS();
  static QString getTabBarCSS();
  static QString getTreeViewCSS();
};

#endif // STRCONSTANTS_H
