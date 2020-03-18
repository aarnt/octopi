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
#include "aurvote.h"

#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

/*
 * This is the Options Dialog called by Octopi and Notifier
 */

OptionsDialog::OptionsDialog(QWidget *parent) :
  QDialog(parent)
{
  if (parent->windowTitle() == QLatin1String("Octopi")) m_calledByOctopi = true;
  else m_calledByOctopi = false;

  setupUi(this);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  removeEventFilter(this);
  initialize();
}

/*
 * Opens AUR tab
 */
void OptionsDialog::gotoAURTab()
{
  setCurrentIndexByTabName(QStringLiteral("AUR"));
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
  QString dir = QStringLiteral("/usr/share/icons");
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
  QString dir = QStringLiteral("/usr/share/icons");
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
  QString dir = QStringLiteral("/usr/share/icons");
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
  QString dir = QStringLiteral("/usr/share/icons");
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

#ifdef ALPM_BACKEND
  initBackendTab();
#else
  removeTabByName(tr("Backend"));
#endif

  initIconTab();
  initUpdatesTab();

  if (m_calledByOctopi)
  {
    removeTabByName(tr("Updates"));
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
  cbUseAlternateRowColor->setChecked(SettingsManager::getUseAlternateRowColor());
  cbConfirmationDialogInSysUpgrade->setChecked(SettingsManager::getEnableConfirmationDialogInSysUpgrade());
  cbEnableInternetCheck->setChecked(SettingsManager::getEnableInternetChecking());
}

/*
 * Initializes available AUR tools (if any)
 */
void OptionsDialog::initAURTab()
{
  if (UnixCommand::getLinuxDistro() == ectn_KAOS ||
      UnixCommand::getLinuxDistro() == ectn_CHAKRA ||
      UnixCommand::getLinuxDistro() == ectn_PARABOLA)
  {
    removeTabByName(QStringLiteral("AUR"));
  }
  else
  {
    lblAURWarning->setStyleSheet(QStringLiteral("QLabel{ color: red; }"));
    QStringList aurTools=UnixCommand::getAvailableAURTools();

    connect(comboAUR, SIGNAL(currentTextChanged(const QString &)), this, SLOT(comboAURChanged(const QString &)));
    connect(bConnect, SIGNAL(clicked()), this, SLOT(onAURConnect()));
    connect(bRegister, SIGNAL(clicked()), this, SLOT(onAURRegister()));

    aurTools.sort();
    comboAUR->addItems(aurTools);
    comboAUR->setCurrentText(SettingsManager::getAURToolName());

    if (comboAUR->currentText() == ctn_PACAUR_TOOL)
    {
      cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
      cbNoEdit->setChecked(SettingsManager::getAURNoEditParam());
    }
    else if (comboAUR->currentText() == ctn_TRIZEN_TOOL)
    {
      cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
      cbNoEdit->setChecked(SettingsManager::getAURNoEditParam());
    }
    else if (comboAUR->currentText() == ctn_PIKAUR_TOOL)
    {
      cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
      cbNoEdit->setChecked(SettingsManager::getAURNoEditParam());
    }
    else if (comboAUR->currentText() == ctn_YAY_TOOL)
    {
      cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
      cbNoEdit->setChecked(SettingsManager::getAURNoEditParam());
    }
    else if (comboAUR->currentText() == ctn_YAOURT_TOOL)
    {
      cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
      cbNoEdit->setChecked(false);
      cbNoEdit->setEnabled(false);
    }

    cbSearchOutdatedAURPackages->setChecked(SettingsManager::getSearchOutdatedAURPackages());
    leAurPassword->setEchoMode(QLineEdit::Password);
    cbEnableAURVoting->setChecked(SettingsManager::getEnableAURVoting());

    leAurUserName->setText(SettingsManager::getAURUserName());
    leAurPassword->setText(SettingsManager::getAURPassword());
    onEnableAURVoting(cbEnableAURVoting->checkState());
    connect(cbEnableAURVoting, SIGNAL(stateChanged(int)), this, SLOT(onEnableAURVoting(int)));
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

void OptionsDialog::initUpdatesTab()
{
  lblCheck->setText(StrConstants::getNotiferSetupDialogGroupBoxTitle());
  rbOnceADay->setText(StrConstants::getOnceADay());
  rbOnceADayAt->setText(StrConstants::getOnceADayAt());
  lblOnceADayAt->setText(StrConstants::getOnceADayAtDesc());
  rbOnceEvery->setText(StrConstants::getOnceEvery());
  rbNever->setText(StrConstants::getNever());
  lblOnceEvery->setText(StrConstants::getOnceEveryDesc().arg(5).arg(44640));  

  connect(rbOnceADay, SIGNAL(clicked()), this, SLOT(selectOnceADay()));
  connect(rbOnceADayAt, SIGNAL(clicked()), this, SLOT(selectOnceADayAt()));
  connect(rbOnceEvery, SIGNAL(clicked()), this, SLOT(selectOnceEvery()));

  //First, which radio button do we select?
  int checkUpdatesInterval = SettingsManager::getCheckUpdatesInterval();
  int checkUpdatesHour = SettingsManager::getCheckUpdatesHour();
  bool useCheckUpdatesInterval = false;
  bool useCheckUpdatesHour = false;

  //User does NOT want to check for updates!
  if (checkUpdatesInterval == -2)
  {
    selectNever();
    return;
  }
  else if (checkUpdatesInterval == -1)
  {
    spinOnceEvery->setValue(5);
  }
  else
  {
    spinOnceEvery->setValue(checkUpdatesInterval);
    useCheckUpdatesInterval = true;
  }

  if (checkUpdatesHour == -1)
  {
    spinOnceADayAt->setValue(0);
  }
  else
  {
    spinOnceADayAt->setValue(checkUpdatesHour);
    useCheckUpdatesHour = true;
  }

  if (useCheckUpdatesInterval)
  {
    selectOnceEvery();
  }
  else if (useCheckUpdatesHour)
  {
    selectOnceADayAt();
  }
  else //We are using just "Once a day"!!!
  {
    selectOnceADay();
  }
}

/*
 * Change the active tabindex given the tabName
 */
void OptionsDialog::setCurrentIndexByTabName(const QString &tabName)
{
  for(int c=0; c<tabWidget->tabBar()->count(); c++)
  {
    if (tabWidget->tabText(c) == tabName)
    {
      tabWidget->setCurrentIndex(c);
      break;
    }
  }
}

/*
 * When user chooses OK button and saves all his changes
 */
void OptionsDialog::accept()
{
  CPUIntensiveComputing *cic=new CPUIntensiveComputing(this);
  bool emptyIconPath = false;
  bool AURHasChanged = false;
  bool AURVotingHasChanged = false;

  if (m_calledByOctopi)
  {
    //Set backend...
    if (SettingsManager::hasPacmanBackend() != rbPacman->isChecked() ||
        (!SettingsManager::hasPacmanBackend()) != rbAlpm->isChecked())
    {
      if (rbPacman->isChecked())
        SettingsManager::setBackend(QStringLiteral("pacman"));
      else
        SettingsManager::setBackend(QStringLiteral("alpm"));

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
  if (cbUseAlternateRowColor->isChecked() != SettingsManager::getUseAlternateRowColor())
  {
    SettingsManager::setUseAlternateRowColor(cbUseAlternateRowColor->isChecked());
    emit alternateRowColorsChanged();
  }
  if (cbConfirmationDialogInSysUpgrade->isChecked() != SettingsManager::getEnableConfirmationDialogInSysUpgrade())
  {
    SettingsManager::setEnableConfirmationDialogInSysUpgrade(cbConfirmationDialogInSysUpgrade->isChecked());
  }
  if (cbEnableInternetCheck->isChecked() != SettingsManager::getEnableInternetChecking())
  {
    SettingsManager::setEnableInternetChecking(cbEnableInternetCheck->isChecked());
  }

  //Set AUR Tool...
  if (UnixCommand::getLinuxDistro() != ectn_KAOS ||
      UnixCommand::getLinuxDistro() != ectn_CHAKRA ||
      UnixCommand::getLinuxDistro() != ectn_PARABOLA)
  {
    if (comboAUR->currentText() == ctn_PACAUR_TOOL && SettingsManager::getAURToolName() != ctn_PACAUR_TOOL)
    {
      SettingsManager::setAURTool(ctn_PACAUR_TOOL);
      AURHasChanged = true;
    }
    else if (comboAUR->currentText() == ctn_YAOURT_TOOL && SettingsManager::getAURToolName() != ctn_YAOURT_TOOL)
    {
      SettingsManager::setAURTool(ctn_YAOURT_TOOL);
      AURHasChanged = true;
    }
    else if (comboAUR->currentText() == ctn_TRIZEN_TOOL && SettingsManager::getAURToolName() != ctn_TRIZEN_TOOL)
    {
      SettingsManager::setAURTool(ctn_TRIZEN_TOOL);
      AURHasChanged = true;
    }
    else if (comboAUR->currentText() == ctn_PIKAUR_TOOL && SettingsManager::getAURToolName() != ctn_PIKAUR_TOOL)
    {
      SettingsManager::setAURTool(ctn_PIKAUR_TOOL);
      AURHasChanged = true;
    }   
    else if (comboAUR->currentText() == ctn_YAY_TOOL && SettingsManager::getAURToolName() != ctn_YAY_TOOL)
    {
      SettingsManager::setAURTool(ctn_YAY_TOOL);
      AURHasChanged = true;
    }
    else if (comboAUR->currentText() == ctn_NO_AUR_TOOL && SettingsManager::getAURToolName() != ctn_NO_AUR_TOOL)
    {
      SettingsManager::setAURTool(ctn_NO_AUR_TOOL);
      AURHasChanged = true;
    }

    if (comboAUR->currentText() == ctn_PACAUR_TOOL && cbNoConfirm->isChecked() != SettingsManager::getAURNoConfirmParam())
    {
      SettingsManager::setAURNoConfirmParam(cbNoConfirm->isChecked());
      AURHasChanged = true;
    }
    if (comboAUR->currentText() == ctn_PACAUR_TOOL && cbNoEdit->isChecked() != SettingsManager::getAURNoEditParam())
    {
      SettingsManager::setAURNoEditParam(cbNoEdit->isChecked());
      AURHasChanged = true;
    }
    if (comboAUR->currentText() == ctn_YAOURT_TOOL && cbNoConfirm->isChecked() != SettingsManager::getAURNoConfirmParam())
    {
      SettingsManager::setAURNoConfirmParam(cbNoConfirm->isChecked());
      AURHasChanged = true;
    }
    if (comboAUR->currentText() == ctn_TRIZEN_TOOL && cbNoConfirm->isChecked() != SettingsManager::getAURNoConfirmParam())
    {
      SettingsManager::setAURNoConfirmParam(cbNoConfirm->isChecked());
      AURHasChanged = true;
    }
    if (comboAUR->currentText() == ctn_TRIZEN_TOOL && cbNoEdit->isChecked() != SettingsManager::getAURNoEditParam())
    {
      SettingsManager::setAURNoEditParam(cbNoEdit->isChecked());
      AURHasChanged = true;
    }
    if (comboAUR->currentText() == ctn_PIKAUR_TOOL && cbNoConfirm->isChecked() != SettingsManager::getAURNoConfirmParam())
    {
      SettingsManager::setAURNoConfirmParam(cbNoConfirm->isChecked());
      AURHasChanged = true;
    }
    if (comboAUR->currentText() == ctn_PIKAUR_TOOL && cbNoEdit->isChecked() != SettingsManager::getAURNoEditParam())
    {
      SettingsManager::setAURNoEditParam(cbNoEdit->isChecked());
      AURHasChanged = true;
    }
    if (comboAUR->currentText() == ctn_YAY_TOOL && cbNoConfirm->isChecked() != SettingsManager::getAURNoConfirmParam())
    {
      SettingsManager::setAURNoConfirmParam(cbNoConfirm->isChecked());
      AURHasChanged = true;
    }
    if (comboAUR->currentText() == ctn_YAY_TOOL && cbNoEdit->isChecked() != SettingsManager::getAURNoEditParam())
    {
      SettingsManager::setAURNoEditParam(cbNoEdit->isChecked());
      AURHasChanged = true;
    }

    if (cbSearchOutdatedAURPackages->isChecked() != SettingsManager::getSearchOutdatedAURPackages())
    {
      SettingsManager::setSearchOutdatedAURPackages(cbSearchOutdatedAURPackages->isChecked());
      AURHasChanged = true;
    }

    if (cbEnableAURVoting->isChecked() != SettingsManager::getEnableAURVoting() ||
        leAurUserName->text() != SettingsManager::getAURUserName() ||
        leAurPassword->text() != SettingsManager::getAURPassword())
    {
      if (cbEnableAURVoting->isChecked()) //If we are enabling the voting system, let's check some things
      {
        if (leAurUserName->text().isEmpty())
        {
          delete cic;
          setCurrentIndexByTabName(QStringLiteral("AUR"));
          QMessageBox::critical(this, StrConstants::getError(), StrConstants::getErrorAURUserNameIsNotSet());
          return;
        }
        if (leAurPassword->text().isEmpty())
        {
          delete cic;
          setCurrentIndexByTabName(QStringLiteral("AUR"));
          QMessageBox::critical(this, StrConstants::getError(), StrConstants::getErrorAURPasswordIsNotSet());
          return;
        }

        //Here we test if the connection is ok!
        AurVote v;
        v.setUserName(leAurUserName->text());
        v.setPassword(leAurPassword->text());
        if (!v.login())
        {
          delete cic;
          QMessageBox::critical(this, StrConstants::getError(), StrConstants::getAURUserNameOrPasswordIsIncorrect());
          return;
        }
      }

      SettingsManager::setEnableAURVoting(cbEnableAURVoting->isChecked());
      SettingsManager::setAURUserName(leAurUserName->text());
      SettingsManager::setAURPassword(leAurPassword->text());

      AURVotingHasChanged = true;
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
    delete cic;
    setCurrentIndexByTabName(tr("Icon"));
    QMessageBox::critical(this, StrConstants::getError(), StrConstants::getErrorIconPathInfoIsNotSet());
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
    //Set Check Updates interval...
    if (rbOnceADay->isChecked())
    {
      SettingsManager::setCheckUpdatesHour(-1);
      SettingsManager::setCheckUpdatesInterval(-1);
    }
    else if (rbOnceADayAt->isChecked())
    {
      SettingsManager::setCheckUpdatesHour(spinOnceADayAt->value());
      SettingsManager::setCheckUpdatesInterval(-1);
    }
    else if (rbOnceEvery->isChecked())
    {
      SettingsManager::setCheckUpdatesInterval(spinOnceEvery->value());
    }
    else if (rbNever->isChecked())
    {
      SettingsManager::setCheckUpdatesInterval(-2);
    }
  }

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
  if (AURVotingHasChanged) emit AURVotingChanged();
  delete cic;
}

void OptionsDialog::removeTabByName(const QString &tabName)
{
  for (int i=0; i < tabWidget->count(); ++i)
  {
    if (tabWidget->tabText(i) == tabName)
      tabWidget->removeTab(i);
  }
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
  rbNever->setChecked(false);
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
  rbNever->setChecked(false);
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
  rbNever->setChecked(false);
}

/*
 * Whenever user selects the forth radio button, we have to disable some widgets
 */
void OptionsDialog::selectNever()
{
  rbOnceEvery->setChecked(false);
  spinOnceADayAt->setEnabled(false);
  spinOnceEvery->setEnabled(false);
  rbOnceADay->setChecked(false);
  rbOnceADayAt->setChecked(false);
  rbNever->setChecked(true);
}

/*
 * Whenever user changes AUR tool in the combobox
 */
void OptionsDialog::comboAURChanged(const QString &text)
{
  if (text == ctn_NO_AUR_TOOL)
  {
    cbNoConfirm->setChecked(false);
    cbNoConfirm->setEnabled(false);
    cbNoEdit->setChecked(false);
    cbNoEdit->setEnabled(false);
    cbSearchOutdatedAURPackages->setChecked(false);
    cbSearchOutdatedAURPackages->setEnabled(false);
  }
  else if (text == ctn_YAOURT_TOOL)
  {
    cbNoConfirm->setEnabled(true);
    cbNoEdit->setEnabled(true);
    cbSearchOutdatedAURPackages->setEnabled(true);
    cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
  }
  else if (text == ctn_PACAUR_TOOL)
  {
    cbNoConfirm->setEnabled(true);
    cbNoEdit->setEnabled(true);
    cbSearchOutdatedAURPackages->setEnabled(true);
    cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
    cbNoEdit->setChecked(SettingsManager::getAURNoEditParam());
    cbSearchOutdatedAURPackages->setChecked(SettingsManager::getSearchOutdatedAURPackages());
  }
  else if (text == ctn_PIKAUR_TOOL)
  {
    cbNoConfirm->setEnabled(true);
    cbNoEdit->setEnabled(true);
    cbSearchOutdatedAURPackages->setEnabled(true);
    cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
    cbNoEdit->setChecked(SettingsManager::getAURNoEditParam());
    cbSearchOutdatedAURPackages->setChecked(SettingsManager::getSearchOutdatedAURPackages());
  }
  else if (text == ctn_TRIZEN_TOOL)
  {
    cbNoConfirm->setEnabled(true);
    cbNoEdit->setEnabled(true);
    cbSearchOutdatedAURPackages->setEnabled(true);
    cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
    cbNoEdit->setChecked(SettingsManager::getAURNoEditParam());
    cbSearchOutdatedAURPackages->setChecked(SettingsManager::getSearchOutdatedAURPackages());
  }
  else if (text == ctn_YAY_TOOL)
  {
    cbNoConfirm->setEnabled(true);
    cbNoEdit->setEnabled(true);
    cbSearchOutdatedAURPackages->setEnabled(true);
    cbNoConfirm->setChecked(SettingsManager::getAURNoConfirmParam());
    cbNoEdit->setChecked(SettingsManager::getAURNoEditParam());
    cbSearchOutdatedAURPackages->setChecked(SettingsManager::getSearchOutdatedAURPackages());
  }
}

/*
 * Whenever user changes the state of 'Enable AUR voting' check box
 */
void OptionsDialog::onEnableAURVoting(int state)
{
  bool value=(state==Qt::Checked);
  gbAURVoting->setEnabled(value);
}

/*
 * Whenever user clicks "Connect" button to test AUR site connection
 */
void OptionsDialog::onAURConnect()
{
  AurVote v;
  v.setUserName(leAurUserName->text());
  v.setPassword(leAurPassword->text());
  bool logged = v.login();
  if(logged)
  {
    //Connection was ok. Let's ask user if he wants to help Octopi project by voting for it
    bool octopiDevVoted=false;
    bool octopiVoted=false;
    bool alpmUtilsVoted=false;
    if (v.isPkgVoted(QStringLiteral("octopi-dev"))==0) octopiDevVoted=true;
    if (v.isPkgVoted(QStringLiteral("octopi"))==0) octopiVoted=true;
    if (v.isPkgVoted(QStringLiteral("alpm_octopi_utils"))==0) alpmUtilsVoted=true;

    if (octopiDevVoted && octopiVoted && alpmUtilsVoted)
    {
      QMessageBox::information(this, StrConstants::getInformation(), StrConstants::getAURConnectionIsOK());
    }
    else
    {
      QMessageBox::StandardButton r = QMessageBox::question(this, StrConstants::getConfirmation(),
                            StrConstants::getAURConnectionIsOK() + "\n" +
                            StrConstants::getWouldYouLikeToHelpThisProject(),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::Yes);
      if (r == QMessageBox::Yes)
      {
        //User opted to help the project, so let's vote for the packages
        if (!octopiDevVoted) v.voteForPkg(QStringLiteral("octopi-dev"));
        if (!octopiVoted) v.voteForPkg(QStringLiteral("octopi"));
        if (!alpmUtilsVoted) v.voteForPkg(QStringLiteral("alpm_octopi_utils"));

        //Let's thank user for voting!
        QMessageBox::information(this, StrConstants::getInformation(), StrConstants::getThankYouForVoting());
      }
    }
  }
  else
    QMessageBox::critical(this, StrConstants::getError(), StrConstants::getAURUserNameOrPasswordIsIncorrect());
}

/*
 * Whenever user clicks "Register" button he is redirected to a browser to create an account at AUR site
 */
void OptionsDialog::onAURRegister()
{
  QDesktopServices::openUrl(QUrl(QStringLiteral("https://aur.archlinux.org/register/")));
}
