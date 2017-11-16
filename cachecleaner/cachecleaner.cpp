/*
Copyright 2015 Michaël Lhomme

This file is part of AppSet.

AppSet is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

AppSet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with AppSet; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "cachecleaner.h"
#include "ui_cachecleaner.h"

#include "../src/strconstants.h"

/*
 * CacheCleaner window constructor
 */
CacheCleaner::CacheCleaner(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::CacheCleaner)
{
  //UI initialization
  ui->setupUi(this);

  int keepInstalled = SettingsManager::getKeepNumInstalledPackages();
  ui->keepInstalledPackagesSpinner->setValue(keepInstalled);

  int keepUninstalled = SettingsManager::getKeepNumUninstalledPackages();
  ui->keepUninstalledPackagesSpinner->setValue(keepUninstalled);


  //create package group wrappers
  m_installed = new PackageGroupModel("",
                                      ui->installedPackagesList,
                                      ui->keepInstalledPackagesSpinner,
                                      ui->refreshInstalledButton,
                                      ui->cleanInstalledButton);

  m_uninstalled = new PackageGroupModel("-u",
                                        ui->uninstalledPackagesList,
                                        ui->keepUninstalledPackagesSpinner,
                                        ui->refreshUninstalledButton,                                                                                
                                        ui->cleanUninstalledButton);

  restoreGeometry(SettingsManager::getCacheCleanerWindowSize());
}

/*
 * Cache Cleaner destructor
 */
CacheCleaner::~CacheCleaner()
{
  delete m_installed;
  delete m_uninstalled;
  delete ui;
}

/*
 * Save settings when closing window
 */
void CacheCleaner::closeEvent(QCloseEvent *)
{
  QByteArray windowSize=saveGeometry();

  SettingsManager::setCacheCleanerWindowSize(windowSize);
  SettingsManager::setKeepNumInstalledPackages(ui->keepInstalledPackagesSpinner->value());
  SettingsManager::setKeepNumUninstalledPackages(ui->keepUninstalledPackagesSpinner->value());
}
