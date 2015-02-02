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
  SettingsManager::setKeepNumInstalledPackages(ui->keepInstalledPackagesSpinner->value());
  SettingsManager::setKeepNumUninstalledPackages(ui->keepUninstalledPackagesSpinner->value());
}
