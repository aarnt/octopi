/***************************************************************************
 *   Copyright (C) 20013 Manuel Tortosa <manutortosa@chakra-project.org>   *
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

#ifndef TRANSACTIONDIALOG_H
#define TRANSACTIONDIALOG_H

#include "ui_transactiondialog.h"
#include <QDialog>

const int ctn_RUN_IN_TERMINAL(328);
class QPushButton;

class TransactionDialog : public QDialog
{
  Q_OBJECT

public:
  TransactionDialog(QWidget * parent);
  virtual ~TransactionDialog(){}

  void setText(const QString text);
  void setInformativeText(const QString text);
  void setDetailedText(const QString detailedtext);

  void removeYesButton();

public slots:
  virtual void reject();

private slots:
  void slotRunInTerminal();
  void slotYes();

private:
  Ui::TransactionDialog *ui;
  QPushButton *m_runInTerminalButton;
};

#endif // TRANSACTIONDIALOG_H
