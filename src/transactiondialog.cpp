/***************************************************************************
 *   Copyright (C) 2013 Manuel Tortosa <manutortosa@chakra-project.org>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#include "transactiondialog.h"
#include "uihelper.h"
#include "strconstants.h"
#include "unixcommand.h"

#include <QMessageBox>
#include <QPushButton>
#include <QDialog>

/*
 * This is the dialog used to show the transaction summary
 */

TransactionDialog::TransactionDialog(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::TransactionDialog)
{
  ui->setupUi(this);

  ui->actionRunInTerminal->setIcon(IconHelper::getIconTerminal());

  m_runInTerminalButton =
      new QPushButton(IconHelper::getIconTerminal(), StrConstants::getRunInTerminal());
  ui->buttonBox->addButton(m_runInTerminalButton, QDialogButtonBox::AcceptRole);

  QPushButton *yesButton = ui->buttonBox->button(QDialogButtonBox::Yes);
  QPushButton *noButton = ui->buttonBox->button(QDialogButtonBox::No);

  noButton->setFocus();

  connect(m_runInTerminalButton, SIGNAL(clicked()), this,
          SLOT(slotRunInTerminal()));
  connect(yesButton, SIGNAL(clicked()), this, SLOT(slotYes()));
  connect(noButton, SIGNAL(clicked()), this, SLOT(reject()));

  setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | Qt::Dialog |
                 Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

  if(UnixCommand::getLinuxDistro()==ectn_CHAKRA || UnixCommand::getLinuxDistro()==ectn_ARCHBSD)
  {
    removeYesButton();
  }
}

void TransactionDialog::setText(const QString text)
{
  ui->text->setText(text);
}

void TransactionDialog::setInformativeText(const QString text)
{
  ui->informativeText->setText(text);
}

void TransactionDialog::setDetailedText(const QString detailedtext)
{
  ui->detailedText->setText(detailedtext);

  //We must search for a 'pacman-version-number' pkg to force terminal upgrade use
  if (detailedtext.contains(QRegExp("pacman-[0-9]+")))
  {
    removeYesButton();
  }
}

/*
 * Useful when you don't want to rely on pacman's -noconfirm option
 */
void TransactionDialog::removeYesButton()
{
  QPushButton *yesButton = ui->buttonBox->button(QDialogButtonBox::Yes);
  ui->buttonBox->removeButton(yesButton);
}

void TransactionDialog::reject()
{
  done(QDialogButtonBox::No);
}

void TransactionDialog::slotRunInTerminal()
{
  done(QDialogButtonBox::AcceptRole);
}

void TransactionDialog::slotYes()
{
  done(QDialogButtonBox::Yes);
}
