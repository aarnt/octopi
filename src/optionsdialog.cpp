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

#include "optionsdialog.h"
#include "settingsmanager.h"
#include "unixcommand.h"
#include "wmhelper.h"
#include "strconstants.h"
#include "terminal.h"
#include "uihelper.h"

#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>

/*
 * This is the Options Dialog called by Octopi and Notifier
 */

OptionsDialog::OptionsDialog(QWidget *parent) :
  QDialog(parent),  
  m_once(false){

  if (parent->windowTitle() == "Octopi") m_calledByOctopi = true;
  else m_calledByOctopi = false;

  setupUi(this);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

  removeEventFilter(this);
  initialize();
}

/*
 * When the dialog is first displayed
 */
void OptionsDialog::paintEvent(QPaintEvent *){
  //This member flag ensures the execution of this code for just ONE time.
  if (!m_once){
    QList<QTableWidgetItem *> l = twTerminal->findItems(SettingsManager::getTerminal(), Qt::MatchExactly);
    if (l.count() == 1){
      twTerminal->setCurrentItem(l.at(0));
      twTerminal->scrollToItem(l.at(0));
    }
    m_once=true;
  }
}

/*
 * Whenever user changes selected tab
 */
void OptionsDialog::currentTabChanged(int tabIndex){
  if (tabWidget->tabText(tabIndex) == tr("Terminal"))
  {
    twTerminal->setFocus();
    QList<QTableWidgetItem*> l = twTerminal->findItems(SettingsManager::getTerminal(), Qt::MatchExactly);
    if (l.count() == 1)
    {
      twTerminal->setCurrentItem(l.at(0));
      twTerminal->scrollToItem(l.at(0));
    }
  }
  else if (tabWidget->tabText(tabIndex) == tr("SU tool"))
  {
    twSUTool->setFocus();
    QList<QTableWidgetItem*> l = twSUTool->findItems(SettingsManager::readSUToolValue(), Qt::MatchExactly);

    if (l.count() == 1)
    {
      twSUTool->setCurrentItem(l.at(0));
      twSUTool->scrollToItem(l.at(0));
    }
  }
}

/*
 * Whenever user checks/unchecks "Use default icons" option
 */
void OptionsDialog::defaultIconChecked(bool checked)
{
  if (checked)
  {
    leRedIcon->clear();
    leYellowIcon->clear();
    leGreenIcon->clear();
    leBusyIcon->clear();
    groupBoxIcons->setEnabled(false);
  }
  else
  {
    groupBoxIcons->setEnabled(true);
  }
}

/*
 * When user chooses new red icon path
 */
void OptionsDialog::selRedIconPath()
{
  QDir qd;
  QString dir = "/usr/share/icons";
  if (!leRedIcon->text().isEmpty()) dir = qd.filePath(leRedIcon->text());

  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Image"), dir, tr("Image Files") + " (*.bmp *.jpg *.png *.svg *.xmp)");

  if (!fileName.isEmpty())
    leRedIcon->setText(fileName);
}

/*
 * When user chooses new yellow icon path
 */
void OptionsDialog::selYellowIconPath()
{
  QDir qd;
  QString dir = "/usr/share/icons";
  if (!leYellowIcon->text().isEmpty()) dir = qd.filePath(leYellowIcon->text());

  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Image"), dir, tr("Image Files") + " (*.bmp *.jpg *.png *.svg *.xmp)");

  if (!fileName.isEmpty())
    leYellowIcon->setText(fileName);
}

/*
 * When user chooses new green icon path
 */
void OptionsDialog::selGreenIconPath()
{
  QDir qd;
  QString dir = "/usr/share/icons";
  if (!leGreenIcon->text().isEmpty()) dir = qd.filePath(leGreenIcon->text());

  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Image"), dir, tr("Image Files") + " (*.bmp *.jpg *.png *.svg *.xmp)");

  if (!fileName.isEmpty())
    leGreenIcon->setText(fileName);
}

/*
 * When user chooses new busy icon path
 */
void OptionsDialog::selBusyIconPath()
{
  QDir qd;
  QString dir = "/usr/share/icons";
  if (!leBusyIcon->text().isEmpty()) dir = qd.filePath(leBusyIcon->text());

  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Image"), dir, tr("Image Files") + " (*.bmp *.jpg *.png *.svg *.xmp)");

  if (!fileName.isEmpty())
    leBusyIcon->setText(fileName);
}

