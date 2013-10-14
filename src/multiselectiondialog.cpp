#include "multiselectiondialog.h"
#include "ui_multiselectiondialog.h"
#include "strconstants.h"
#include <iostream>

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
}

MultiSelectionDialog::~MultiSelectionDialog()
{
  delete ui;
}

/*
 * Adds a package item to the treeWidget
 */
void MultiSelectionDialog::addPackageItem(const QString &name, const QString &description, const QString &repository)
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

    //std::cout << item->row() << " : " << item->column() << std::endl;
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
