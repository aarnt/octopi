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

#include "constants.h"
#include "strconstants.h"
#include "unixcommand.h"

QString StrConstants::getApplicationName(){
  return QStringLiteral("Octopi");
}

QString StrConstants::getQtVersion(){
  return QLatin1String("Qt ") + QStringLiteral(QT_VERSION_STR);
}

QString StrConstants::getApplicationCliHelp(){
 QString str = getApplicationName() + QLatin1String(" - ") + ctn_APPLICATION_VERSION /*getApplicationVersion()*/ + QLatin1String("\n") +
     QLatin1String("\n") + getHelpUsage() + QLatin1String("\n\n") +
     QLatin1String("\t-version: ") + QObject::tr("show application version.") + QLatin1String("\n") +
     QLatin1String("\t-sysupgrade: ") + QObject::tr("force a system upgrade at startup.") + QLatin1String("\n") +
     QLatin1String("\t-sysupgrade-noconfirm: ") + QObject::tr("force a system upgrade at startup, withou asking for confirmation.") +
     QLatin1String("\n");

 return str;
}

QString StrConstants::getMissingPacmanDBFile()
{
  return QObject::tr("Pacman Database is missing!\nYou may need to synchronize database!");
}

QString StrConstants::getAll(){
  return QObject::tr("All");
}

QString StrConstants::getForeignRepositoryName(){
  static bool firstTime=true;
  static QString ret=QStringLiteral("AUR");

  if (firstTime)
  {
    if (UnixCommand::getLinuxDistro() == ectn_CHAKRA)
      ret=QStringLiteral("CCR");
    if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      ret=QStringLiteral("KCP");
    if (UnixCommand::getLinuxDistro() == ectn_PARABOLA)
      ret=QStringLiteral("Custom");

    firstTime=false;
  }

  return ret;
}

QString StrConstants::getForeignPkgRepositoryName(){
  static bool firstTime=true;
  static QString ret=QStringLiteral("aur");

  if (firstTime)
  {
    if (UnixCommand::getLinuxDistro() == ectn_CHAKRA)
      ret=QStringLiteral("ccr");
    else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      ret=QStringLiteral("kcp");
    else if (UnixCommand::getLinuxDistro() == ectn_PARABOLA)
      ret=QStringLiteral("custom");

    firstTime=false;
  }

  return ret;
}

QString StrConstants::getForeignRepositoryGroupName()
{
  static bool firstTime=true;
  static QString ret=QStringLiteral("AUR");

  if (firstTime)
  {
    if( UnixCommand::getLinuxDistro() == ectn_CHAKRA )
      ret=QStringLiteral("Ccr");
    else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      ret=QStringLiteral("KCP");
    else if (UnixCommand::getLinuxDistro() == ectn_PARABOLA)
      ret=QStringLiteral("Custom");

    firstTime=false;
  }

  return ret;
}

QString StrConstants::getForeignRepositoryTargetPrefix()
{
  static bool firstTime=true;
  static QString ret=QStringLiteral("aur/");

  if (firstTime)
  {
    if( UnixCommand::getLinuxDistro() == ectn_CHAKRA )
      ret=QStringLiteral("ccr/");
    else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      ret=QStringLiteral("kcp/");
    else if (UnixCommand::getLinuxDistro() == ectn_PARABOLA)
      ret=QStringLiteral("custom/");

    firstTime=false;
  }

  return ret;
}

QString StrConstants::getArchLinuxNews(){
  return QObject::tr("Arch Linux news");
}

QString StrConstants::getArtixLinuxNews(){
  return QObject::tr("Artix Linux news");
}

QString StrConstants::getChakraNews(){
  return QObject::tr("Chakra news");
}

QString StrConstants::getCondresOSNews(){
  return QObject::tr("Condres OS news");
}

QString StrConstants::getEndeavourOSNews(){
  return QObject::tr("EndeavourOS news");
}

QString StrConstants::getKaOSNews(){
  return QObject::tr("KaOS news");
}

QString StrConstants::getManjaroLinuxNews(){
  return QObject::tr("Manjaro Linux news");
}

QString StrConstants::getObarunLinuxNews(){
  return QObject::tr("Obarun Linux news");
}

QString StrConstants::getParabolaNews(){
  return QObject::tr("Parabola GNU/Linux-libre news");
}

QString StrConstants::getInformation(){
  return QObject::tr("Information");
}

