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

MultiSelectionDialog::MultiSelectionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::MultiSelectionDialog)
{
  ui->setupUi(this);
  ui->twDepPackages->setColumnWidth(0, 150); //Package name
  ui->twDepPackages->setColumnWidth(1, 315); //Package description
  ui->twDepPackages->horizontalHeader()->setResizeMode( QHeaderView::Fixed );
  ui->twDepPackages->setColumnWidth(2, 0); //Package repository
  ui->twDepPackages->horizontalHeader()->setDefaultAlignment( Qt::AlignLeft );
  ui->twDepPackages->horizontalHeaderItem(0)->setText(StrConstants::getName());
  ui->twDepPackages->horizontalHeaderItem(1)->setText(StrConstants::getDescription());

  QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
  QPushButton *cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
  connect(okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | Qt::Dialog |
                 Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
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
}

/*
 * Retrieve the selected optional dep packages
 */
QStringList MultiSelectionDialog::getSelectedPackages()
{
  QStringList result;
  int lastRow=0;
  QString name;
  QString repository;

  foreach (QTableWidgetItem * item, ui->twDepPackages->selectedItems())
  {
    if (lastRow != item->row())
    {
      result.append(repository + "/" + name);
    }

    if (item->column() == 0) name = item->text();
    else if (item->column() == 2) repository = item->text();

    lastRow = item->row();
  }

  if(!name.isEmpty() && !repository.isEmpty())
  {
    result.append(repository + "/" + name);
  }

  return result;
}

void MultiSelectionDialog::reject()
{
  done(QDialogButtonBox::Cancel);
}

void MultiSelectionDialog::slotOk()
{
  done(QDialogButtonBox::Ok);
}
