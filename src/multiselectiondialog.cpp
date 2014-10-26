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

#include "multiselectiondialog.h"
#include "ui_multiselectiondialog.h"
#include "strconstants.h"

#include <QPushButton>
#include <QKeyEvent>

/*
 * Dialog which appears whenever users have to choose optional packages to install/remove
 */

MultiSelectionDialog::MultiSelectionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::MultiSelectionDialog)
{
  m_actionIsToCheck = true;

  QStringList hhl;
  hhl << StrConstants::getName() << StrConstants::getDescription();

  ui->setupUi(this);
  ui->twDepPackages->setColumnCount(3);
  ui->twDepPackages->setHorizontalHeaderLabels(hhl);

  ui->twDepPackages->setColumnWidth(0, 150); //Package name
  ui->twDepPackages->setColumnWidth(1, 385); //Package description
  //ui->twDepPackages->horizontalHeader()->setResizeMode( QHeaderView::Fixed );
  ui->twDepPackages->setColumnWidth(2, 0); //Package repository
  ui->twDepPackages->horizontalHeader()->setDefaultAlignment( Qt::AlignLeft );
  ui->twDepPackages->setToolTip(StrConstants::getPressCtrlAToSelectAll());

  QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
  QPushButton *cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
  connect(okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | Qt::Dialog |
                 Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

  ui->twDepPackages->installEventFilter(this);
}

MultiSelectionDialog::~MultiSelectionDialog()
{
  delete ui;
}

/*
 * Adds a package item to the treeWidget
 */
void MultiSelectionDialog::addPackageItem(const QString &name, const QString &description,
                                          const QString &repository)
{
  ui->twDepPackages->setRowCount(ui->twDepPackages->rowCount() + 1);
  QTableWidgetItem *itemName = new QTableWidgetItem(name);
  QTableWidgetItem *itemDescription = new QTableWidgetItem(description);
  QTableWidgetItem *itemRepository = new QTableWidgetItem(repository);

  int currentRow = ui->twDepPackages->rowCount()-1;
  ui->twDepPackages->setItem(currentRow, 0, itemName);
  ui->twDepPackages->setItem(currentRow, 1, itemDescription);
  ui->twDepPackages->setItem(currentRow, 2, itemRepository);
  itemName->setCheckState(Qt::Unchecked);
}

/*
 * Retrieve the selected optional dep packages
 */
QStringList MultiSelectionDialog::getSelectedPackages()
{
  QStringList result;
  QString name;
  QString repository;

  for(int row=0; row < ui->twDepPackages->rowCount(); row++)
  {
    if (ui->twDepPackages->item(row, 0)->checkState() == Qt::Checked)
    {
      name = ui->twDepPackages->item(row, 0)->text();
      repository = ui->twDepPackages->item(row, 2)->text();
      result.append(repository + "/" + name);
    }
  }

  return result;
}

/*
 * Helper to select all packages available
 */
void MultiSelectionDialog::setAllSelected()
{
  m_actionIsToCheck = false;

  for(int row=0; row < ui->twDepPackages->rowCount(); row++)
  {
    ui->twDepPackages->item(row, 0)->setCheckState(Qt::Checked);
  }
}

/*
 * This Event method is called whenever the user presses a key
 */
bool MultiSelectionDialog::eventFilter(QObject *obj, QEvent *evt)
{
  if(obj->objectName() == ui->twDepPackages->objectName())
  {
    if (evt->type() == QKeyEvent::KeyPress)
    {
      QKeyEvent *ke = static_cast<QKeyEvent*>(evt);
      if (ke->key() == Qt::Key_A && ke->modifiers() == Qt::ControlModifier)
      {        
        for(int row=0; row < ui->twDepPackages->rowCount(); row++)
        {
          if (m_actionIsToCheck)
          {
            ui->twDepPackages->item(row, 0)->setCheckState(Qt::Checked);
          }
          else
          {
            ui->twDepPackages->item(row, 0)->setCheckState(Qt::Unchecked);
          }
        }

        m_actionIsToCheck = !m_actionIsToCheck;

        return true;
      }      
    }
  }

  return false;
}

/*
 * Slot called when this dialog is cancelled (Cancel or ESC)
 */
void MultiSelectionDialog::reject()
{
  done(QDialogButtonBox::Cancel);
}

/*
 * Slot called when user presses OK button
 */
void MultiSelectionDialog::slotOk()
{
  done(QDialogButtonBox::Ok);
}