QString StrConstants::getError(){
  return QObject::tr("Error");
}

QString StrConstants::getNewsErrorMessage(){
  return QObject::tr("No news could be found! Press Ctrl+G to download the latest news.");
}

QString StrConstants::getShowingCachedNews()
{
  return QObject::tr("Showing cached news.");
}

QString StrConstants::getIncompatibleLinuxDistroError(){
  return QObject::tr("This Linux distro seems to be incompatible with Octopi!");
}

QString StrConstants::getInternetUnavailableError(){
  return QObject::tr("Internet seems unavailable!");
}

QString StrConstants::getDisplayAllGroups(){
  return QObject::tr("Display all groups");
}

QString StrConstants::getForeignToolGroup(){
 QString tool = Package::getForeignRepositoryToolName();
  tool[0] = tool[0].toUpper();
  tool = QLatin1String("<") + tool + QLatin1String(">");

  return tool;
}

QString StrConstants::getHelpUsage(){
  return QObject::tr("Usage");
}

QString StrConstants::getHelpAbout(){
  return QObject::tr("About");
}

QString StrConstants::getHelp(){
  return QObject::tr("Help");
}

QString StrConstants::getOptions(){
  return QObject::tr("&Options");
}

QString StrConstants::getName(){
  return QObject::tr("Name");
}

QString StrConstants::getVersion(){
  return QObject::tr("Version");
}

QString StrConstants::getOutdated()
{
  return QObject::tr("Outdated");
}

QString StrConstants::getOutdatedVersion(){
  return QObject::tr("Outdated version");
}

QString StrConstants::getAvailableVersion(){
  return QObject::tr("Available version");
}

QString StrConstants::getNoDescriptionAvailabe(){
  return QObject::tr("No description available.");
}

QString StrConstants::getURL(){
  return QObject::tr("URL");
}

QString StrConstants::getRepository(){
  return QObject::tr("Repository");
}

QString StrConstants::getLicenses(){
  return QObject::tr("Licenses");
}

QString StrConstants::getMaintainer(){
  return QObject::tr("Maintainer");
}

QString StrConstants::getLastModified(){
  return QObject::tr("Last Modified");
}

QString StrConstants::getOutOfDate(){
  return QObject::tr("Out-of-date");
}

QString StrConstants::getGroup(){
  return QObject::tr("Group");
}

QString StrConstants::getGroups(){
  return QObject::tr("Groups");
}

QString StrConstants::getProvides(){
  return QObject::tr("Provides");
}

QString StrConstants::getDependsOn(){
  return QObject::tr("Depends On");
}

QString StrConstants::getMakeDeps(){
  return QObject::tr("Make Deps");
}

QString StrConstants::getRequiredBy(){
  return QObject::tr("Required By");
}

QString StrConstants::getOptionalFor(){
  return QObject::tr("Optional For");
}

QString StrConstants::getOptionalDeps(){
  return QObject::tr("Optional Deps");
}

QString StrConstants::getConflictsWith(){
  return QObject::tr("Conflicts With");
}

QString StrConstants::getReplaces(){
  return QObject::tr("Replaces");
}

QString StrConstants::getVotesHeader(){
  return QObject::tr("Votes");
}

QString StrConstants::getVotes(){
  return QObject::tr("votes");
}

QString StrConstants::getVote()
{
  return QObject::tr("Vote");
}

QString StrConstants::getAURVotedPackageList()
{
  return QObject::tr("AUR voted package list");
}

QString StrConstants::getUnvote()
{
  return QObject::tr("Unvote");
}

QString StrConstants::getOpenPKGBUILD()
{
  return QObject::tr("Open PKGBUILD");
}

QString StrConstants::getShowPKGBUILDDiff()
{
  return QObject::tr("Show PKGBUILD diff");
}

QString StrConstants::getOpenNewsInBrowser()
{
  return QObject::tr("Open news in a browser");
}

QString StrConstants::getThereIsOnlyOneVersionAvailable()
{
  return QObject::tr("There is only one version available of \"%1\"");
}

QString StrConstants::getDownloadSize(){
  return QObject::tr("Download Size");
}

QString StrConstants::getInstalledSize(){
  return QObject::tr("Installed Size");
}

QString StrConstants::getInstallReason(){
  return QObject::tr("Install Reason");
}

QString StrConstants::getExplicitly(){
  return QObject::tr("Explicitly");
}