/*
 * Main initialization code
 */
void OptionsDialog::initialize(){
  m_backendHasChanged = false;
  m_iconHasChanged = false;

  initButtonBox();
  initGeneralTab();
  initAURTab();
  initBackendTab();
  initIconTab();
  initSUToolTab();
  initSynchronizationTab();
  initTerminalTab();

  if (m_calledByOctopi)
  {
    removeTabByName(tr("Synchronization"));
  }
  else
  {
    removeTabByName(tr("Backend"));
  }

  tabWidget->setCurrentIndex(0);
}

void OptionsDialog::initButtonBox(){
  buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
  buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
}

/*
 * Initializes General tab
 */
void OptionsDialog::initGeneralTab()
{
  cbShowPackageNumbersOutput->setChecked(SettingsManager::getShowPackageNumbersOutput());
  cbShowStopTransaction->setChecked(SettingsManager::getShowStopTransaction());
}

/*
 * Initializes available AUR tools (if any)
 */
void OptionsDialog::initAURTab()
{
  bool pacaurTool=false;
  bool yaourtTool=false;

  if ((UnixCommand::getLinuxDistro() != ectn_KAOS) &&
    (UnixCommand::getLinuxDistro() != ectn_CHAKRA))
  {
    if (UnixCommand::hasTheExecutable("pacaur"))
      pacaurTool=true;
    if (UnixCommand::hasTheExecutable("yaourt"))
      yaourtTool=true;
  }

  if (!pacaurTool && !yaourtTool)
  {
    removeTabByName("AUR");
  }
  else
  {
    if (!pacaurTool)
    {
      rbPacaur->setEnabled(false);
      cbPacaurNoConfirm->setEnabled(false);
      cbPacaurNoEdit->setEnabled(false);
    }
    if (!yaourtTool)
    {
      rbYaourt->setEnabled(false);
      cbYaourtNoConfirm->setEnabled(false);
    }

    if (SettingsManager::getAURToolName() == "pacaur")
      rbPacaur->setChecked(true);
    else if (SettingsManager::getAURToolName() == "yaourt")
      rbYaourt->setChecked(true);
    else if (SettingsManager::getAURToolName() == "DO_NOT_USE_AUR")
      rbDoNotUse->setChecked(true);    
    else if (pacaurTool)
    {
      rbPacaur->setChecked(true);
      SettingsManager::setAURTool("pacaur");
    }

    cbPacaurNoConfirm->setChecked(SettingsManager::getPacaurNoConfirmParam());
    cbPacaurNoEdit->setChecked(SettingsManager::getPacaurNoEditParam());
    cbYaourtNoConfirm->setChecked(SettingsManager::getYaourtNoConfirmParam());
  }    
}

/*
 * Initializes Backend tab
 */
void OptionsDialog::initBackendTab()
{
  if (SettingsManager::hasPacmanBackend())
    rbPacman->setChecked(true);
  else
    rbAlpm->setChecked(true);
}

/*
 * Initializes Icon tab
 */
void OptionsDialog::initIconTab()
{
  connect(cbUseDefaultIcons, SIGNAL(clicked(bool)), this, SLOT(defaultIconChecked(bool)));
  connect(tbSelRedIcon, SIGNAL(clicked(bool)), this, SLOT(selRedIconPath()));
  connect(tbSelYellowIcon, SIGNAL(clicked(bool)), this, SLOT(selYellowIconPath()));
  connect(tbSelGreenIcon, SIGNAL(clicked(bool)), this, SLOT(selGreenIconPath()));
  connect(tbSelBusyIcon, SIGNAL(clicked(bool)), this, SLOT(selBusyIconPath()));

  //Do we use default icon path?
  if (SettingsManager::getUseDefaultAppIcon())
  {
    cbUseDefaultIcons->setChecked(true);
    groupBoxIcons->setEnabled(false);
  }
  else
  {
    cbUseDefaultIcons->setChecked(false);
    groupBoxIcons->setEnabled((true));

    leRedIcon->setText(SettingsManager::getOctopiRedIconPath());
    leYellowIcon->setText(SettingsManager::getOctopiYellowIconPath());
    leGreenIcon->setText(SettingsManager::getOctopiGreenIconPath());
    leBusyIcon->setText(SettingsManager::getOctopiBusyIconPath());

    m_redIconPath = leRedIcon->text();
    m_yellowIconPath = leYellowIcon->text();
    m_greenIconPath = leGreenIcon->text();
    m_busyIconPath = leBusyIcon->text();
  }
}

