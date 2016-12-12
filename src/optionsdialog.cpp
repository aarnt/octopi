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
  connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(restoreDefaults(QAbstractButton*)));
  connect(cbHighlightItems, SIGNAL(toggled(bool)), this, SLOT(toggleSpinBoxHighlightedSearchItems(bool)));
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

  removeEventFilter(this);
  initialize();
  show();
}

void OptionsDialog::paintEvent(QPaintEvent *){
  //This member flag ensures the execution of this code for just ONE time.
  /*if (!m_once){
    QList<QTableWidgetItem *> l = twTerminal->findItems(SettingsManager::getUpdaterMirror(), Qt::MatchExactly);
    if (l.count() == 1){
      twMirror->setCurrentItem(l.at(0));
      twMirror->scrollToItem(l.at(0));
    }
    m_once=true;
  }*/
}

void OptionsDialog::currentTabChanged(int tabIndex){
  if (tabIndex == 2){
    twTerminal->setFocus();
    twTerminal->setCurrentCell(twTerminal->currentIndex().row(), 0);
  }
}

void OptionsDialog::initialize(){
  setModal(true);

  tabWidget->removeTab(0); //REMOVE IT LATER
  tabWidget->setCurrentIndex(1);
  initButtonBox();
  initCheckBoxes();
  initComboPrivilege();
  initFontSlider();
  initGroupBox();
  initTerminalTableWidget();
}

void OptionsDialog::initButtonBox(){
  buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
  buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
  buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText(tr("Restore defaults"));
}

void OptionsDialog::initCheckBoxes(){
  /*cbCloseButtonHidesApp->setChecked(SettingsManager::getWindowCloseHidesApp());
  cbShowPackageTooltip->setToolTip(StrConstants::getNeedsAppRestart());
  cbShowPackageTooltip->setChecked(SettingsManager::getShowPackageTooltip());
  cbShowToolbar->setChecked(SettingsManager::getShowToolBar());
  cbShowStatusBar->setChecked(SettingsManager::getShowStatusBar());
  cbStartHidden->setChecked(SettingsManager::getStartIconified());
  cbHighlightItems->setToolTip(StrConstants::getNeedsAppRestart());
  sbHighlightItems->setToolTip(StrConstants::getNeedsAppRestart());

  int limit = SettingsManager::getHighlightedSearchItems();
  if (limit == 0){
    cbHighlightItems->setChecked(false);
    sbHighlightItems->setEnabled(false);
  }
  else{
    cbHighlightItems->setChecked(true);
    sbHighlightItems->setEnabled(true);
    sbHighlightItems->setValue(limit);
  }*/
}

void OptionsDialog::initFontSlider(){
  /*sliderFont->setToolTip(StrConstants::getNeedsAppRestart());
  lblFontSize->setToolTip(StrConstants::getNeedsAppRestart());
  sliderFont->setMinimum(-4);
  sliderFont->setMaximum(6);
  sliderFont->setSingleStep(1);  
  sliderFont->setPageStep(1);
  sliderFont->setValue(SettingsManager::getFontSizeFactor());*/
}

void OptionsDialog::initComboPrivilege(){
  /*cbPrivilege->addItem(StrConstants::getAutomaticSuCommand());

  if (UnixCommand::hasTheExecutable(ctn_GKSU_1) || (UnixCommand::hasTheExecutable(ctn_GKSU_2)))
    cbPrivilege->addItem(ctn_GKSU_2);
  if (UnixCommand::hasTheExecutable(ctn_KDESU))
    cbPrivilege->addItem(ctn_KDESU);
  if (UnixCommand::hasTheExecutable(ctn_TDESU))
    cbPrivilege->addItem(ctn_TDESU);
  if (UnixCommand::hasTheExecutable(ctn_KTSUSS) && UnixCommand::isKtsussVersionOK())
    cbPrivilege->addItem(ctn_KTSUSS);

  int index = cbPrivilege->findText(SettingsManager::getPrivilegeEscalationTool());
  if (index >= 0)
    cbPrivilege->setCurrentIndex(index);
  else
    cbPrivilege->setCurrentIndex(0);*/
}

