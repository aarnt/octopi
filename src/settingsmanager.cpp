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

/*
 * This SettingsManager class holds the getters and setters needed to deal with the database
 * in which all Octopi configuration is saved and retrieved (~/.config/octopi/)
 */

#include "settingsmanager.h"
#include "unixcommand.h"
#include "uihelper.h"
#include "qaesencryption.h"

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QSettings>
#include <QDir>
#include <QCryptographicHash>

//Initialization of Singleton pointer...
SettingsManager* SettingsManager::m_pinstance = 0;

SettingsManager::SettingsManager(){
  m_SYSsettings = new QSettings(QSettings::UserScope, ctn_ORGANIZATION, ctn_APPLICATION);
}

SettingsManager::~SettingsManager(){  
  delete m_SYSsettings;
}

// Class singleton
SettingsManager* SettingsManager::instance(){
  if (m_pinstance == 0)
  {
    m_pinstance = new SettingsManager();
  }

  return m_pinstance;
}

//CacheCleaner related --------------------------------------------------------------
int SettingsManager::getKeepNumInstalledPackages() {
  return instance()->getSYSsettings()->value(ctn_KEY_KEEP_NUM_INSTALLED, 0).toInt();
}

int SettingsManager::getKeepNumUninstalledPackages() {
  return instance()->getSYSsettings()->value(ctn_KEY_KEEP_NUM_UNINSTALLED, 0).toInt();
}

