#include "cachecleaner.h"
#include "ui_cachecleaner.h"

#include "../src/strconstants.h"

#include <QMessageBox>


/*
 * CacheCleaner window constructor
 */
CacheCleaner::CacheCleaner(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::CacheCleaner)
{
  ui->setupUi(this);

  m_cmdInstalled = new UnixCommand(this);
  m_cmdUninstalled = new UnixCommand(this);


  int keepInstalled = SettingsManager::getKeepNumInstalledPackages();
  ui->keepInstalledPackagesSpinner->setValue(keepInstalled);

  int keepUninstalled = SettingsManager::getKeepNumUninstalledPackages();
  ui->keepUninstalledPackagesSpinner->setValue(keepUninstalled);


  connect( ui->keepInstalledPackagesSpinner, SIGNAL( valueChanged(int) ), SLOT( keepInstalledChanged() ) );
  connect( ui->keepUninstalledPackagesSpinner, SIGNAL( valueChanged(int) ), SLOT( keepUninstalledChanged() ) );

  connect( ui->refreshInstalledButton, SIGNAL( clicked() ), SLOT( refreshInstalledCache() ) );
  connect( ui->refreshUninstalledButton, SIGNAL( clicked() ), SLOT( refreshUninstalledCache() ) );

  connect( ui->cleanInstalledButton, SIGNAL( clicked() ), SLOT( cleanInstalledCache() ) );
  connect( ui->cleanUninstalledButton, SIGNAL( clicked() ), SLOT( cleanUninstalledCache() ) );


  //refresh cache informations at startup
  refreshUninstalledCache();
  refreshInstalledCache();
}



/*
 * Cache Cleaner destructor
 */
