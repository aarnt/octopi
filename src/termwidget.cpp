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

#include "termwidget.h"
#include "strconstants.h"

#include <QApplication>

/*
 * This class extends some features of QTermWidget and adds some customizations
 */

TermWidget::TermWidget(QWidget *parent):
  QTermWidget(parent)
{
  QFont font = QApplication::font();
  font.setFamily("Monospace");
  font.setPointSize(11);
  //this->setHistorySize(1);
  this->setTerminalFont(font);
  this->setScrollBarPosition(QTermWidget::ScrollBarRight);
  this->changeDir("/");
  this->setColorScheme("WhiteOnBlack");
  this->execute("export TERM=xterm");
  this->execute("clear");

  connect(this, SIGNAL(receivedData(QString)), this, SLOT(parseOutput(QString)));
  connect(this, SIGNAL(termKeyPressed(QKeyEvent*)), this, SLOT(onKeyPressed(QKeyEvent*)));
}

/*
 * Sends the given command to be executed in this terminal
 */
void TermWidget::execute(QString command)
{
  this->sendText(command);
  //After every command, we must send ENTER/RETURN
  this->enter();
}

/*
 * Sends the ENTER/RETURN key to this terminal
 */
void TermWidget::enter()
{
  //This is the ENTER/RETURN key!
  QString enter("\r");
  this->sendText(enter);
}

/*
 * Whenever we find "Press any key to continue..."
 */
void TermWidget::parseOutput(QString str)
{
  if (str.contains(StrConstants::getPressAnyKey()))
  {
    emit onPressAnyKeyToContinue();
  }
  else if (str.contains("quit: command not found") ||
           str.contains("close: command not found"))
  {
    emit onKeyQuit();
  }
}

/*
 * Whenever user hits a key inside this terminal
 */
void TermWidget::onKeyPressed(QKeyEvent *ke)
{
  if ((ke->key() == Qt::Key_Z && ke->modifiers() == Qt::ControlModifier) ||
    (ke->key() == Qt::Key_D && ke->modifiers() == Qt::ControlModifier) ||
    (ke->key() == Qt::Key_C && ke->modifiers() == Qt::ControlModifier))
  {
    emit onCancelControlKey();
  }
  else if (ke->key() == Qt::Key_F11)
  {
    emit onKeyF11();
  }
}
