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

#include "unixcommand.h"

class StrConstants{
public:

  static QString getApplicationName(){
    return "Octopi";
  }

  static QString getApplicationVersion(){
    return "0.7.0-dev";
  }

  static QString getQtVersion(){
    return "Qt " + QString(QT_VERSION_STR);
  }

  static QString getApplicationCliHelp(){
    QString str =
        "\n" + QObject::tr("Octopi usage help:") + "\n\n" +
        "-version: " + QObject::tr("show application version.") + "\n" +
        "-style <Qt4-style>: " + QObject::tr("use a different Qt4 style (ex: -style gtk).") + "\n" +
        "-removecmd <Remove-command>: " + QObject::tr("use a different remove command (ex: -removecmd R).") + "\n" +
        "-sysupgrade: " + QObject::tr("force a system upgrade at startup.") + "\n";

    return str;
  }

  static QString getAll(){
    return QObject::tr("All");
  }

  static QString getForeignRepositoryName(){
    if (UnixCommand::getLinuxDistro() == ectn_CHAKRA)
      return QLatin1String( "CCR" );

    if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      return QLatin1String( "KCP" );

    return QLatin1String( "AUR" );
  }

  static QString getForeignPkgRepositoryName(){
    if (UnixCommand::getLinuxDistro() == ectn_CHAKRA)
      return QLatin1String( "ccr" );
    else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      return QLatin1String( "kcp" );

    return QLatin1String( "aur" );
  }

  static QString getForeignRepositoryToolName()
  {
    static bool first=true;
    static QString ret;

    if (first)
    {
      if( UnixCommand::getLinuxDistro() == ectn_CHAKRA )
        ret = QLatin1String( "ccr" );
      else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
        ret = QLatin1String( "kcp" );
      /*else if (UnixCommand::hasTheExecutable("pacaur"))
        ret = QLatin1String( "pacaur" );*/
      else
        ret = QLatin1String( "yaourt" );

      first = false;
    }

    return ret;
  }

  static QString getForeignRepositoryGroupName()
  {
    if( UnixCommand::getLinuxDistro() == ectn_CHAKRA )
      return QLatin1String( "Ccr" );
    else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      return QLatin1String( "KCP" );

    return QLatin1String( "AUR" );
  }

  static QString getForeignRepositoryTargetPrefix()
  {
    if( UnixCommand::getLinuxDistro() == ectn_CHAKRA )
      return QLatin1String( "ccr/" );
    else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      return "kcp/";

    return QLatin1String( "aur/" );
  }

  static QString getAntergosNews(){
    return QObject::tr("Antergos news");
  }

  static QString getArchBSDNews(){
    return QObject::tr("ArchBSD news");
  }

  static QString getArchLinuxNews(){
    return QObject::tr("Arch Linux news");
  }

  static QString getChakraNews(){
    return QObject::tr("Chakra news");
  }

  static QString getKaOSNews(){
    return QObject::tr("KaOS news");
  }

  static QString getManjaroLinuxNews(){
    return QObject::tr("Manjaro Linux news");
  }

  static QString getNetrunnerNews(){
    return QObject::tr("Netrunner Rolling news");
  }

  static QString getNewsErrorMessage(){
    return QObject::tr("No news could be found! Press Ctrl+G to download the latest news.");
  }

  static QString getIncompatibleLinuxDistroError(){
    return QObject::tr("This Linux distro seems to be incompatible with Octopi!");
  }

  static QString getInternetUnavailableError(){
    return QObject::tr("Internet seems unavailable!");
  }

  static QString getDisplayAllGroups(){
    return QObject::tr("Display all groups");
  }

  static QString getForeignToolGroup(){
    QString tool = getForeignRepositoryToolName();
    tool[0] = tool[0].toUpper();
    tool = "<" + tool + ">";
    return tool.toLatin1();
  }

  static QString getHelpUsage(){
    return QObject::tr("Usage");
  }

  static QString getHelpAbout(){
    return QObject::tr("About");
  }

  static QString getName(){
    return QObject::tr("Name");
  }

  static QString getVersion(){
    return QObject::tr("Version");
  }

  static QString getOutdatedVersion(){
    return QObject::tr("Outdated version");
  }

  static QString getAvailableVersion(){
    return QObject::tr("Available version");
  }

