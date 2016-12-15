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

#include <iostream>
#include "optionsdialog.h"
#include "settingsmanager.h"
#include "unixcommand.h"
#include "wmhelper.h"
#include "strconstants.h"
#include "terminal.h"

#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>

OptionsDialog::OptionsDialog(QWidget *parent) :
  QDialog(parent),

  m_once(false){
  setupUi(this);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

  removeEventFilter(this);
  initialize();
}

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

void OptionsDialog::currentTabChanged(int tabIndex){
  if (tabIndex == 2){
    twTerminal->setFocus();
    twTerminal->setCurrentCell(twTerminal->currentIndex().row(), 0);
  }
}

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

void OptionsDialog::initialize(){
  m_backendHasChanged = false;
  m_iconHasChanged = false;

  initButtonBox();
  initBackendTab();
  initIconTab();
  initTerminalTab();

  tabWidget->setCurrentIndex(0);
}

void OptionsDialog::initButtonBox(){
  buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
  buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

  //buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText(tr("Restore defaults"));
}

void OptionsDialog::initBackendTab()
{
  if (SettingsManager::hasPacmanBackend())
    rbPacman->setChecked(true);
  else
    rbAlpm->setChecked(true);
}

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

void OptionsDialog::initTerminalTab(){
  QStringList terminals = Terminal::getListOfAvailableTerminals();

  if (terminals.count() <= 2)
  {
    twTerminal->setEnabled(false);
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
  //twTerminal->horizontalHeader()->setResizeMode(0, QHeaderView::Fixed);
  //twTerminal->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  //twTerminal->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  //twTerminal->horizontalHeader()->setFixedHeight(22);
  twTerminal->setSelectionBehavior(QAbstractItemView::SelectRows);
  twTerminal->setSelectionMode(QAbstractItemView::SingleSelection);

  connect(twTerminal, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(accept()));

  QStringList slLabels;
  slLabels << tr("Name");
  twTerminal->setHorizontalHeaderLabels(slLabels);

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

void OptionsDialog::accept(){
  QString selectedTerminal;
  bool emptyIconPath = false;

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

  //Set icon...
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

  //Set terminal...
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

  setResult(res);
  QDialog::accept();
}