void SettingsManager::setCacheCleanerWindowSize(QByteArray newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_CACHE_CLEANER_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setKeepNumInstalledPackages(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_KEEP_NUM_INSTALLED, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setKeepNumUninstalledPackages(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_KEEP_NUM_UNINSTALLED, newValue);
  instance()->getSYSsettings()->sync();
}

QByteArray SettingsManager::getRepoEditorWindowSize()
{
  return (instance()->getSYSsettings()->value(ctn_KEY_REPO_EDITOR_WINDOW_SIZE, 0).toByteArray());
}

void SettingsManager::setRepoEditorWindowSize(QByteArray newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_REPO_EDITOR_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

//CacheCleaner related --------------------------------------------------------------


//Notifier related ------------------------------------------------------------------

int SettingsManager::getCheckUpdatesHour()
{
  SettingsManager p_instance;
  int h = p_instance.getSYSsettings()->value(ctn_KEY_CHECKUPDATES_HOUR, -1).toInt();

  if (h != -1)
  {
    if (h < 0)
    {
      h = 0;
      p_instance.getSYSsettings()->setValue(ctn_KEY_CHECKUPDATES_HOUR, h);
      p_instance.getSYSsettings()->sync();
    }
    else if (h > 23)
    {
      h = 23;
      p_instance.getSYSsettings()->setValue(ctn_KEY_CHECKUPDATES_HOUR, h);
      p_instance.getSYSsettings()->sync();
    }
  }

  return h;
}

//The syncDb interval is in MINUTES and it cannot be less than 5!
int SettingsManager::getCheckUpdatesInterval()
{
  const int ctn_MAX_MIN = 44640;
  SettingsManager p_instance;
  int n = p_instance.getSYSsettings()->value(ctn_KEY_CHECKUPDATES_INTERVAL, -1).toInt();

  if ((n != -1 && n != -2) && (n < 5))
  {
    n = 5;
    p_instance.getSYSsettings()->setValue(ctn_KEY_CHECKUPDATES_INTERVAL, n);
    p_instance.getSYSsettings()->sync();
  }
  else if (n > ctn_MAX_MIN)
  {
    n = ctn_MAX_MIN; //This is 1 month (31 days), the maximum allowed!
    p_instance.getSYSsettings()->setValue(ctn_KEY_CHECKUPDATES_INTERVAL, n);
    p_instance.getSYSsettings()->sync();
  }

  return n;
}

QDateTime SettingsManager::getLastCheckUpdatesTime()
{
  if (!instance()->getSYSsettings()->contains(ctn_KEY_LAST_CHECKUPDATES_TIME))
  {
    return QDateTime();
  }
  else
  {
    SettingsManager p_instance;
    return (p_instance.getSYSsettings()->value(ctn_KEY_LAST_CHECKUPDATES_TIME, 0)).toDateTime();
  }
}

void SettingsManager::setCheckUpdatesHour(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_CHECKUPDATES_HOUR, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setCheckUpdatesInterval(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_CHECKUPDATES_INTERVAL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setLastCheckUpdatesTime(QDateTime newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_LAST_CHECKUPDATES_TIME, newValue);
  instance()->getSYSsettings()->sync();
}
//Notifier related ------------------------------------------------------------------


//Octopi related --------------------------------------------------------------------
int SettingsManager::getCurrentTabIndex(){
  return instance()->getSYSsettings()->value(
        ctn_KEY_CURRENT_TAB_INDEX, 0).toInt();
}

int SettingsManager::getPanelOrganizing(){
  return instance()->getSYSsettings()->value(ctn_KEY_PANEL_ORGANIZING, ectn_NORMAL ).toInt();
}

int SettingsManager::getPackageListOrderedCol(){
  return instance()->getSYSsettings()->value(ctn_KEY_PACKAGE_LIST_ORDERED_COL, 1 ).toInt();
}

int SettingsManager::getPackageListSortOrder(){
  return instance()->getSYSsettings()->value(
        ctn_KEY_PACKAGE_LIST_SORT_ORDER, Qt::AscendingOrder ).toInt();
}

int SettingsManager::getAURPackageListOrderedCol()
{
  return instance()->getSYSsettings()->value(ctn_KEY_AUR_PACKAGE_LIST_ORDERED_COL, 1 ).toInt();
}

int SettingsManager::getAURPackageListSortOrder()
{
  return instance()->getSYSsettings()->value(
        ctn_KEY_AUR_PACKAGE_LIST_SORT_ORDER, Qt::AscendingOrder ).toInt();
}

int SettingsManager::getPackageIconColumnWidth()
{
  return instance()->getSYSsettings()->value(
        ctn_KEY_PACKAGE_ICON_COLUMN_WIDTH, 24).toInt();
}

int SettingsManager::getPackageNameColumnWidth()
{
  return instance()->getSYSsettings()->value(
        ctn_KEY_PACKAGE_NAME_COLUMN_WIDTH, 400).toInt();
}

int SettingsManager::getPackageVersionColumnWidth()
{
  return instance()->getSYSsettings()->value(
        ctn_KEY_PACKAGE_VERSION_COLUMN_WIDTH, 260).toInt();
}

int SettingsManager::getPackageRepositoryColumnWidth()
{
  return instance()->getSYSsettings()->value(
        ctn_KEY_PACKAGE_REPOSITORY_COLUMN_WIDTH, 150).toInt();
}

bool SettingsManager::getUseDefaultAppIcon()
{
  if (!instance()->getSYSsettings()->contains(ctn_KEY_USE_DEFAULT_APP_ICON)){
    instance()->getSYSsettings()->setValue(ctn_KEY_USE_DEFAULT_APP_ICON, true);
    return true;
  }
  else
  {
    SettingsManager p_instance;
    return (p_instance.getSYSsettings()->value(ctn_KEY_USE_DEFAULT_APP_ICON, true)).toBool();
  }
}

QString SettingsManager::getOctopiBusyIconPath()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_OCTOPI_BUSY_ICON_PATH, QLatin1String(""))).toString();
}

QString SettingsManager::getOctopiRedIconPath()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_OCTOPI_RED_ICON_PATH, QLatin1String(""))).toString();
}

QString SettingsManager::getOctopiYellowIconPath()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_OCTOPI_YELLOW_ICON_PATH, QLatin1String(""))).toString();
}

QString SettingsManager::getOctopiGreenIconPath()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_OCTOPI_GREEN_ICON_PATH, QLatin1String(""))).toString();
}

bool SettingsManager::isDistroRSSUrlEmpty()
{
  SettingsManager p_instance;
  return p_instance.getSYSsettings()->value(ctn_KEY_DISTRO_RSS_URL, QLatin1String("")).toString().isEmpty();
}