  static QString getNoDescriptionAvailabe(){
    return QObject::tr("No description available.");
  }

  static QString getURL(){
    return QObject::tr("URL");
  }

  static QString getRepository(){
    return QObject::tr("Repository");
  }

  static QString getLicenses(){
    return QObject::tr("Licenses");
  }

  static QString getGroup(){
    return QObject::tr("Group");
  }

  static QString getGroups(){
    return QObject::tr("Groups");
  }

  static QString getProvides(){
    return QObject::tr("Provides");
  }

  static QString getDependsOn(){
    return QObject::tr("Depends On");
  }

  static QString getRequiredBy(){
    return QObject::tr("Required By");
  }

  static QString getOptionalFor(){
    return QObject::tr("Optional For");
  }

  static QString getOptionalDeps(){
    return QObject::tr("Optional Deps");
  }

  static QString getConflictsWith(){
    return QObject::tr("Conflicts With");
  }

  static QString getReplaces(){
    return QObject::tr("Replaces");
  }

  static QString getPopularityHeader(){
    return QObject::tr("Popularity");
  }

  static QString getVotes(){
    return QObject::tr("votes");
  }

  static QString getDownloadSize(){
    return QObject::tr("Download Size");
  }

  static QString getInstalledSize(){
    return QObject::tr("Installed Size");
  }

  static QString getPackager(){
    return QObject::tr("Packager");
  }

  static QString getPackage(){
    return QObject::tr("Package");
  }

  static QString getPackages(){
    return QObject::tr("Packages");
  }

  static QString getArchitecture(){
    return QObject::tr("Architecture");
  }

  static QString getBuildDate(){
    return QObject::tr("Build Date");
  }

  static QString getDescription(){
    return QObject::tr("Description");
  }

  static QString getAttention(){
    return QObject::tr("Attention");
  }

  static QString getAutomaticSuCommand(){
    return QObject::tr("automatic");
  }

  static QString getPassword(){
    return QObject::tr("Password");
  }

  static QString getTabInfoName(){
    return QObject::tr("Info");
  }

  static QString getTabFilesName(){
    return QObject::tr("Files");
  }

  static QString getTabTransactionName(){
    return QObject::tr("Transaction");
  }

  static QString getTabOutputName(){
    return QObject::tr("Output");
  }

  static QString getTabNewsName(){
    return QObject::tr("News");
  }

  static QString getContentsOf(){
    return QObject::tr("Contents of \"%1\"");
  }

  static QString getFind(){
    return QObject::tr("Find");
  }

  static QString getClear(){
    return QObject::tr("Clear");
  }

  static QString getOutdatedInstalledVersion(){
    return QObject::tr(" (outdated installed version is %1)");
  }

  static QString getNewerInstalledVersion(){
    return QObject::tr(" (newer installed version is %1)");
  }

  static QString getBuildingPackageList(){
    return QObject::tr("Building package list...");
  }

  static QString getSearchingForDistroNews(){
    return QObject::tr("Searching for %1 latest news...");
  }

  static QString getOneOutdatedPackage(){
    return QObject::tr("There is one outdated package in your system:");
  }

  static QString getOutdatedPackages(int outdatedPackagesCount){
    return QObject::tr("There are %n outdated packages in your system:", 0, outdatedPackagesCount);
  }

  static QString getNewVersionAvailable(){
    return QObject::tr("(version %1 is available)");
  }

  static QString getSelectedPackages(int packageCount){
    return QObject::tr("%n (%2) packages", 0, packageCount);
  }

  static QString getNumberInstalledPackages(int installedPackagesCount){
    return QObject::tr("%n installed", 0, installedPackagesCount);
  }

  static QString getNumberOutdatedPackages(int outdatedPackagesCount){
    return QObject::tr("%n outdated", 0, outdatedPackagesCount);
  }

  static QString getNumberAvailablePackages(int availablePackagesCount){
    return QObject::tr("%n available", 0, availablePackagesCount);
  }

  static QString getCleaningPackageCache(){
    return QObject::tr("Cleaning package cache...");
  }

  static QString getRemovingPacmanTransactionLockFile(){
    return QObject::tr("Removing Pacman's transaction lock file...");
  }

