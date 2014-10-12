/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2014 Alexandre Albuquerque Arnt
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

#include "strconstants.h"
#include "uihelper.h"
#include "terminalselectordialog.h"

#include <QQmlContext>
#include <QtQml/QQmlProperty>
#include <QtQuickWidgets/QQuickWidget>
#include <QQuickItem>
#include <QDialogButtonBox>
#include <QDialog>
#include <QVBoxLayout>

/*
 * This dialog lets user chooses one of the available terminals to use with Octopi
 */

TerminalSelectorDialog::TerminalSelectorDialog(QWidget *parent, QStringList terminalList):
  QDialog(parent)
{
  m_quickWidget = new QQuickWidget(this);
  m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

  setMinimumWidth(290);
  setMaximumWidth(290);
  setMinimumHeight(245);
  setMaximumHeight(245);

  QQmlContext *context = m_quickWidget->rootContext();

  context->setContextProperty("terminalModel", QVariant::fromValue(terminalList));  

  m_quickWidget->setSource(QUrl("qrc:/resources/qml/chooseterminal.qml"));
  setWindowTitle(StrConstants::getChooseATerminal());
  QDialogButtonBox *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(m_quickWidget); //*** This is the magic: QtQuick inside a QWidget!
  mainLayout->addWidget(buttonBox);

  QQuickItem *item = m_quickWidget->rootObject();
  QObject::connect(item, SIGNAL(terminalSelected(int)), this, SLOT(setSelectedTerminalIndex(int)));
  QObject::connect(item, SIGNAL(dialogOK()), this, SLOT(accept()));

  setWindowIcon(IconHelper::getIconTerminal());
  setLayout(mainLayout);
}

void TerminalSelectorDialog::setInitialTerminalIndex(int newIndex)
{
  m_initialTerminalIndex = newIndex;
}

int TerminalSelectorDialog::exec()
{
  QQuickItem *item = m_quickWidget->rootObject();
  QMetaObject::invokeMethod(item, "resetIndex", Q_ARG(QVariant, m_initialTerminalIndex));

  return QDialog::exec();
}