QString StrConstants::getAsDependency(){
  return QObject::tr("As a dependency");
}

QString StrConstants::getPackager(){
  return QObject::tr("Packager");
}

QString StrConstants::getPackage(){
  return QObject::tr("Package");
}

QString StrConstants::getPackages(){
  return QObject::tr("Packages");
}

QString StrConstants::getArchitecture(){
  return QObject::tr("Architecture");
}

QString StrConstants::getBuildDate(){
  return QObject::tr("Build Date");
}

QString StrConstants::getInstallDate(){
  return QObject::tr("Install Date");
}

QString StrConstants::getDescription(){
  return QObject::tr("Description");
}

QString StrConstants::getAttention(){
  return QObject::tr("Attention");
}

QString StrConstants::getPartialUpdatesNotSupported(){
  return QObject::tr("Partial updates are not supported!");
}

QString StrConstants::getAutomaticSuCommand(){
  return QObject::tr("automatic");
}

QString StrConstants::getPassword(){
  return QObject::tr("Password");
}

QString StrConstants::getTabInfoName(){
  return QObject::tr("Info");
}

QString StrConstants::getTabFilesName(){
  return QObject::tr("Files");
}

QString StrConstants::getTabTransactionName(){
  return QObject::tr("Transaction");
}

QString StrConstants::getActions(){
  return QObject::tr("Actions");
}

QString StrConstants::getTabOutputName(){
  return QObject::tr("Output");
}

QString StrConstants::getTabNewsName(){
  return QObject::tr("News");
}

QString StrConstants::getTabTerminal()
{
  return QObject::tr("Terminal");
}

QString StrConstants::getContentsOf(){
  return QObject::tr("Contents of \"%1\"");
}

QString StrConstants::getFind(){
  return QObject::tr("Find");
}

QString StrConstants::getClear(){
  return QObject::tr("Clear");
}

QString StrConstants::getOutdatedInstalledVersion(){
  return QObject::tr(" (outdated installed version is %1)");
}

QString StrConstants::getNewerInstalledVersion(){
  return QObject::tr(" (newer installed version is %1)");
}

QString StrConstants::getBuildingPackageList(){
  return QObject::tr("Building package list...");
}

QString StrConstants::getSearchingForDistroNews(){
  return QObject::tr("Searching for %1 latest news...");
}

QString StrConstants::getOneOutdatedPackage(){
  return QObject::tr("There is one outdated package in your system:");
}

QString StrConstants::getOutdatedPackages(int outdatedPackagesCount){
  return QObject::tr("There are %n outdated packages in your system:", nullptr, outdatedPackagesCount);
}

QString StrConstants::getNewVersionAvailable(){
  return QObject::tr("(version %1 is available)");
}

QString StrConstants::getTotalPackages(int packageCount){
  return QObject::tr("%n packages", nullptr, packageCount);
}

QString StrConstants::getSelectedPackages(int selectedCount){
  return QObject::tr("%n selected", nullptr, selectedCount);
}

QString StrConstants::getNumberInstalledPackages(int installedPackagesCount){
  return QObject::tr("%n installed", nullptr, installedPackagesCount);
}

QString StrConstants::getNumberOutdatedPackages(int outdatedPackagesCount){
  return QObject::tr("%n outdated", nullptr, outdatedPackagesCount);
}

QString StrConstants::getNumberAvailablePackages(int availablePackagesCount){
  return QObject::tr("%n available", nullptr, availablePackagesCount);
}

QString StrConstants::getCleaningPackageCache(){
  return QObject::tr("Cleaning package cache...");
}

QString StrConstants::getRemovingPacmanTransactionLockFile(){
  return QObject::tr("Removing Pacman's transaction lock file...");
}

QString StrConstants::getSyncing(){
  return QStringLiteral("Syncing");
}

QString StrConstants::getPressAnyKey(){
  //return QStringLiteral("Press any key to continue...");
  return QStringLiteral("PAKtC");
}

QString StrConstants::getCouldNotAttachToParent()
{
  return QStringLiteral("octopi-helper[aborted]: Couldn't attach to parent");
}

QString StrConstants::getSuspiciousExecutionDetected()
{
  return QStringLiteral("octopi-helper[aborted]: Suspicious execution method");
}

QString StrConstants::getSuspiciousTransactionDetected()
{
  return QStringLiteral("octopi-helper[aborted]: Suspicious transaction detected ->");
}