  static QString getSyncing(){
    return QObject::tr("Syncing");
  }

  static QString getPressAnyKey(){
    return QObject::tr("Press any key to continue...");
  }

  static QString getSyncMirror(){
    return "Mirror-check...";
  }

  static QString getSyncDatabase(){
    return QObject::tr("Sync database");
  }

  static QString getSyncDatabases(){
    return QObject::tr("Synchronizing databases...");
  }

  static QString getIsUpToDate(){
    return QObject::tr("is up to date");
  }

  static QString getSystemUpgrade(){
    return QObject::tr("Starting full system upgrade...");
  }

  static QString getInstallingPackages(){
    return QObject::tr("Installing selected packages...");
  }

  static QString getRemovingPackages(){
    return QObject::tr("Removing selected packages...");
  }

  static QString getRemovingAndInstallingPackages(){
    return QObject::tr("Removing/installing selected packages...");
  }

  static QString getChooseATerminal(){
    return QObject::tr("Choose a terminal");
  }

  static QString getRunningCommandInTerminal(){
    return QObject::tr("Running command in terminal...");
  }

  static QString getCommandFinishedOK(){
    return QObject::tr("Command finished OK!");
  }

  static QString getCommandFinishedWithErrors(){
    return QObject::tr("Command finished with errors!");
  }

  static QString geRetrievingPackage(){
    return QObject::tr("Retrieving %1");
  }

  static QString getTotalDownloadSize(){
    return QObject::tr("Total download size: %1 KB");
  }

  static QString getRetrievePackage(){
    return QObject::tr("The following package needs to be retrieved");
  }

  static QString getRemovePackage(){
    return QObject::tr("The following package will be removed");
  }

  static QString getRetrievePackages(int retrievePackagesCount){
    return QObject::tr("The following %n packages need to be retrieved", 0, retrievePackagesCount);
  }

  static QString getRemovePackages(int removePackagesCount){
    return QObject::tr("The following %n packages will be removed", 0, removePackagesCount);
  }

  static QString getWarnHoldPkgFound() {
    return QObject::tr("There are forbidden packages in the removal list!");
  }

  static QString getNoNewUpdatesAvailable(){
    return QObject::tr("There are no new updates available!");
  }

  static QString getOneNewUpdate(){
    return QObject::tr("There is an update available!");
  }

  static QString getNewUpdates(int newUpdatesCount){
    return QObject::tr("There are %n updates available!", 0, newUpdatesCount);
  }

  static QString getConfirmationQuestion(){
    return QObject::tr("Confirm?");
  }

  static QString getWarning(){
    return QObject::tr("Warning!!!");
  }

  static QString getConfirmation(){
    return QObject::tr("Confirmation");
  }

  static QString getThereHasBeenATransactionError(){
    return QObject::tr("There has been a transaction error!");
  }

  static QString getConfirmExecuteTransactionInTerminal(){
    return QObject::tr("Do you want to execute this transaction in a Terminal?");
  }

  static QString getCleanCacheConfirmation(){
    return QObject::tr("Do you really want to clean the package cache?");
  }

  static QString getRemovePacmanTransactionLockFileConfirmation(){
    return QObject::tr("Do you really want to remove Pacman's transaction lock file?");
  }

  static QString getCancelTransactionConfirmation(){
    return QObject::tr("Do you really want to cancel the transaction?");
  }

  static QString getEnterAdministratorsPassword(){
    return QObject::tr(
          "Please, enter the administrator's password");
  }

  static QString getErrorNoSuCommand(){
    return
      QObject::tr("There are no means to get administrator's credentials.");
  }

  static QString getYoullNeedSuFrontend(){
    return QObject::tr("You'll need to install a su frontend like gksu or kdesu.");
  }

  static QString getErrorRunningWithRoot(){
    return QObject::tr("You can not run Octopi with administrator's credentials.");
  }

  static QString getThereIsAPendingTransaction(){
    return QObject::tr("There is a pending transaction");
  }

  static QString getDoYouReallyWantToQuit(){
    return QObject::tr("Do you really want to quit?");
  }

  static QString getExecutingCommand(){
    return QObject::tr("Executing command");    
  }

  static QString getRunInTerminal(){
    return QObject::tr("Run in terminal");
  }

