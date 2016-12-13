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

OptionsDialog::OptionsDialog(QWidget *parent) :
  QDialog(parent),
  m_once(false){

  setAttribute(Qt::WA_DeleteOnClose);
  setupUi(this);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

  removeEventFilter(this);
  initialize();
  show();
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

void OptionsDialog::initialize(){
  setModal(true);

  initButtonBox();
  initTerminalTab();

  tabWidget->setCurrentIndex(0);
}

void OptionsDialog::initButtonBox(){
  buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
  buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

  //buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText(tr("Restore defaults"));
}

void OptionsDialog::initIconTab()
{
  //
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
  twTerminal->setColumnCount(2);
  twTerminal->setColumnWidth(0, 140);
  twTerminal->setColumnWidth(1, 460);
  twTerminal->verticalHeader()->hide();
  //twTerminal->horizontalHeader()->setResizeMode(0, QHeaderView::Fixed);
  //twTerminal->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  //twTerminal->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  twTerminal->horizontalHeader()->setFixedHeight(22);
  twTerminal->setSelectionBehavior(QAbstractItemView::SelectRows);
  twTerminal->setSelectionMode(QAbstractItemView::SingleSelection);

  connect(twTerminal, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(accept()));

  QStringList slLabels;
  slLabels << tr("Name");
  twTerminal->setHorizontalHeaderLabels(slLabels);

  while (row < (terminals.count()-1))
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

void OptionsDialog::initTerminalTableWidget(){
  /*twMirror->setShowGrid(false);
  twMirror->setColumnCount(2);
  twMirror->setColumnWidth(0, 140);
  twMirror->setColumnWidth(1, 460);
  twMirror->verticalHeader()->hide();
  twMirror->horizontalHeader()->setResizeMode(0, QHeaderView::Fixed);
  twMirror->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  twMirror->verticalHeader()->setResizeMode(QHeaderView::Fixed);
  twMirror->horizontalHeader()->setFixedHeight(22);
  twMirror->setSelectionBehavior(QAbstractItemView::SelectRows);
  twMirror->setSelectionMode(QAbstractItemView::SingleSelection);

  connect(twMirror, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(accept()));

  QStringList slLabels;
  slLabels << tr("Country") << tr("URL");
  twMirror->setHorizontalHeaderLabels(slLabels);

  QFile mirrorsFile(":/resources/updater/mirrors2.txt");
  int row=0;

  if (!mirrorsFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

  //First we populate the built-in mirror list
  QTextStream in(&mirrorsFile);
  QString s;

  bool armedSlack = UnixCommand::getSlackArchitecture().contains("arm", Qt::CaseInsensitive);

  while (!in.atEnd()) {
    s = in.readLine().trimmed();

    bool slackwarearmMirror = s.contains("(arm)", Qt::CaseInsensitive);

    if ((slackwarearmMirror && !armedSlack) ||
        (!slackwarearmMirror && armedSlack))
        continue;

    if (!s.isEmpty() && s[0] != '#') //If it is not a comment...
      row++;
  }

  //Then, we populate the User defined mirror list, if any.
  int rowUser = 0;
  QFile userMirrorsFile(StrConstants::getUserMirrorsFile());
  QTextStream in2(&userMirrorsFile);

  if (userMirrorsFile.exists()){
    if (userMirrorsFile.open(QIODevice::ReadOnly | QIODevice::Text)){
      QString s2;
      QStringList slAux;

      while (!in2.atEnd()) {
        s2 = in2.readLine().trimmed();

        slAux = s2.split(",");
        if(slAux.count() != 2)
          continue;

        s2.remove(QRegExp("\n"));
        if (!s2.isEmpty() && s2[0] != '#') //If it is not a comment...
          rowUser++;
      }
    }
  }

  twMirror->setRowCount(row + rowUser);

  in.seek(0);
  if (userMirrorsFile.isOpen())
    in2.seek(0);

  insertMirrorsInTable(&in);

  if (userMirrorsFile.isOpen())
    insertMirrorsInTable(&in2, row);

  mirrorsFile.close();
  if (userMirrorsFile.isOpen())
    userMirrorsFile.close();

  twMirror->sortByColumn(0, Qt::AscendingOrder);
  */
}

void OptionsDialog::accept(){
  QString selectedTerminal;

  //Set icons...




  //Set terminal...
  if(twTerminal->currentItem())
    selectedTerminal = twTerminal->item(twTerminal->row(twTerminal->currentItem()), 1)->text();

  if (SettingsManager::getTerminal() != selectedTerminal)
    SettingsManager::setTerminal(selectedTerminal);


  QDialog::accept();
}