QString SettingsManager::getDistroRSSUrl()
{
  SettingsManager p_instance;
  LinuxDistro distro = UnixCommand::getLinuxDistro();

  if (distro == ectn_ARCHLINUX || distro == ectn_ARCHBANGLINUX /*|| distro == ectn_SWAGARCH*/)
    return (p_instance.getSYSsettings()->value(ctn_KEY_DISTRO_RSS_URL, QStringLiteral("https://www.archlinux.org/feeds/news/"))).toString();
  else if (distro == ectn_CHAKRA)
    return (p_instance.getSYSsettings()->value(ctn_KEY_DISTRO_RSS_URL, QStringLiteral("https://community.chakralinux.org/c/news.rss"))).toString();
  else if (distro == ectn_CONDRESOS)
    return (p_instance.getSYSsettings()->value(ctn_KEY_DISTRO_RSS_URL, QStringLiteral("https://condresos.codelinsoft.it/index.php/blog?format=feed&amp;type=rss"))).toString();
  else if (distro == ectn_ENDEAVOUROS)
    return (p_instance.getSYSsettings()->value(ctn_KEY_DISTRO_RSS_URL, QStringLiteral("https://endeavouros.com/feed/"))).toString();
  else if (distro == ectn_KAOS)
    return (p_instance.getSYSsettings()->value(ctn_KEY_DISTRO_RSS_URL, QStringLiteral("https://kaosx.us/feed.xml"))).toString();
  else if (distro == ectn_MANJAROLINUX)
    return (p_instance.getSYSsettings()->value(ctn_KEY_DISTRO_RSS_URL, QStringLiteral("https://forum.manjaro.org/c/announcements.rss"))).toString();
  /*else if (distro == ectn_NETRUNNER)
    return (p_instance.getSYSsettings()->value(ctn_KEY_DISTRO_RSS_URL, "https://www.netrunner.com/feed/")).toString();*/
  else if (distro == ectn_PARABOLA)
    return (p_instance.getSYSsettings()->value(ctn_KEY_DISTRO_RSS_URL, QStringLiteral("https://www.parabola.nu/feeds/news/"))).toString();
  else return QLatin1String("");
}

bool SettingsManager::getShowPackageNumbersOutput()
{
  SettingsManager p_instance;
  return p_instance.getSYSsettings()->value( ctn_KEY_SHOW_PACKAGE_NUMBERS_OUTPUT, 1).toBool();
}

bool SettingsManager::getShowStopTransaction()
{
  SettingsManager p_instance;
  return p_instance.getSYSsettings()->value(ctn_KEY_SHOW_STOP_TRANSACTION, 1).toBool();
}