  static QString getNeedsAppRestart(){
    return QObject::tr("Needs application restart to take effect");
  }

  static QString getWarnNeedsAppRestart(){
    return QObject::tr("These changes need application restart to take effect!");
  }

  static QString getFileChooserTitle(){
    return QObject::tr("Select the packages you want to install");
  }

  static QString getThisIsNotATextFile(){
    return QObject::tr("This file does not appear to be a simple text.\n"
                       "Are you sure you want to open it?");
  }

  static QString getTransactionInstallText(){
    return QObject::tr("To be installed");
  }

  static QString getTransactionRemoveText(){
    return QObject::tr("To be removed");
  }

  static QString getRemove(){
    return QObject::tr("remove");
  }

  static QString getInstall(){
    return QObject::tr("install");
  }

  static QString getRemoveItem(){
    return QObject::tr("Remove item");
  }

  static QString getRemoveItems(){
    return QObject::tr("Remove items");
  }

  static QString getPressCtrlAToSelectAll(){
    return QObject::tr("Press Ctrl+A to select/deselect all");
  }

  static QString getUseAURTool(){
    return QObject::tr("Use \"%1\" tool").arg(getForeignRepositoryToolName());
  }

  static QString getCopyFullPath(){
    return QObject::tr("Copy path to clipboard");
  }

  static QString getNotifierSetupDialogTitle(){
    return QObject::tr("Setup");
  }

  static QString getNotiferSetupDialogGroupBoxTitle(){
    return QObject::tr("Sync database interval");
  }

  static QString getOnceADay(){
    return QObject::tr("Once a day");
  }

  static QString getOnceADayAt(){
    return QObject::tr("Once a day, at");
  }

  static QString getOnceADayAtDesc(){
    return QObject::tr("(value in 24-hour format: 0 to 23)");
  }

  static QString getOnceEvery(){
    return QObject::tr("Once every");
  }

  static QString getOnceEveryDesc(){
    return QObject::tr("(value in minutes: 10 to 1380)");
  }

  static QString getSetInterval(){
    return QObject::tr("Set interval...");
  }

  //Style Sheets ---------------------------------

  static QString getToolBarCSS(){
    return QString("QToolBar { border: 5px; } "
                   "QToolTip {}");
  }

  static QString getFilterPackageNotFoundCSS(){
    return QString("QLineEdit{ color: white; "
                   "background-color: rgb(207, 135, 142);"
                   "border-color: rgb(206, 204, 197);}"
                   );
  }

  static QString getFilterPackageFoundCSS(){
    return QString("QLineEdit, SearchLineEdit{ color: black; "
                   "background-color: rgb(255, 255, 200);"
                   "border-color: rgb(206, 204, 197);}"
                   );
  }

  static QString getDockWidgetTitleCSS(){
    return QString("QDockWidget::title { "
                   "text-align: right;"
                   "background: transparent;"
                   "padding-right: 5px;}"
                   );
  }

  static QString getTabBarCSS(){
    return QString("QTabBar::close-button {"
                   "image: url(:/resources/images/window-close.png);"
                   "border-radius: 4px}"
                   "QTabBar::close-button:hover {"
                   "background-color: palette(light)}"
                   "QTabBar::close-button:pressed {"
                   "background-color: palette(mid)}"
                   );
  }

  static QString getTreeViewCSS(){
    return QString("QTreeView::branch:has-siblings:!adjoins-item {"
                   "   border-image: url(:/resources/styles/vline.png) 0;}"
                   "QTreeView::branch:has-siblings:adjoins-item {"
                   "    border-image: url(:/resources/styles/branch-more.png) 0;}"
                   "QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
                   "   border-image: url(:/resources/styles/branch-end.png) 0;}"
                   "QTreeView::branch:has-children:!has-siblings:closed,"
                   "QTreeView::branch:closed:has-children:has-siblings {"
                   "       border-image: none;"
                   "        image: url(:/resources/styles/branch-closed_BW.png);}"
                   "QTreeView::branch:open:has-children:!has-siblings,"
                   "QTreeView::branch:open:has-children:has-siblings  {"
                   "       border-image: none;"
                   "       image: url(:/resources/styles/branch-open_BW.png);}");
  }
};

#endif // STRCONSTANTS_H
