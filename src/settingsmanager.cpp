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

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QSettings>
#include <QDir>

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
  instance()->getSYSsettings()->setValue( ctn_KEY_CACHE_CLEANER_WINDOW_SIZE, newValue);
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
    return (p_instance.getSYSsettings()->value( ctn_KEY_LAST_CHECKUPDATES_TIME, 0)).toDateTime();
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
  return instance()->getSYSsettings()->value( ctn_KEY_PANEL_ORGANIZING, ectn_NORMAL ).toInt();
}

int SettingsManager::getPackageListOrderedCol(){
  return instance()->getSYSsettings()->value( ctn_KEY_PACKAGE_LIST_ORDERED_COL, 1 ).toInt();
}

int SettingsManager::getPackageListSortOrder(){
  return instance()->getSYSsettings()->value(
        ctn_KEY_PACKAGE_LIST_SORT_ORDER, Qt::AscendingOrder ).toInt();
}

int SettingsManager::getAURPackageListOrderedCol()
{
  return instance()->getSYSsettings()->value( ctn_KEY_AUR_PACKAGE_LIST_ORDERED_COL, 1 ).toInt();
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
    return (p_instance.getSYSsettings()->value( ctn_KEY_USE_DEFAULT_APP_ICON, true) == true);
  }
}

QString SettingsManager::getOctopiBusyIconPath()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_OCTOPI_BUSY_ICON_PATH, "")).toString();
}

QString SettingsManager::getOctopiRedIconPath()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_OCTOPI_RED_ICON_PATH, "")).toString();
}

QString SettingsManager::getOctopiYellowIconPath()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_OCTOPI_YELLOW_ICON_PATH, "")).toString();
}

QString SettingsManager::getOctopiGreenIconPath()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_OCTOPI_GREEN_ICON_PATH, "")).toString();
}

bool SettingsManager::getShowPackageNumbersOutput()
{
  SettingsManager p_instance;
  return p_instance.getSYSsettings()->value( ctn_KEY_SHOW_PACKAGE_NUMBERS_OUTPUT, 1).toBool();
}

bool SettingsManager::getShowStopTransaction()
{
  SettingsManager p_instance;
  return p_instance.getSYSsettings()->value( ctn_KEY_SHOW_STOP_TRANSACTION, 1).toBool();
}