QString StrConstants::getErrorPacmanProcessExecuting(){
  return QStringLiteral("octopi-helper[aborted]: There is an instance of pacman already running");
}

QString StrConstants::getCheckUpdates()
{
  return QObject::tr("Check updates");
}

QString StrConstants::getSystemUpgrade()
{
  return QObject::tr("System upgrade");
}

QString StrConstants::getCheckingForUpdates()
{
  return QObject::tr("Checking for updates...");
}

QString StrConstants::getNoUpdatesAvailable()
{
  return QObject::tr("No updates available");
}

QString StrConstants::getSyncMirror(){
  return QStringLiteral("Mirror-check...");
}

QString StrConstants::getSyncDatabase(){
  return QObject::tr("Sync database");
}

QString StrConstants::getSyncDatabases(){
  return QObject::tr("Synchronizing databases...");
}

QString StrConstants::getIsUpToDate(){
  return QStringLiteral("is up to date");
}

QString StrConstants::getSysInfoGenerated()
{
  return QObject::tr("SysInfo file generated on: %1<br>If you wish, post the output in your distro's forum for help.");
}

QString StrConstants::getSystemUpgradeMsg(){
  return QObject::tr("Starting full system upgrade...");
}

QString StrConstants::getChangingInstallReason(){
  return QObject::tr("Changing the install reason of selected packages...");
}

QString StrConstants::getInstallingPackages(){
  return QObject::tr("Installing selected packages...");
}

QString StrConstants::getRemovingPackages(){
  return QObject::tr("Removing selected packages...");
}

QString StrConstants::getRemovingAndInstallingPackages(){
  return QObject::tr("Removing/installing selected packages...");
}

QString StrConstants::getRunningCommandInTerminal(){
  return QObject::tr("Running command in terminal...");
}

QString StrConstants::getCommandFinishedOK(){
  return QObject::tr("Command finished OK!");
}

QString StrConstants::getCommandFinishedWithErrors(){
  return QObject::tr("Command finished with errors!");
}

QString StrConstants::getErrorCouldNotDownloadTempYay()
{
  return QObject::tr("Temporary \"yay-bin\" could not be downloaded!");
}

QString StrConstants::getTempYayDownloaded()
{
  return QObject::tr("Temporary \"yay-bin\" downloaded.");
}

QString StrConstants::getDownloadingTempYay()
{
  return QObject::tr("Downloading temporary \"yay-bin\"...");
}

QString StrConstants::geRetrievingPackage(){
  return QObject::tr("Retrieving %1");
}

QString StrConstants::getTotalDownloadSize(){
  return QObject::tr("Total download size: %1 KB");
}

QString StrConstants::getRetrievePackage(){
  return QObject::tr("The following package needs to be retrieved");
}

QString StrConstants::getRemovePackage(){
  return QObject::tr("The following package will be removed");
}

QString StrConstants::getRetrievePackages(int retrievePackagesCount){
  return QObject::tr("The following %n packages need to be retrieved", nullptr, retrievePackagesCount);
}

QString StrConstants::getRemovePackages(int removePackagesCount){
  return QObject::tr("The following %n packages will be removed", nullptr, removePackagesCount);
}

QString StrConstants::getWarnHoldPkgFound() {
  return QObject::tr("There are forbidden packages in the removal list!");
}

QString StrConstants::getNoNewUpdatesAvailable(){
  return QObject::tr("There are no new updates available!");
}

QString StrConstants::getOneNewUpdate(){
  return QObject::tr("There is an update available!");
}

QString StrConstants::getNewUpdates(int newUpdatesCount){
  return QObject::tr("There are %n updates available!", nullptr, newUpdatesCount);
}

QString StrConstants::getConfirmationQuestion(){
  return QObject::tr("Confirm?");
}

QString StrConstants::getWarning(){
  return QObject::tr("Warning!!!");
}

QString StrConstants::getConfirmation(){
  return QObject::tr("Confirmation");
}

QString StrConstants::getThereHasBeenATransactionError(){
  return QObject::tr("There has been a transaction error!");
}

QString StrConstants::getConfirmExecuteTransactionInTerminal(){
  return QObject::tr("Do you want to execute this transaction in a Terminal?");
}

QString StrConstants::getCancelActionsConfirmation(){
  return QObject::tr("Do you really want to cancel actions?");
}

QString StrConstants::getEnterAdministratorsPassword(){
  return QObject::tr(
        "Please, enter the administrator's password");
}

