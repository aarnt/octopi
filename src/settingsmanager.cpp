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

SettingsManager* SettingsManager::m_pinstance = 0;

SettingsManager::SettingsManager(){
  m_SYSsettings = new QSettings(QSettings::UserScope, ctn_ORGANIZATION, ctn_APPLICATION);
}

SettingsManager::~SettingsManager(){  
  delete m_SYSsettings;
}

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

int SettingsManager::getInstalledPackageListOrderedCol(){
  return instance()->getSYSsettings()->value(
        ctn_KEY_INSTALLED_PACKAGE_LIST_ORDERED_COL, 1 ).toInt();
}

int SettingsManager::getPackageListSortOrder(){
  return instance()->getSYSsettings()->value(
        ctn_KEY_PACKAGE_LIST_SORT_ORDER, Qt::AscendingOrder ).toInt();
}

int SettingsManager::getInstalledPackageListSortOrder(){
  return instance()->getSYSsettings()->value(
        ctn_KEY_INSTALLED_PACKAGE_LIST_SORT_ORDER, Qt::AscendingOrder ).toInt();
}

bool SettingsManager::getShowToolBar(){
  return (instance()->getSYSsettings()->value( ctn_KEY_SHOW_TOOLBAR, true).toInt() == 1);
}

bool SettingsManager::getShowStatusBar(){
  return (instance()->getSYSsettings()->value( ctn_KEY_SHOW_STATUSBAR, true).toInt() == 1);
}

bool SettingsManager::getShowPackageTooltip(){
  return (instance()->getSYSsettings()->value( ctn_KEY_SHOW_PACKAGE_TOOLTIP, true).toInt() == 1);
}

bool SettingsManager::getStartIconified(){
  return (instance()->getSYSsettings()->value( ctn_KEY_START_ICONIFIED, false).toInt() == 1);
}

int SettingsManager::getHighlightedSearchItems(){
  return (instance()->getSYSsettings()->value( ctn_KEY_HIGHLIGHTED_SEARCH_ITEMS, 100).toInt());
}

bool SettingsManager::getSkipMirrorCheckAtStartup(){
  if (!instance()->getSYSsettings()->contains(ctn_KEY_SKIP_MIRRORCHECK_ON_STARTUP)){
    instance()->getSYSsettings()->setValue(ctn_KEY_SKIP_MIRRORCHECK_ON_STARTUP, false);
  }

  return (instance()->getSYSsettings()->value( ctn_KEY_SKIP_MIRRORCHECK_ON_STARTUP, false).toInt() == 1);
}

QString SettingsManager::getTerminal(){
  if (!instance()->getSYSsettings()->contains(ctn_KEY_TERMINAL)){
    instance()->getSYSsettings()->setValue(ctn_KEY_TERMINAL, ctn_AUTOMATIC);
  }

  return (instance()->getSYSsettings()->value( ctn_KEY_TERMINAL, ctn_AUTOMATIC)).toString();
}

bool SettingsManager::getUsePkgTools(){
  return (instance()->getSYSsettings()->value( ctn_KEY_USE_PKGTOOLS, true).toInt() == 1);
}

bool SettingsManager::getUseSilentActionOutput(){
  return (instance()->getSYSsettings()->value( ctn_KEY_USE_SILENT_ACTION_OUTPUT, true).toInt() == 1);
}

bool SettingsManager::getWindowCloseHidesApp(){
  return (instance()->getSYSsettings()->value( ctn_KEY_WINDOW_CLOSE_HIDES_APP, false).toInt() == 1);
}

int SettingsManager::getFontSizeFactor(){
  return (instance()->getSYSsettings()->value( ctn_KEY_FONT_SIZE_FACTOR, 0).toInt());
}

QByteArray SettingsManager::getWindowSize(){
  return (instance()->getSYSsettings()->value( ctn_KEY_WINDOW_SIZE, 0).toByteArray());
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

void SettingsManager::setInstalledPackageListOrderedCol(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_INSTALLED_PACKAGE_LIST_ORDERED_COL, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPackageListSortOrder(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_PACKAGE_LIST_SORT_ORDER, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setInstalledPackageListSortOrder(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_INSTALLED_PACKAGE_LIST_SORT_ORDER, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setPanelOrganizing(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_PANEL_ORGANIZING, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setFrozenPkgList(QStringList newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_FROZEN_PACKAGES_LIST,
                                          QVariant( static_cast<QStringList>(newValue)) );
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setDefaultDirectory(QString newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_DEFAULT_DIRECTORY, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setUpdaterDirectory(QString newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_UPDATER_DIRECTORY, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setUpdaterMirror(QString newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_UPDATER_MIRROR, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setShowToolBar(bool newValue){
  int value=0;
  if (newValue) value=1;

  instance()->getSYSsettings()->setValue( ctn_KEY_SHOW_TOOLBAR, value);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setShowStatusBar(bool newValue){
  int value=0;
  if (newValue) value=1;

  instance()->getSYSsettings()->setValue( ctn_KEY_SHOW_STATUSBAR, value);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setShowPackageTooltip(bool newValue){
  int value=0;
  if (newValue) value=1;

  instance()->getSYSsettings()->setValue( ctn_KEY_SHOW_PACKAGE_TOOLTIP, value);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setStartIconified(bool newValue){
  int value=0;
  if (newValue) value=1;

  instance()->getSYSsettings()->setValue( ctn_KEY_START_ICONIFIED, value);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setHighlightedSearchItems(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_HIGHLIGHTED_SEARCH_ITEMS, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setUsePkgTools(bool newValue){
  int value=0;
  if (newValue) value=1;

  instance()->getSYSsettings()->setValue( ctn_KEY_USE_PKGTOOLS, value);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setUseSilentActionOutput(bool newValue){
  int value=0;
  if (newValue) value=1;

  instance()->getSYSsettings()->setValue( ctn_KEY_USE_SILENT_ACTION_OUTPUT, value);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setWindowCloseHidesApp(bool newValue){
  int value=0;
  if (newValue) value=1;

  instance()->getSYSsettings()->setValue( ctn_KEY_WINDOW_CLOSE_HIDES_APP, value);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setFontSizeFactor(int newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_FONT_SIZE_FACTOR, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setWindowSize(QByteArray newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_WINDOW_SIZE, newValue);
  instance()->getSYSsettings()->sync();
}

void SettingsManager::setSplitterHorizontalState(QByteArray newValue){
  instance()->getSYSsettings()->setValue( ctn_KEY_SPLITTER_HORIZONTAL_STATE, newValue);
  instance()->getSYSsettings()->sync();
}

// Class singleton
SettingsManager* SettingsManager::instance(){
  if (m_pinstance == 0)
  {
    m_pinstance = new SettingsManager();
  }

  return m_pinstance;
}