QString SettingsManager::getAURTool()
{
  QString params;

  SettingsManager p_instance;
  QString ret = (p_instance.getSYSsettings()->value( ctn_KEY_AUR_TOOL, "")).toString();

  if (ret == ctn_NO_AUR_TOOL) return ret;
  else if (ret == ctn_PACAUR_TOOL)
  {
    if (getAurNoConfirmParam()) params += " --noconfirm ";
    if (getAurNoEditParam()) params += " --noedit ";
    ret += params;
  }
  else if (ret == ctn_YAOURT_TOOL)
  {
    if (getAurNoConfirmParam()) params += " --noconfirm ";
    ret += params;
  }
  else if (ret == ctn_TRIZEN_TOOL)
  {
    if (getAurNoConfirmParam()) params += " --noconfirm ";
    if (getAurNoEditParam()) params += " --noedit ";
    ret += params;
  }
  else if (ret == ctn_PIKAUR_TOOL)
  {
    if (getAurNoConfirmParam()) params += " --noconfirm ";
    if (getAurNoEditParam()) params += " --noedit ";
    ret += params;
  }
  else if (ret == ctn_YAY_TOOL)
  {
    if (getAurNoConfirmParam()) params += " --noconfirm ";
    if (getAurNoEditParam()) params += " --noeditmenu ";
    ret += params;
  }
  //System does not have selected aurtool. Let's see if there are any other available...
  else if (ret.isEmpty() || !UnixCommand::hasTheExecutable(ret))
  {
    if (UnixCommand::hasTheExecutable(ctn_TRIZEN_TOOL))
    {
      if (getAurNoConfirmParam()) params += " --noconfirm ";
      if (getAurNoEditParam()) params += " --noedit ";

      p_instance.setAURTool(ctn_TRIZEN_TOOL);
      p_instance.getSYSsettings()->sync();
      ret = ctn_TRIZEN_TOOL + params;
    }
    else if (UnixCommand::hasTheExecutable(ctn_PIKAUR_TOOL))
    {
      if (getAurNoConfirmParam()) params += " --noconfirm ";
      if (getAurNoEditParam()) params += " --noedit ";

      p_instance.setAURTool(ctn_PIKAUR_TOOL);
      p_instance.getSYSsettings()->sync();
      ret = ctn_PIKAUR_TOOL + params;
    }
    else if (UnixCommand::hasTheExecutable(ctn_YAY_TOOL))
    {
      if (getAurNoConfirmParam()) params += " --noconfirm ";
      if (getAurNoEditParam()) params += " --noeditmenu ";

      p_instance.setAURTool(ctn_YAY_TOOL);
      p_instance.getSYSsettings()->sync();
      ret = ctn_YAY_TOOL + params;
    }
    else if (UnixCommand::hasTheExecutable(ctn_YAOURT_TOOL))
    {
      if (getAurNoConfirmParam()) params += " --noconfirm ";

      p_instance.setAURTool(ctn_YAOURT_TOOL);
      p_instance.getSYSsettings()->sync();
      ret = ctn_YAOURT_TOOL + params;
    }
    else if (UnixCommand::hasTheExecutable(ctn_PACAUR_TOOL))
    {
      if (getAurNoConfirmParam()) params += " --noconfirm ";
      if (getAurNoEditParam()) params += " --noedit ";

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
  return p_instance.getSYSsettings()->value( ctn_KEY_AUR_TOOL, ctn_NO_AUR_TOOL).toString();
}

/*
 * Tests if Pacaur is using "--noconfirm" parameter
 */
bool SettingsManager::getAurNoConfirmParam()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_AUR_NO_CONFIRM_PARAM, 0)).toBool();
}

/*
 * Tests if Pacaur is using "--noedit" parameter
 */
bool SettingsManager::getAurNoEditParam()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_AUR_NO_EDIT_PARAM, 0)).toBool();
}

bool SettingsManager::getSearchOutdatedAURPackages()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_SEARCH_OUTDATED_AUR_PACKAGES, 0)).toBool();
}

bool SettingsManager::getUseAlternateRowColor()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_USE_ALTERNATE_ROW_COLOR, 0)).toBool();
}

int SettingsManager::getConsoleFontSize()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_CONSOLE_SIZE, 0)).toInt();
}

/*
 * Retrieves value of field "SU_TOOL", without guessing for AUTOMATIC
 */
QString SettingsManager::readSUToolValue()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_SU_TOOL, ctn_AUTOMATIC)).toString();
}

QString SettingsManager::getSUTool()
{
  SettingsManager p_instance;
  QString ret = (p_instance.getSYSsettings()->value( ctn_KEY_SU_TOOL, ctn_AUTOMATIC)).toString();

  if (ret == ctn_AUTOMATIC)
  {
    ret = WMHelper::getSUTool();
  }

  return ret;
}

bool SettingsManager::getSkipMirrorCheckAtStartup(){
  if (!instance()->getSYSsettings()->contains(ctn_KEY_SKIP_MIRRORCHECK_ON_STARTUP)){
    instance()->getSYSsettings()->setValue(ctn_KEY_SKIP_MIRRORCHECK_ON_STARTUP, 0);
  }

  return (instance()->getSYSsettings()->value( ctn_KEY_SKIP_MIRRORCHECK_ON_STARTUP, false).toInt() == 1);
}