QString StrConstants::getErrorNoSuCommand(){
  return
    QObject::tr("There are no means to get administrator's credentials.");
}

QString StrConstants::getYoullNeedSuFrontend(){
  return QObject::tr("You'll need to install \"octopi-sudo\" in order to use Octopi.");
}

QString StrConstants::getYoullNeedToInstallAURTool()
{
  return QObject::tr("You'll need one of those tools to use AUR:\n\n"
                     "%1, %2, %3 %4 or %5").arg(QStringLiteral("pacaur"), QStringLiteral("pikaur"),
                                                                          QStringLiteral("trizen"),
                                                                          QStringLiteral("yaourt"),
                                                                          QStringLiteral("yay"));
}

QString StrConstants::getDoYouWantToInstallYayTool()
{
  return QObject::tr("You need a package in order to use AUR.\n"
                     "Do you want to install it?");
}

QString StrConstants::getAURConnectionIsOK()
{
  return QObject::tr("Connection is OK!");
}

QString StrConstants::getWouldYouLikeToHelpThisProject(){
  return QObject::tr("Would you like to help this project by voting for it?\n"
                     "You only need to accept and wait a few seconds.");
}

QString StrConstants::getThankYouForVoting()
{
  return QObject::tr("Thank you for voting for Octopi!");
}

QString StrConstants::getAURUserNameOrPasswordIsIncorrect()
{
  return QObject::tr("Username and/or password is/are incorrect.");
}

QString StrConstants::getErrorAURUserNameIsNotSet()
{
  return QObject::tr("AUR username is not set.");
}

QString StrConstants::getErrorAURPasswordIsNotSet()
{
  return QObject::tr("AUR password is not set.");
}

QString StrConstants::getSelectAURBuildDir()
{
  return QObject::tr("Select the build directory for the AUR packages");
}

QString StrConstants::getErrorIconPathInfoIsNotSet(){
  return QObject::tr("Icon path information is not set.");
}

QString StrConstants::getErrorRunOctopiAsUsrBin(){
  return QObject::tr("You must use \"/usr/bin/octopi\" to run Octopi");
}

QString StrConstants::getErrorRunOctopiNotifierAsUsrBin(){
  return QObject::tr("You must use \"/usr/bin/octopi-notifier\" to run Octopi Notifier");
}

QString StrConstants::getErrorRunOctopiCacheCleanerAsUsrBin(){
  return QObject::tr("You must use \"/usr/bin/octopi-cachecleaner\" to run Octopi Cache Cleaner");
}

QString StrConstants::getErrorRunningWithRoot(){
  return QObject::tr("You can not run Octopi with administrator's credentials.");
}

QString StrConstants::getThereIsARunningTransaction(){
  return QObject::tr("Canceling the running transaction may damage your system!");
}

QString StrConstants::getThereArePendingActions(){
  return QObject::tr("There are pending actions");
}

QString StrConstants::getDoYouReallyWantToQuit(){
  return QObject::tr("Do you really want to quit?");
}

QString StrConstants::getDoYouAgreeToGenerateSysInfo()
{
  return QObject::tr("You are about to generate \"octopi-sysinfo.log\" in your home path with the contents of:\n\n"
                     "\"/etc/lsb-release\", \"inxi -Fxz\", \"uname -a\", \"mhwd -li -d,\"\n"
                     "\"journalctl -b -p err\", \"cat /etc/pacman.conf\", \"pacman -Qm\"\n"
                     "and \"head --bytes=256K /var/log/pacman.log\"\n\n"
                     "Do you want to continue?\n");
}

QString StrConstants::getExecutingCommand(){
  return QObject::tr("Executing command");
}

QString StrConstants::getRunInTerminal(){
  return QObject::tr("Run in terminal");
}

QString StrConstants::getNeedsAppRestart(){
  return QObject::tr("This change needs application restart to take effect");
}

QString StrConstants::getWarnNeedsAppRestart(){
  return QObject::tr("These changes need application restart to take effect!");
}

QString StrConstants::getFileChooserTitle(){
  return QObject::tr("Select the packages you want to install");
}

QString StrConstants::getThisIsNotATextFile(){
  return QObject::tr("This file does not appear to be a simple text.\n"
                     "Are you sure you want to open it?");
}

QString StrConstants::getTransactionInstallText(){
  return QObject::tr("To be installed");
}