QString SettingsManager::getAURTool()
{
  QString params;

  SettingsManager p_instance;
  QString ret = (p_instance.getSYSsettings()->value(ctn_KEY_AUR_TOOL, QLatin1String(""))).toString();

  if (ret == ctn_NO_AUR_TOOL) return ret;
  else if (ret == ctn_PACAUR_TOOL)
  {
    if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");
    if (getAURNoEditParam()) params += QLatin1String(" --noedit ");
    ret += params;
  }
  else if (ret == ctn_YAOURT_TOOL)
  {
    if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");
    ret += params;
  }
  else if (ret == ctn_TRIZEN_TOOL)
  {
    if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");
    if (getAURNoEditParam()) params += QLatin1String(" --noedit ");
    ret += params;
  }
  else if (ret == ctn_PIKAUR_TOOL)
  {
    if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");
    if (getAURNoEditParam()) params += QLatin1String(" --noedit ");
    ret += params;
  }
  else if (ret == ctn_YAY_TOOL)
  {
    if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");
    if (getAURNoEditParam()) params += QLatin1String(" --noeditmenu ");
    ret += params;
  }
  //System does not have selected aurtool. Let's see if there are any other available...
  else if (ret.isEmpty() || !UnixCommand::hasTheExecutable(ret))
  {
    if (UnixCommand::hasTheExecutable(ctn_TRIZEN_TOOL))
    {
      if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");
      if (getAURNoEditParam()) params += QLatin1String(" --noedit ");

      p_instance.setAURTool(ctn_TRIZEN_TOOL);
      p_instance.getSYSsettings()->sync();
      ret = ctn_TRIZEN_TOOL + params;
    }
    else if (UnixCommand::hasTheExecutable(ctn_PIKAUR_TOOL))
    {
      if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");
      if (getAURNoEditParam()) params += QLatin1String(" --noedit ");

      p_instance.setAURTool(ctn_PIKAUR_TOOL);
      p_instance.getSYSsettings()->sync();
      ret = ctn_PIKAUR_TOOL + params;
    }
    else if (UnixCommand::hasTheExecutable(ctn_YAY_TOOL))
    {
      if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");
      if (getAURNoEditParam()) params += QLatin1String(" --noeditmenu ");

      p_instance.setAURTool(ctn_YAY_TOOL);
      p_instance.getSYSsettings()->sync();
      ret = ctn_YAY_TOOL + params;
    }
    else if (UnixCommand::hasTheExecutable(ctn_YAOURT_TOOL))
    {
      if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");

      p_instance.setAURTool(ctn_YAOURT_TOOL);
      p_instance.getSYSsettings()->sync();
      ret = ctn_YAOURT_TOOL + params;
    }
    else if (UnixCommand::hasTheExecutable(ctn_PACAUR_TOOL))
    {
      if (getAURNoConfirmParam()) params += QLatin1String(" --noconfirm ");
      if (getAURNoEditParam()) params += QLatin1String(" --noedit ");

      p_instance.setAURTool(ctn_PACAUR_TOOL);
      p_instance.getSYSsettings()->sync();
      ret = ctn_PACAUR_TOOL + params;
    }
  }

  return ret;
}

QString SettingsManager::getAURToolName()
{
  SettingsManager p_instance;
  return p_instance.getSYSsettings()->value(ctn_KEY_AUR_TOOL, ctn_NO_AUR_TOOL).toString();
}

/*
 * Tests if Pacaur is using "--noconfirm" parameter
 */
bool SettingsManager::getAURNoConfirmParam()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_AUR_NO_CONFIRM_PARAM, 0)).toBool();
}

/*
 * Tests if Pacaur is using "--noedit" parameter
 */
bool SettingsManager::getAURNoEditParam()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_AUR_NO_EDIT_PARAM, 0)).toBool();
}

bool SettingsManager::getSearchOutdatedAURPackages()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_SEARCH_OUTDATED_AUR_PACKAGES, 0)).toBool();
}

bool SettingsManager::getEnableAURVoting()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_ENABLE_AUR_VOTING, 0)).toBool();
}

QString SettingsManager::getAURUserName()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_AUR_USERNAME, QLatin1String(""))).toString();
}

QString SettingsManager::getAURPassword()
{
  SettingsManager p_instance;
  QByteArray encryptedValue = (p_instance.getSYSsettings()->value(ctn_KEY_AUR_PASSWORD, QLatin1String(""))).toByteArray();

  QString aurUserName = getAURUserName();
  if (aurUserName.isEmpty()) return QLatin1String("");

  QByteArray hashKey = QCryptographicHash::hash(ctn_OCTOPI_COPYRIGHT.toLocal8Bit(), QCryptographicHash::Sha256);
  QByteArray hashIV = QCryptographicHash::hash(aurUserName.toLocal8Bit(), QCryptographicHash::Md5);

  QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
  QByteArray decodeText = encryption.decode(encryptedValue, hashKey, hashIV);
  QString decryptedValue = QString::fromUtf8(encryption.removePadding(decodeText));
  return decryptedValue;
}

QString SettingsManager::getProxySettings()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_PROXY_SETTINGS, QLatin1String("")).toString());
}

bool SettingsManager::getUseAlternateRowColor()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_USE_ALTERNATE_ROW_COLOR, 0)).toBool();
}

bool SettingsManager::getEnableConfirmationDialogInSysUpgrade()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_ENABLE_TRANSACTION_DIALOG_IN_SYSTEM_UPGRADE, 1)).toBool();
}