/*
 * Initializes super user tool used
 */
void OptionsDialog::initSUToolTab()
{
  if (UnixCommand::getLinuxDistro() == ectn_KAOS)
  {
    if (m_calledByOctopi) removeTabByName(tr("SU tool"));
    else removeTabByName(tr("SU tool"));
    return;
  }

  QStringList list;
  list << ctn_AUTOMATIC;

  //Now we populate the list of available SU tools
  if (UnixCommand::hasTheExecutable(ctn_GKSU_2)){
    list << ctn_GKSU_2;
  }
  if (UnixCommand::hasTheExecutable(ctn_KDESU)){
    list << ctn_KDESU;
  }
  if (UnixCommand::hasTheExecutable(ctn_LXQTSU)){
    list << ctn_LXQTSU;
  }
  if (UnixCommand::hasTheExecutable(ctn_TDESU)){
    list << ctn_TDESU;
  }

  if (list.count() == 1)
  {
    if (m_calledByOctopi) removeTabByName(tr("SU tool"));
    else removeTabByName(tr("SU tool"));
    return;
  }

  twSUTool->setRowCount(list.count());
  twSUTool->setShowGrid(false);
  twSUTool->setColumnCount(1);
  twSUTool->setColumnWidth(0, 460);
  twSUTool->verticalHeader()->hide();
  twSUTool->horizontalHeader()->hide();
  twSUTool->setSelectionBehavior(QAbstractItemView::SelectRows);
  twSUTool->setSelectionMode(QAbstractItemView::SingleSelection);

  int row = 0;
  connect(twSUTool, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(accept()));

  while (row < (list.count()))
  {
    QTableWidgetItem *itemSU = new QTableWidgetItem();
    itemSU->setFlags(itemSU->flags() ^ Qt::ItemIsEditable);
    itemSU->setText(list.at(row));
    twSUTool->setItem(row, 0, itemSU);
    twSUTool->setRowHeight(row, 25);
    row++;
  }

  twSUTool->sortByColumn(0, Qt::AscendingOrder);
}

void OptionsDialog::initSynchronizationTab()
{
  lblSync->setText(StrConstants::getNotiferSetupDialogGroupBoxTitle());
  rbOnceADay->setText(StrConstants::getOnceADay());
  rbOnceADayAt->setText(StrConstants::getOnceADayAt());
  lblOnceADayAt->setText(StrConstants::getOnceADayAtDesc());
  rbOnceEvery->setText(StrConstants::getOnceEvery());
  lblOnceEvery->setText(StrConstants::getOnceEveryDesc().arg(5).arg(44640));

  connect(rbOnceADay, SIGNAL(clicked()), this, SLOT(selectOnceADay()));
  connect(rbOnceADayAt, SIGNAL(clicked()), this, SLOT(selectOnceADayAt()));
  connect(rbOnceEvery, SIGNAL(clicked()), this, SLOT(selectOnceEvery()));

  //First, which radio button do we select?
  int syncDbInterval = SettingsManager::getSyncDbInterval();
  int syncDbHour = SettingsManager::getSyncDbHour();
  bool useSyncDbInterval = false;
  bool useSyncDbHour = false;

  if (syncDbInterval == -1)
  {
    spinOnceEvery->setValue(5);
  }
  else if (syncDbInterval != -1)
  {
    spinOnceEvery->setValue(syncDbInterval);
    useSyncDbInterval = true;
  }
  if (syncDbHour == -1)
  {
    spinOnceADayAt->setValue(0);
  }
  else if (syncDbHour != -1)
  {
    spinOnceADayAt->setValue(syncDbHour);
    useSyncDbHour = true;
  }

  if (useSyncDbInterval)
  {
    rbOnceEvery->setChecked(true);
    selectOnceEvery();
  }
  else if (useSyncDbHour)
  {
    rbOnceADayAt->setChecked(true);
    selectOnceADayAt();
  }
  else //We are using just "Once a day"!!!
  {
    rbOnceADay->setChecked(true);
    selectOnceADay();
  }
}