bool SettingsManager::getShowGroupsPanel()
{
  if (!instance()->getSYSsettings()->contains(ctn_KEY_SHOW_GROUPS_PANEL)){
    instance()->getSYSsettings()->setValue(ctn_KEY_SHOW_GROUPS_PANEL, 1);
  }

  return (instance()->getSYSsettings()->value( ctn_KEY_SHOW_GROUPS_PANEL, false).toInt() == 1);
}

/*
 * Returns true if the property "backend" has anything different from "alpm"
 */
bool SettingsManager::hasPacmanBackend()
{
  if (!instance()->getSYSsettings()->contains(ctn_KEY_BACKEND))
  {
    instance()->getSYSsettings()->setValue(ctn_KEY_BACKEND, "pacman");
    return true;
  }
  else
  {
    SettingsManager p_instance;
    return (p_instance.getSYSsettings()->value( ctn_KEY_BACKEND, "pacman") != "alpm");
  }
}

QByteArray SettingsManager::getCacheCleanerWindowSize()
{
  return (instance()->getSYSsettings()->value( ctn_KEY_CACHE_CLEANER_WINDOW_SIZE, 0).toByteArray());
}

QString SettingsManager::getTerminal(){
  return ctn_QTERMWIDGET;

  /*if (!instance()->getSYSsettings()->contains(ctn_KEY_TERMINAL))
  {
    instance()->getSYSsettings()->setValue(ctn_KEY_TERMINAL, ctn_AUTOMATIC);
    return ctn_AUTOMATIC;
  }
  else
  {
    SettingsManager p_instance;
    return (p_instance.getSYSsettings()->value( ctn_KEY_TERMINAL, ctn_AUTOMATIC)).toString();
  }*/
}

QByteArray SettingsManager::getWindowSize(){
  return (instance()->getSYSsettings()->value( ctn_KEY_WINDOW_SIZE, 0).toByteArray());
}

QByteArray SettingsManager::getTransactionWindowSize()
{
  return (instance()->getSYSsettings()->value( ctn_KEY_TRANSACTION_WINDOW_SIZE, 0).toByteArray());
}

QByteArray SettingsManager::getOutputDialogWindowSize()
{
  return (instance()->getSYSsettings()->value( ctn_KEY_OUTPUTDIALOG_WINDOW_SIZE, 0).toByteArray());
}

QByteArray SettingsManager::getOptionalDepsWindowSize()
{
  return (instance()->getSYSsettings()->value( ctn_KEY_OPTIONALDEPS_WINDOW_SIZE, 0).toByteArray());
}

QByteArray SettingsManager::getSplitterHorizontalState(){
  return (instance()->getSYSsettings()->value( ctn_KEY_SPLITTER_HORIZONTAL_STATE, 0).toByteArray());
}