bool SettingsManager::getEnableInternetChecking()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_ENABLE_INTERNET_CHECKING, 1)).toBool();
}

int SettingsManager::getConsoleFontSize()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_CONSOLE_SIZE, 0)).toInt();
}

/*
 * Retrieves value of field "SU_TOOL", without guessing for AUTOMATIC
 */
QString SettingsManager::readSUToolValue()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_SU_TOOL, ctn_AUTOMATIC)).toString();
}

QString SettingsManager::getSUTool()
{
  return ctn_OCTOPISUDO;
}

bool SettingsManager::getShowGroupsPanel()
{
  if (!instance()->getSYSsettings()->contains(ctn_KEY_SHOW_GROUPS_PANEL)){
    instance()->getSYSsettings()->setValue(ctn_KEY_SHOW_GROUPS_PANEL, true);
  }

  return (instance()->getSYSsettings()->value(ctn_KEY_SHOW_GROUPS_PANEL, false)).toBool();
}

/*
 * Returns true if the property "backend" has anything different from "alpm"
 * Defaults to using ALPM backend
 */
bool SettingsManager::hasPacmanBackend()
{
  if (!instance()->getSYSsettings()->contains(ctn_KEY_BACKEND))
  {
    instance()->getSYSsettings()->setValue(ctn_KEY_BACKEND, QStringLiteral("alpm"));
    return false;
  }
  else
  {
    SettingsManager p_instance;
    return (p_instance.getSYSsettings()->value(ctn_KEY_BACKEND, QStringLiteral("pacman")) != QLatin1String("alpm"));
  }
}

QByteArray SettingsManager::getCacheCleanerWindowSize()
{
  return (instance()->getSYSsettings()->value(ctn_KEY_CACHE_CLEANER_WINDOW_SIZE, 0).toByteArray());
}

QString SettingsManager::getTerminal()
{
  return ctn_QTERMWIDGET;
}

QByteArray SettingsManager::getWindowSize(){
  return (instance()->getSYSsettings()->value(ctn_KEY_WINDOW_SIZE, 0).toByteArray());
}

QByteArray SettingsManager::getTransactionWindowSize()
{
  return (instance()->getSYSsettings()->value(ctn_KEY_TRANSACTION_WINDOW_SIZE, 0).toByteArray());
}

QByteArray SettingsManager::getOutputDialogWindowSize()
{
  return (instance()->getSYSsettings()->value(ctn_KEY_OUTPUTDIALOG_WINDOW_SIZE, 0).toByteArray());
}

QByteArray SettingsManager::getOptionalDepsWindowSize()
{
  return (instance()->getSYSsettings()->value(ctn_KEY_OPTIONALDEPS_WINDOW_SIZE, 0).toByteArray());
}

QByteArray SettingsManager::getSplitterHorizontalState(){
  return (instance()->getSYSsettings()->value(ctn_KEY_SPLITTER_HORIZONTAL_STATE, 0).toByteArray());
}

