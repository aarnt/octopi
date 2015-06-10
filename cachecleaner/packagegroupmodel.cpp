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

#include "packagegroupmodel.h"
#include "../src/strconstants.h"

#include <QApplication>
#include <QMessageBox>

/*
 * Constructor
 *
 * @param listView The list view to display packages
 * @param spinner The spinner to configure the number of archive to keep
 * @param refreshBtn The refresh view button
 * @param cleanBtn The clean button
 */
PackageGroupModel::PackageGroupModel(QString optionsString,
                                     QListWidget *listView,
                                     QSpinBox *spinner,
                                     QPushButton *refreshBtn,
                                     QPushButton *cleanBtn)
                                        : QObject(NULL),
                                        m_optionsString(optionsString), 
                                        m_listView(listView), 
                                        m_spinner(spinner), 
                                        m_refreshButton(refreshBtn),
                                        m_cleanButton(cleanBtn),
                                        m_cmd(new UnixCommand(this)),
                                        m_acc(new ProcessOutputAccumulator(m_cmd)),
                                        m_oldKeepValue(spinner->value())
{
  m_cleanButton->setText(tr("Clean"));

  //setup UI slots
  connect( m_spinner, SIGNAL( valueChanged(int) ), SLOT( updateKeepArchives() ) );
  connect( m_spinner, SIGNAL( valueChanged(int) ), SLOT( refreshCacheView() ) );
  connect( m_spinner, SIGNAL( editingFinished() ), SLOT( keepArchivesChanged() ) );
  connect( m_refreshButton, SIGNAL( clicked() ), SLOT( refreshCacheView() ) );
  connect( m_cleanButton, SIGNAL( clicked() ), SLOT( cleanCache() ) );

  //refresh cache informations at startup
  refreshCacheView();
}

/*
 * Destructor
 */
PackageGroupModel::~PackageGroupModel()
{
  delete m_acc;
  delete m_cmd;
}

/*
 * Handle spinner change: disable the clean button
 * to ensure consistency between the list and what
 * will effectively be cleared if the user press
 * the button
 */
void PackageGroupModel::updateKeepArchives()
{
  m_cleanButton->setEnabled(false);
}

/*
 * Refresh the cache when the spinner change are validated
 */
void PackageGroupModel::keepArchivesChanged()
{
  if(m_spinner->value() != m_oldKeepValue)
    refreshCacheView();
}

/*
 * Return the options to pass to paccache according to the current context
 */
QString PackageGroupModel::getOptions()
{
  return m_optionsString + " -k " + QString::number(m_spinner->value());
}

/*
 * Refresh the view
 */
void PackageGroupModel::refreshCacheView()
{
  //update UI for background refresh
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_acc->reset();
  m_refreshButton->setEnabled(false);
  m_cleanButton->setEnabled(false);
  m_listView->clear();

  m_oldKeepValue = m_spinner->value();

  //connect handler slot and call the command
  QObject::connect(m_cmd, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( finishedDryrun ( int, QProcess::ExitStatus )) );

  m_cmd->executeCommandAsNormalUser("paccache -v -d " + getOptions());
}

/*
 * Checks if some SU utility is available...
 * Returns false if not!
 */
bool PackageGroupModel::isSUAvailable()
{
  //If there are no means to run the actions, we must warn!
  if (UnixCommand::isRootRunning() && WMHelper::isKDERunning())
  {
    return true;
  }
  else if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
    QMessageBox::critical( 0, StrConstants::getApplicationName(),
                           StrConstants::getErrorNoSuCommand() +
                           "\n" + StrConstants::getYoullNeedSuFrontend());
    return false;
  }
  else
    return true;
}

/*
 * Call paccache to effectively clear the cache
 */
void PackageGroupModel::cleanCache()
{
  if (!isSUAvailable())
    return;

  //update UI buttons
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_acc->reset();
  m_refreshButton->setEnabled(false);
  m_cleanButton->setEnabled(false);

  //connect handler slot and call the command
  QObject::connect(m_cmd, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( finishedClean( int, QProcess::ExitStatus )) );

  QByteArray tmp = "paccache -r " + getOptions().toLatin1();
  m_cmd->executeCommand(QLatin1String(tmp), ectn_LANG_USER_DEFINED);
}

/*
 * Handle the result of the refresh action
 */
void PackageGroupModel::finishedDryrun(int exitCode, QProcess::ExitStatus)
{
  //disconnect the handler
  QObject::disconnect(m_cmd, SIGNAL( finished ( int, QProcess::ExitStatus )),
                      this, SLOT( finishedDryrun ( int, QProcess::ExitStatus )) );

  QApplication::restoreOverrideCursor();

  if(exitCode > 1)
  {
    //process failed, provide info on errors
    QMessageBox::critical(m_listView, "Error whith the underlying process", m_acc->getErrors());
  }
  else if (exitCode == 0)
  {
    //process finished successfully, process the resulting output
    processDryrunResult(m_acc->getOutput());
  }

  //in either case, reenable the refresh button
  m_refreshButton->setEnabled(true);
}

/*
 * Handle the result of the clean action
 */
void PackageGroupModel::finishedClean(int exitCode, QProcess::ExitStatus)
{
  //disconnect the handler
  QObject::disconnect(m_cmd, SIGNAL( finished ( int, QProcess::ExitStatus ) ),
                      this, SLOT( finishedClean(int, QProcess::ExitStatus ) ) );

  QApplication::restoreOverrideCursor();

  if(exitCode != 0)
  {
    //process failed, provide info on errors
    QMessageBox::critical(m_listView, "Error whith the underlying process",m_acc->getErrors());
    m_refreshButton->setEnabled(true);
    m_cleanButton->setEnabled(true);
  }
  else
  {
    //refresh the view
    refreshCacheView();
  }
}

/*
 * Process the output of the refresh commands
 *
 * @param output The output of the dryrun process
 */
void PackageGroupModel::processDryrunResult(QString output) {
  QStringList lines = output.split(QRegExp("\\n"), QString::SkipEmptyParts);

  if(lines.length() == 1 || output.contains("*.pkg.tar?(.+([^.]))"))
  {
    //"==> no candidate packages found for pruning"
    m_cleanButton->setText(tr("Clean"));
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

        m_cleanButton->setText(tr("Clean %1").arg(" " + size + " " + unit));
      }
      else
        m_listView->addItem(line);
    }

    //there is packages to clean so reenable the clean button
    m_cleanButton->setEnabled(true);
  }
}