void SettingsManager::setCurrentTabIndex(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_CURRENT_TAB_INDEX, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPackageListOrderedCol(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_PACKAGE_LIST_ORDERED_COL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPackageListSortOrder(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_PACKAGE_LIST_SORT_ORDER, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setAURPackageListOrderedCol(int newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_AUR_PACKAGE_LIST_ORDERED_COL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setAURPackageListSortOrder(int newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_AUR_PACKAGE_LIST_SORT_ORDER, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setShowGroupsPanel(int newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_SHOW_GROUPS_PANEL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPanelOrganizing(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_PANEL_ORGANIZING, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setUseDefaultAppIcon(bool newValue)
{
  //int v=0;
  //if (newValue) v=1;

  instance()->getSYSsettings()->setValue( ctn_KEY_USE_DEFAULT_APP_ICON, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOctopiBusyIconPath(const QString &newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_OCTOPI_BUSY_ICON_PATH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOctopiRedIconPath(const QString &newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_OCTOPI_RED_ICON_PATH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOctopiYellowIconPath(const QString &newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_OCTOPI_YELLOW_ICON_PATH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOctopiGreenIconPath(const QString &newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_OCTOPI_GREEN_ICON_PATH, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setBackend(const QString &newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_BACKEND, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setWindowSize(QByteArray newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setTransactionWindowSize(QByteArray newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_TRANSACTION_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOutputDialogWindowSize(QByteArray newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_OUTPUTDIALOG_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setOptionalDepsWindowSize(QByteArray newValue)
{
  instance()->getSYSsettings()->setValue( ctn_KEY_OPTIONALDEPS_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setSplitterHorizontalState(QByteArray newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_SPLITTER_HORIZONTAL_STATE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setTerminal(const QString& newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_TERMINAL, newValue);
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
void SettingsManager::setAurNoConfirmParam(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_AUR_NO_CONFIRM_PARAM, newValue);
  instance()->getSYSsettings()->sync();
}

/*
 * Sets if Pacaur tool will use "--noedit" parameter
 */
void SettingsManager::setAurNoEditParam(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_AUR_NO_EDIT_PARAM, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setSearchOutdatedAURPackages(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_SEARCH_OUTDATED_AUR_PACKAGES, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setUseAlternateRowColor(bool newValue)
{
  instance()->getSYSsettings()->setValue(ctn_KEY_USE_ALTERNATE_ROW_COLOR, newValue);
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

/*
 * Search all supported SU tools to see if the selected one is valid
 */
bool SettingsManager::isValidSUToolSelected()
{
  QString userSUTool = readSUToolValue();

  if (userSUTool == ctn_AUTOMATIC)
    return true;

  if (userSUTool == ctn_GKSU_2 ||
      userSUTool == ctn_KDESU ||
      userSUTool == ctn_LXQTSU ||
      //userSUTool == ctn_OCTOPISUDO ||
      userSUTool == ctn_TDESU)
  {
    if (UnixCommand::hasTheExecutable(userSUTool))
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

/*
 * Search all supported terminals to see if the selected one is valid
 */
bool SettingsManager::isValidTerminalSelected()
{
  QString userTerminal = getTerminal();

  if (userTerminal == ctn_AUTOMATIC)
    return true;

#ifdef QTERMWIDGET
  if (userTerminal == ctn_XFCE_TERMINAL ||
      userTerminal == ctn_LXDE_TERMINAL ||
      userTerminal == ctn_LXQT_TERMINAL ||
      userTerminal == ctn_KDE_TERMINAL ||
      userTerminal == ctn_TDE_TERMINAL ||
      userTerminal == ctn_CINNAMON_TERMINAL ||
      userTerminal == ctn_MATE_TERMINAL ||
      userTerminal == ctn_RXVT_TERMINAL ||
      userTerminal == ctn_QTERMWIDGET ||
      userTerminal == ctn_XTERM)
  {
    if (userTerminal == ctn_QTERMWIDGET) return true;
    else if (UnixCommand::hasTheExecutable(userTerminal))
    {
      return true;
    }
    else
    {
      return false;
    }
  }
#else
  if (userTerminal == ctn_XFCE_TERMINAL ||
      userTerminal == ctn_LXDE_TERMINAL ||
      userTerminal == ctn_LXQT_TERMINAL ||
      userTerminal == ctn_KDE_TERMINAL ||
      userTerminal == ctn_TDE_TERMINAL ||
      userTerminal == ctn_CINNAMON_TERMINAL ||
      userTerminal == ctn_MATE_TERMINAL ||
      userTerminal == ctn_RXVT_TERMINAL ||
      userTerminal == ctn_XTERM)
  {
    if (UnixCommand::hasTheExecutable(userTerminal))
    {
      return true;
    }
    else
    {
      return false;
    }
  }
#endif

  else
  {
    return false;
  }

}

bool SettingsManager::isInstantSearchSelected()
{
  SettingsManager p_instance;
  return (p_instance.getSYSsettings()->value( ctn_KEY_INSTANT_SEARCH, 1)).toBool();
}

//Octopi related --------------------------------------------------------------------