void SettingsManager::setCurrentTabIndex(int newValue){
  instance()->getSYSsettings()->setValue(ctn_KEY_CURRENT_TAB_INDEX, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPackageListOrderedCol(int newValue){
  instance()->getSYSsettings()->setValue(ctn_KEY_PACKAGE_LIST_ORDERED_COL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPackageListSortOrder(int newValue){
  instance()->getSYSsettings()->setValue(ctn_KEY_PACKAGE_LIST_SORT_ORDER, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setAURPackageListOrderedCol(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_AUR_PACKAGE_LIST_ORDERED_COL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setAURPackageListSortOrder(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_AUR_PACKAGE_LIST_SORT_ORDER, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setShowGroupsPanel(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_SHOW_GROUPS_PANEL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPanelOrganizing(int newValue){
  instance()->getSYSsettings()->setValue(ctn_KEY_PANEL_ORGANIZING, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setUseDefaultAppIcon(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_USE_DEFAULT_APP_ICON, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOctopiBusyIconPath(const QString &newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_OCTOPI_BUSY_ICON_PATH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOctopiRedIconPath(const QString &newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_OCTOPI_RED_ICON_PATH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOctopiYellowIconPath(const QString &newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_OCTOPI_YELLOW_ICON_PATH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOctopiGreenIconPath(const QString &newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_OCTOPI_GREEN_ICON_PATH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setBackend(const QString &newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_BACKEND, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setDistroRSSUrl(const QString &newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_DISTRO_RSS_URL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setWindowSize(QByteArray newValue){
  instance()->getSYSsettings()->setValue(ctn_KEY_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setTransactionWindowSize(QByteArray newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_TRANSACTION_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOutputDialogWindowSize(QByteArray newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_OUTPUTDIALOG_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOptionalDepsWindowSize(QByteArray newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_OPTIONALDEPS_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setSplitterHorizontalState(QByteArray newValue){
  instance()->getSYSsettings()->setValue(ctn_KEY_SPLITTER_HORIZONTAL_STATE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setTerminal(const QString& newValue){
  instance()->getSYSsettings()->setValue(ctn_KEY_TERMINAL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPackageIconColumnWidth(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_PACKAGE_ICON_COLUMN_WIDTH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPackageNameColumnWidth(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_PACKAGE_NAME_COLUMN_WIDTH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPackageVersionColumnWidth(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_PACKAGE_VERSION_COLUMN_WIDTH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPackageRepositoryColumnWidth(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_PACKAGE_REPOSITORY_COLUMN_WIDTH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setShowPackageNumbersOutput(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_SHOW_PACKAGE_NUMBERS_OUTPUT, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setShowStopTransaction(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_SHOW_STOP_TRANSACTION, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setAURTool(const QString &newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_AUR_TOOL, newValue);
  instance()->getSYSsettings()->sync();
}

/*
 * Sets if Pacaur tool will use "--noconfirm" parameter
 */
void SettingsManager::setAURNoConfirmParam(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_AUR_NO_CONFIRM_PARAM, newValue);
  instance()->getSYSsettings()->sync();
}

/*
 * Sets if Pacaur tool will use "--noedit" parameter
 */
void SettingsManager::setAURNoEditParam(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_AUR_NO_EDIT_PARAM, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setSearchOutdatedAURPackages(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_SEARCH_OUTDATED_AUR_PACKAGES, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setEnableAURVoting(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_ENABLE_AUR_VOTING, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setAURUserName(const QString &newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_AUR_USERNAME, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setAURPassword(const QString &newValue)
{
  //We need to encrypt the given newValue

  QString aurUserName = getAURUserName();
  if (aurUserName.isEmpty()) return;

  QByteArray hashKey = QCryptographicHash::hash(ctn_OCTOPI_COPYRIGHT.toLocal8Bit(), QCryptographicHash::Sha256);
  QByteArray hashIV = QCryptographicHash::hash(aurUserName.toLocal8Bit(), QCryptographicHash::Md5);

  QAESEncryption encryption(QAESEncryption::AES_256, QAESEncryption::CBC);
  QByteArray encryptedValue = encryption.encode(newValue.toLocal8Bit(), hashKey, hashIV);

  instance()->getSYSsettings()->setValue(ctn_KEY_AUR_PASSWORD, encryptedValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setProxySettings(const QString &newValue)
{
  if (!newValue.isEmpty())
  {
    instance()->getSYSsettings()->setValue(ctn_KEY_PROXY_SETTINGS, newValue);
    instance()->getSYSsettings()->sync();
  }
}

void SettingsManager::setUseAlternateRowColor(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_USE_ALTERNATE_ROW_COLOR, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setEnableConfirmationDialogInSysUpgrade(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_ENABLE_TRANSACTION_DIALOG_IN_SYSTEM_UPGRADE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setEnableInternetChecking(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_ENABLE_INTERNET_CHECKING, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setSUTool(const QString &newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_SU_TOOL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setInstantSearchSelected(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_INSTANT_SEARCH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setConsoleFontSize(int newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_CONSOLE_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

bool SettingsManager::isInstantSearchSelected()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value(ctn_KEY_INSTANT_SEARCH, 1)).toBool();
}

//Octopi related --------------------------------------------------------------------