void OptionsDialog::initGroupBox(){
  /*rbPkgTools->setChecked(SettingsManager::getUsePkgTools());
  rbSpkg->setChecked(!rbPkgTools->isChecked());
  lblSpkg->setOpenExternalLinks(true);

  QString spkg(tr("A fast and robust tool for Slackware package management") +
               "<br>" + tr("Written by Ondrej Jirman, 2005-2006") +
               "<br>" + tr("Official website: %1").arg("<a href=\"http://spkg.megous.com\">http://spkg.megous.com</a>."));
  lblSpkg->setText("<html lang='utf-8'>" + spkg + "</html>");

  if (!UnixCommand::isSpkgInstalled()){
      rbSpkg->setEnabled(false);
      lblSpkg->setEnabled(false);
  }

  cbUseSilentOuput->setChecked(SettingsManager::getUseSilentActionOutput());*/
}

void OptionsDialog::insertMirrorsInTable(QTextStream *stream, int row){
  /*bool armedSlack = UnixCommand::getSlackArchitecture().contains("arm", Qt::CaseInsensitive);

  while (!stream->atEnd()) {
    QTableWidgetItem *itemCountry = new QTableWidgetItem();
    //Sets the item ReadOnly!
    itemCountry->setFlags(itemCountry->flags() ^ Qt::ItemIsEditable);
    QTableWidgetItem *itemURL = new QTableWidgetItem();
    //Sets the item ReadOnly!
    itemURL->setFlags(itemURL->flags() ^ Qt::ItemIsEditable);

    QString line = stream->readLine().trimmed();
    bool slackwarearmMirror = line.contains("(arm)", Qt::CaseInsensitive);
    if ((slackwarearmMirror && !armedSlack) ||
        (!slackwarearmMirror && armedSlack))
        continue;

    QStringList res;
    QString country, url;

    //If it is not a comment...
    if (!line.isEmpty() && line[0] != '#'){
      res = line.split(",");
      if (res.count() == 2){
        country = res.at(0);
        url = res.at(1).trimmed();
        itemCountry->setText(country);
        itemURL->setText(url);
        twMirror->setItem(row, 0, itemCountry);
        twMirror->setItem(row, 1, itemURL);
        twMirror->setRowHeight(row, 25);
        row++;
      }
    }
  }*/
}

void OptionsDialog::initTerminalTableWidget(){
  this->setMinimumHeight(430);
  this->setMinimumWidth(665);

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
  cbAutoCheckUpdates->setChecked(SettingsManager::getAutomaticCheckUpdates());*/
}

void OptionsDialog::toggleSpinBoxHighlightedSearchItems(bool checkedState){
  sbHighlightItems->setEnabled(checkedState);
}

void OptionsDialog::restoreDefaults(QAbstractButton* button){
  /*if (buttonBox->standardButton(button)==QDialogButtonBox::RestoreDefaults){
    int rep = QMessageBox::question(this, tr("Confirmation"),
                                    tr("Do you really want to restore all default values?"),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (rep == QMessageBox::Yes){
      cbCloseButtonHidesApp->setChecked(false);
      cbShowToolbar->setChecked(true);
      cbShowPackageTooltip->setChecked(true);
      cbShowStatusBar->setChecked(true);
      cbStartHidden->setChecked(false);
      cbHighlightItems->setChecked(true);
      sbHighlightItems->setValue(100);
      cbPrivilege->setCurrentIndex(0);
      sliderFont->setValue(0);

      rbPkgTools->setChecked(true);
      rbSpkg->setChecked(false);
      cbUseSilentOuput->setChecked(true);

      QList<QTableWidgetItem*> l = twMirror->findItems(ctn_SLACKWARE_MIRROR_USA, Qt::MatchExactly);
      if (l.count() == 1){
        twMirror->setCurrentItem(l.at(0));
      }

      cbAutoCheckUpdates->setCheckable(true);
    }
  }*/
}

void OptionsDialog::setFontSize(const int fontSize){
  /*setStyleSheet("QGroupBox, QRadioButton, QCheckBox, QLabel, QTableWidget, "
                "QHeaderView, QPushButton, QComboBox, QSpinBox {"
                "    font-family: \"Verdana\";"
                "    font-size: " + QString::number(fontSize+1) + "px;"
                "}"
                "QTabWidget {"
                "    border: 1px solid gray;}" );

  tabWidget->setFont(QFont("Verdana", SettingsManager::getTodoFontSize() + 2));*/
}