CacheCleaner::~CacheCleaner()
{
  delete m_cmdInstalled;
  delete m_cmdUninstalled;
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



/*
 * Retrieve the number of archives to keep for installed packages
 */
int CacheCleaner::getKeepInstalledNumber()
{
  return ui->keepInstalledPackagesSpinner->value();
}



/*
 * Retrieve the number of archives to keep for uninstalled packages
 */
int CacheCleaner::getKeepUninstalledNumber()
{
  return ui->keepUninstalledPackagesSpinner->value();
}



/*
 * Retrieve the option string to pass to paccache for installed package
 */
QString CacheCleaner::getCleanInstalledOptions()
{
  return "-k " + QString::number(getKeepInstalledNumber());
}



/*
 * Retrieve the option string to pass to paccache for uninstalled package
 */
QString CacheCleaner::getCleanUninstalledOptions()
{
  return "-u -k " + QString::number(getKeepUninstalledNumber());
}



/*
 * Handle installed spinner change: disable the clean
 * button to ensure consistency between the list and
 * what will effectively be cleared if the user press
 * the button
 */
void CacheCleaner::keepInstalledChanged()
{
  ui->cleanInstalledButton->setEnabled(false);
}



/*
 * Handle uninstalled spinner change: disable the clean
 * button to ensure consistency between the list and
 * what will effectively be cleared if the user press
 * the button
 */
void CacheCleaner::keepUninstalledChanged()
{
  ui->cleanUninstalledButton->setEnabled(false);
}



/*
 * Refresh the view for installed packages
 */
void CacheCleaner::refreshInstalledCache()
{
  //update UI for background refresh
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui->refreshInstalledButton->setEnabled(false);
  ui->cleanInstalledButton->setEnabled(false);
  ui->installedPackagesList->clear();

  //connect handler slot and call the command
  QObject::connect(m_cmdInstalled, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SLOT( finishedDryrunInstalled ( int, QProcess::ExitStatus )) );

  m_cmdInstalled->executeCommandAsNormalUser("paccache -v -d " + getCleanInstalledOptions());
}



/*
 * Refresh the view for uninstalled packages
 */
void CacheCleaner::refreshUninstalledCache()
{
  //update UI for background refresh
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui->refreshUninstalledButton->setEnabled(false);
  ui->cleanUninstalledButton->setEnabled(false);
  ui->uninstalledPackagesList->clear();

  //connect handler slot and call the command
  QObject::connect(m_cmdUninstalled, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SLOT( finishedDryrunUninstalled ( int, QProcess::ExitStatus )) );

  m_cmdUninstalled->executeCommandAsNormalUser("paccache -v -d " + getCleanUninstalledOptions());
}



/*
 * Call paccache to effectively clear the cache for installed packages
 */
void CacheCleaner::cleanInstalledCache()
{
  //update UI buttons
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui->refreshInstalledButton->setEnabled(false);
  ui->cleanInstalledButton->setEnabled(false);

  //connect handler slot and call the command
  QObject::connect(m_cmdInstalled, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SLOT( finishedInstalled ( int, QProcess::ExitStatus )) );

  m_cmdInstalled->executeCommand("paccache -r " + getCleanInstalledOptions());
}



/*
 * Call paccache to effectively clear the cache for uninstalled packages
 */
void CacheCleaner::cleanUninstalledCache()
{
  //connect UI buttons
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui->refreshUninstalledButton->setEnabled(false);
  ui->cleanUninstalledButton->setEnabled(false);

  //connect handler slot and call the command
  QObject::connect(m_cmdUninstalled, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                   SLOT( finishedUninstalled ( int, QProcess::ExitStatus )) );

  m_cmdUninstalled->executeCommand("paccache -r " + getCleanUninstalledOptions());
}



/*
 * Handle the result of the refresh action for installed packages
 */
void CacheCleaner::finishedDryrunInstalled(int exitCode, QProcess::ExitStatus)
{
  //disconnect the handler
  QObject::disconnect(m_cmdInstalled, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                      SLOT( finishedDryrunInstalled ( int, QProcess::ExitStatus )) );

  QApplication::restoreOverrideCursor();

  if(exitCode != 0)
  {
    //process failed, provide info on errors
    QMessageBox::critical(this, "Error whith the underlying process", m_cmdInstalled->readAllStandardError());

  }
  else
  {
    //process finished successfully, process the resulting output
    QString output = m_cmdInstalled->readAllStandardOutput();
    processDryrunResult(output, ui->installedPackagesList, ui->cleanInstalledButton);
  }

  //in either case, reenable the refresh button
  ui->refreshInstalledButton->setEnabled(true);
}



/*
 * Handle the result of the refresh action for uninstalled packages
 */
void CacheCleaner::finishedDryrunUninstalled(int exitCode, QProcess::ExitStatus)
{
  //disconnect the handler
  QObject::disconnect(m_cmdUninstalled, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                      SLOT( finishedDryrunUninstalled ( int, QProcess::ExitStatus )) );

  QApplication::restoreOverrideCursor();

  if(exitCode != 0)
  {
    //process failed, provide info on errors
    QMessageBox::critical(this, "Error whith the underlying process", m_cmdUninstalled->readAllStandardError());

  }
  else
  {
    //process finished successfully, process the resulting output
    QString output = m_cmdUninstalled->readAllStandardOutput();
    processDryrunResult(output, ui->uninstalledPackagesList, ui->cleanUninstalledButton);
  }

  //in either case, reenable the refresh button
  ui->refreshUninstalledButton->setEnabled(true);
}



/*
 * Handle the result of the clean action for installed packages
 */
void CacheCleaner::finishedInstalled(int exitCode, QProcess::ExitStatus)
{
  //disconnect the handler
  QObject::disconnect(m_cmdInstalled, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                      SLOT( finishedInstalled ( int, QProcess::ExitStatus )) );

  QApplication::restoreOverrideCursor();

  if(exitCode != 0)
  {
    //process failed, provide info on errors
    QMessageBox::critical(this, "Error whith the underlying process", m_cmdInstalled->readAllStandardError());

  }
  else
  {
    //refresh the view
    refreshInstalledCache();
  }
}



/*
 * Handle the result of the clean action for installed packages
 */
void CacheCleaner::finishedUninstalled(int exitCode, QProcess::ExitStatus)
{
  //disconnect the handler
  QObject::disconnect(m_cmdUninstalled, SIGNAL( finished ( int, QProcess::ExitStatus )), this,
                      SLOT( finishedUninstalled ( int, QProcess::ExitStatus )) );

  QApplication::restoreOverrideCursor();

  if(exitCode != 0)
  {
    //process failed, provide info on errors
    QMessageBox::critical(this, "Error whith the underlying process", m_cmdInstalled->readAllStandardError());

  }
  else
  {
    //refresh the view
    refreshUninstalledCache();
  }
}



/*
 * Process the output of the refresh commands
 *
 * @param output The output of the dryrun process
 * @param list The listview to update (installed or uninstalled packages)
 * @param cleanButton The clean button to update (installed or uninstalled packages)
 */
void CacheCleaner::processDryrunResult(QString output, QListWidget *list, QPushButton *cleanButton) {
  QStringList lines = output.split(QRegExp("\\n"), QString::SkipEmptyParts);

  if(lines.length() == 1)
  {
    //"==> no candidate packages found for pruning"
    cleanButton->setText(tr("Clean"));

  }
  else
  {
    //process package list
    for(int i = 0; i < lines.length(); i++)
    {
      QString line = lines.at(i);

      if(i == 0)
        //skip the first line ("==> Candidate packages:")
        continue;

      else if(i == lines.length() - 1)
      {
        //extract size from "==> finished dry run: 8 candidates (disk space saved: 19.11 MiB)")
        QStringList components = line.split(" ");

        QString unit = components.takeLast();
        unit.remove(unit.length() - 1, 1);

        QString size = components.takeLast();

        cleanButton->setText(tr("Clean ") + " " + size + " " + unit);

      }
      else
        list->addItem(line);
    }

    //there is packages to clean so reenable the clean button
    cleanButton->setEnabled(true);
  }
}