/*
 * Initializes Terminal tab
 */
void OptionsDialog::initTerminalTab(){
  QStringList terminals = Terminal::getListOfAvailableTerminals();

  if (terminals.count() <= 2)
  {
    removeTabByName(tr("Terminal"));
    return;
  }

  twTerminal->setRowCount(terminals.count());

  int row=0;
  QString terminal;

  twTerminal->setShowGrid(false);
  twTerminal->setColumnCount(1);
  twTerminal->setColumnWidth(0, 460);
  twTerminal->verticalHeader()->hide();
  twTerminal->horizontalHeader()->hide();
  twTerminal->setSelectionBehavior(QAbstractItemView::SelectRows);
  twTerminal->setSelectionMode(QAbstractItemView::SingleSelection);

  connect(twTerminal, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(accept()));

  while (row < (terminals.count()))
  {
    QTableWidgetItem *itemTerminal = new QTableWidgetItem();
    itemTerminal->setFlags(itemTerminal->flags() ^ Qt::ItemIsEditable);
    itemTerminal->setText(terminals.at(row));
    twTerminal->setItem(row, 0, itemTerminal);
    twTerminal->setRowHeight(row, 25);
    row++;
  }

  twTerminal->sortByColumn(0, Qt::AscendingOrder);
}

/*
 * When user chooses OK button and saves all his changes
 */