void OptionsDialog::accept(){
  /*QString selectedMirror;
  bool needsAppRestart=false;

  if(twMirror->currentItem())
    selectedMirror = twMirror->item(twMirror->row(twMirror->currentItem()), 1)->text();

  if (SettingsManager::getWindowCloseHidesApp() != cbCloseButtonHidesApp->isChecked()){
    SettingsManager::setWindowCloseHidesApp(cbCloseButtonHidesApp->isChecked());
    qApp->setQuitOnLastWindowClosed(!cbCloseButtonHidesApp->isChecked());
  }

  if (SettingsManager::getShowToolBar() != cbShowToolbar->isChecked())
    SettingsManager::setShowToolBar(cbShowToolbar->isChecked());

  if (SettingsManager::getShowStatusBar() != cbShowStatusBar->isChecked())
    SettingsManager::setShowStatusBar(cbShowStatusBar->isChecked());

  if (SettingsManager::getShowPackageTooltip() != cbShowPackageTooltip->isChecked()){
    SettingsManager::setShowPackageTooltip(cbShowPackageTooltip->isChecked());
    needsAppRestart = true;
  }

  if (SettingsManager::getStartIconified() != cbStartHidden->isChecked())
    SettingsManager::setStartIconified(cbStartHidden->isChecked());

  int newValue=-1;

  if (cbHighlightItems->isChecked())
    newValue = sbHighlightItems->value();
  else
    newValue = 0;

  if (newValue != SettingsManager::getHighlightedSearchItems()){
    SettingsManager::setHighlightedSearchItems(newValue);
    needsAppRestart = true;
  }

  if (SettingsManager::getUpdaterMirror() != selectedMirror)
    SettingsManager::setUpdaterMirror(selectedMirror);

  if (SettingsManager::getAutomaticCheckUpdates() != cbAutoCheckUpdates->isChecked())
    SettingsManager::setAutomaticCheckUpdates(cbAutoCheckUpdates->isChecked());

  if (SettingsManager::getFontSizeFactor() != sliderFont->value()){
    SettingsManager::setFontSizeFactor(sliderFont->value());
    needsAppRestart = true;
  }

  if (SettingsManager::getUsePkgTools() != rbPkgTools->isChecked())
    SettingsManager::setUsePkgTools(rbPkgTools->isChecked());

  if (SettingsManager::getUseSilentActionOutput() != cbUseSilentOuput->isChecked())
    SettingsManager::setUseSilentActionOutput(cbUseSilentOuput->isChecked());

  if (SettingsManager::getPrivilegeEscalationTool() != cbPrivilege->currentText()){
    if (cbPrivilege->currentIndex() != 0)
      SettingsManager::setPrivilegeEscalationTool(cbPrivilege->currentText());
    else
      SettingsManager::setPrivilegeEscalationTool(ctn_AUTOMATIC);
  }

  if (needsAppRestart){
    QMessageBox::information(this, StrConstants::getAttention(), StrConstants::getWarnNeedsAppRestart());
  }

  QDialog::accept();*/
}

/*void OptionsDialog::testMirrors(){
  for (int r=0; r<twMirror->rowCount(); r++){
    QTableWidgetItem *item = twMirror->item(r, 1);
    QString mirror = item->text();
    QProcess proc;

#if QT_VERSION >= 0x040600
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "us_EN");
    proc.setProcessEnvironment(env);
#endif

    proc.start("curl " + mirror);
    proc.waitForFinished();

    QString res = proc.readAllStandardOutput();

    if (res.indexOf("slackware-13.37") >= 0 || res.indexOf("armedslack-13.37") >= 0){
      std::cout << "Mirror " << mirror.toAscii().data() << " is OK." << std::endl;
    }
    else{
      std::cout << "Mirror " << mirror.toAscii().data() << " IS PROBLEMATIC!!!" << std::endl;
    }

    proc.close();
  }
}*/