QString StrConstants::getTransactionRemoveText(){
  return QObject::tr("To be removed");
}

QString StrConstants::getRemove(){
  return QObject::tr("Remove");
}

QString StrConstants::getInstall(){
  return QObject::tr("Install");
}

QString StrConstants::getReinstall()
{
  return QObject::tr("Reinstall");
}

QString StrConstants::getChangeInstallReason()
{
  return QObject::tr("Change Install Reason");
}

QString StrConstants::getUpdate()
{
  return QObject::tr("Update");
}

QString StrConstants::getRemoveItem(){
  return QObject::tr("Remove item");
}

QString StrConstants::getRemoveItems(){
  return QObject::tr("Remove items");
}

QString StrConstants::getPressCtrlAToSelectAll(){
  return QObject::tr("Press Ctrl+A to select/deselect all");
}

QString StrConstants::getUseForeignTool(){
  return QObject::tr("Use \"%1\" tool").arg(Package::getForeignRepositoryToolName());
}

QString StrConstants::getCopyFullPath(){
  return QObject::tr("Copy path to clipboard");
}

QString StrConstants::getNotifierSetupDialogTitle(){
  return QObject::tr("Setup");
}

QString StrConstants::getNotiferSetupDialogGroupBoxTitle(){
  return QObject::tr("Check for package updates interval");
}

QString StrConstants::getOnceADay(){
  return QObject::tr("Once a day");
}

QString StrConstants::getOnceADayAt(){
  return QObject::tr("Once a day, at");
}

QString StrConstants::getOnceADayAtDesc(){
  return QObject::tr("(value in 24-hour format: 0 to 23)");
}

QString StrConstants::getOnceEvery(){
  return QObject::tr("Once every");
}

QString StrConstants::getOnceEveryDesc(){
  return QObject::tr("(value in minutes: %1 to %2)"); //5 to 44640
}

QString StrConstants::getSetInterval(){
  return QObject::tr("Set interval...");
}

QString StrConstants::getStop()
{
  return QObject::tr("Stop");
}

QString StrConstants::getExecutableCouldNotBeFound()
{
  return QObject::tr("%1 could not be found in the system!");
}

QString StrConstants::getZoomIn()
{
  return QObject::tr("Zoom in");
}

QString StrConstants::getZoomOut()
{
  return QObject::tr("Zoom out");
}

QString StrConstants::getMaximize()
{
  return QObject::tr("Maximize");
}

QString StrConstants::getCopy()
{
  return QObject::tr("Copy");
}

QString StrConstants::getPaste()
{
  return QObject::tr("Paste");
}

QString StrConstants::getNever()
{
  return QObject::tr("Never");
}

QString StrConstants::getUpgrading()
{
  return QObject::tr("Upgrading...");
}

QString StrConstants::getExit()
{
  return QObject::tr("Exit");
}

//Style Sheets ---------------------------------

QString StrConstants::getToolBarCSS(){
  return QString(QStringLiteral("QToolBar { border: 5px; } "
                 "QToolTip {}"));
}

QString StrConstants::getFilterPackageNotFoundCSS(){
  return QString(QStringLiteral("QLineEdit{ color: white; "
                 "background-color: rgb(207, 135, 142);"
                 "border-color: rgb(206, 204, 197);}")
                 );
}

QString StrConstants::getFilterPackageFoundCSS(){
  return QString(QStringLiteral("QLineEdit, SearchLineEdit{ color: black; "
                 "background-color: rgb(255, 255, 200);"
                 "border-color: rgb(206, 204, 197);}")
                 );
}

QString StrConstants::getDockWidgetTitleCSS(){
  return QString(QStringLiteral("QDockWidget::title { "
                 "text-align: right;"
                 "background: transparent;"
                 "padding-right: 5px;}")
                 );
}

QString StrConstants::getTabBarCSS(){
  return QString(QStringLiteral("QTabBar::close-button {"
                 "image: url(:/resources/images/window-close.png);"
                 "border-radius: 4px}"
                 "QTabBar::close-button:hover {"
                 "background-color: palette(light)}"
                 "QTabBar::close-button:pressed {"
                 "background-color: palette(mid)}")
                 );
}

QString StrConstants::getTreeViewCSS(){
  return QString(QStringLiteral("QTreeView::branch:has-siblings:!adjoins-item {"
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
                 "       image: url(:/resources/styles/branch-open_BW.png);}"));
}