void OptionsDialog::accept(){
  CPUIntensiveComputing cic;
  bool emptyIconPath = false;
  bool AURHasChanged = false;

  if (m_calledByOctopi)
  {
    //Set backend...
    if (SettingsManager::hasPacmanBackend() != rbPacman->isChecked() ||
        (!SettingsManager::hasPacmanBackend()) != rbAlpm->isChecked())
    {
      if (rbPacman->isChecked())
        SettingsManager::setBackend("pacman");
      else
        SettingsManager::setBackend("alpm");

      m_backendHasChanged = true;
    }
  }

  //Set General...
  if (cbShowPackageNumbersOutput->isChecked() != SettingsManager::getShowPackageNumbersOutput())
  {
    SettingsManager::setShowPackageNumbersOutput(cbShowPackageNumbersOutput->isChecked());
  }
  if (cbShowStopTransaction->isChecked() != SettingsManager::getShowStopTransaction())
  {
    SettingsManager::setShowStopTransaction(cbShowStopTransaction->isChecked());
  }

  //Set AUR Tool...
  if (tabAUR->isVisible())
  {
    if (rbPacaur->isChecked() && SettingsManager::getAURToolName() != "pacaur")
    {
      SettingsManager::setAURTool("pacaur");
      AURHasChanged = true;
    }
    else if (rbYaourt->isChecked() && SettingsManager::getAURToolName() != "yaourt")
    {
      SettingsManager::setAURTool("yaourt");
      AURHasChanged = true;
    }
    else if (rbDoNotUse->isChecked() && SettingsManager::getAURToolName() != "DO_NOT_USE_AUR")
    {
      SettingsManager::setAURTool("DO_NOT_USE_AUR");
      AURHasChanged = true;
    }

    if (cbPacaurNoConfirm->isChecked() != SettingsManager::getPacaurNoConfirmParam())
    {
      SettingsManager::setPacaurNoConfirmParam(cbPacaurNoConfirm->isChecked());
      AURHasChanged = true;
    }
    if (cbPacaurNoEdit->isChecked() != SettingsManager::getPacaurNoEditParam())
    {
      SettingsManager::setPacaurNoEditParam(cbPacaurNoEdit->isChecked());
      AURHasChanged = true;
    }
    if (cbYaourtNoConfirm->isChecked() != SettingsManager::getYaourtNoConfirmParam())
    {
      SettingsManager::setYaourtNoConfirmParam(cbYaourtNoConfirm->isChecked());
      AURHasChanged = true;
    }
  }

  //Set icon...
  if (!cbUseDefaultIcons->isChecked())
  {
    if (leRedIcon->text().isEmpty())
    {
      emptyIconPath = true;
    }
    else if (leYellowIcon->text().isEmpty())
    {
      emptyIconPath = true;
    }
    else if (leGreenIcon->text().isEmpty())
    {
      emptyIconPath = true;
    }
    else if (leBusyIcon->text().isEmpty())
    {
      emptyIconPath = true;
    }
  }

  if (emptyIconPath)
  {
    QMessageBox::critical(this, StrConstants::getError(), StrConstants::getErrorIconPathInfoIncomplete());
    return;
  }

  if (SettingsManager::getUseDefaultAppIcon() != cbUseDefaultIcons->isChecked())
  {
    SettingsManager::setUseDefaultAppIcon(cbUseDefaultIcons->isChecked());
    m_iconHasChanged = true;
  }

  if (leRedIcon->text() != m_redIconPath)
  {
    SettingsManager::setOctopiRedIconPath(leRedIcon->text());
    m_iconHasChanged = true;
  }

  if (leYellowIcon->text() != m_yellowIconPath)
  {
    SettingsManager::setOctopiYellowIconPath(leYellowIcon->text());
    m_iconHasChanged = true;
  }

  if (leGreenIcon->text() != m_greenIconPath)
  {
    SettingsManager::setOctopiGreenIconPath(leGreenIcon->text());
    m_iconHasChanged = true;
  }

  if (leBusyIcon->text() != m_busyIconPath)
  {
    SettingsManager::setOctopiBusyIconPath(leBusyIcon->text());
    m_iconHasChanged = true;
  }

  if (!m_calledByOctopi)
  {
    //Set synchronization...
    if (rbOnceADay->isChecked())
    {
      SettingsManager::setSyncDbHour(-1);
      SettingsManager::setSyncDbInterval(-1);
    }
    else if (rbOnceADayAt->isChecked())
    {
      SettingsManager::setSyncDbHour(spinOnceADayAt->value());
      SettingsManager::setSyncDbInterval(-1);
    }
    else if (rbOnceEvery->isChecked())
    {
      SettingsManager::setSyncDbInterval(spinOnceEvery->value());
    }
  }

  //Set SU tool...
  QString selectedSUTool = SettingsManager::getSUTool();

  if (twSUTool->currentItem())
    selectedSUTool = twSUTool->item(twSUTool->row(twSUTool->currentItem()), 0)->text();

  if (SettingsManager::getSUTool() != selectedSUTool)
    SettingsManager::setSUTool(selectedSUTool);

  //Set terminal...
  QString selectedTerminal;

  if (twTerminal->currentItem())
    selectedTerminal = twTerminal->item(twTerminal->row(twTerminal->currentItem()), 0)->text();

  if (SettingsManager::getTerminal() != selectedTerminal)
    SettingsManager::setTerminal(selectedTerminal);

  Options::result res=0;

  if (m_iconHasChanged)
  {
    res |= Options::ectn_ICON;
  }
  if (m_backendHasChanged)
  {
    res |= Options::ectn_BACKEND;
  }

  QDialog::accept();
  setResult(res);

  if (AURHasChanged) emit AURToolChanged();
}

/*
 * Whenever user selects the first radio button, we have to disable some widgets
 */
void OptionsDialog::selectOnceADay()
{
  rbOnceADay->setChecked(true);
  spinOnceADayAt->setEnabled(false);
  spinOnceEvery->setEnabled(false);
  rbOnceADayAt->setChecked(false);
  rbOnceEvery->setChecked(false);
}

/*
 * Whenever user selects the second radio button, we have to disable some widgets
 */
void OptionsDialog::selectOnceADayAt()
{
  rbOnceADayAt->setChecked(true);
  spinOnceADayAt->setEnabled(true);
  spinOnceEvery->setEnabled(false);
  rbOnceADay->setChecked(false);
  rbOnceEvery->setChecked(false);
}

/*
 * Whenever user selects the third radio button, we have to disable some widgets
 */
void OptionsDialog::selectOnceEvery()
{
  rbOnceEvery->setChecked(true);
  spinOnceADayAt->setEnabled(false);
  spinOnceEvery->setEnabled(true);
  rbOnceADay->setChecked(false);
  rbOnceADayAt->setChecked(false);
}

void OptionsDialog::removeTabByName(const QString &tabName)
{
  for (int i=0; i < tabWidget->count(); ++i)
  {
    if (tabWidget->tabText(i) == tabName)
      tabWidget->removeTab(i);
  }
}
