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
#include "settingsmanager.h"

#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QAbstractButton>
#include <QRegularExpression>
//#include <QDebug>

/*
 * This class extends some features of QTermWidget (LXQt's QTerminal widget) and adds some customizations
 */

TermWidget::TermWidget(QWidget *parent):
  QTermWidget(parent)
{
  setHistorySize(6000);
  setScrollBarPosition(QTermWidget::ScrollBarRight);
  setContextMenuPolicy(Qt::CustomContextMenu);

  setColorScheme(SettingsManager::getTerminalColorScheme());
  QFont f = QApplication::font();
  f.setFamily(SettingsManager::getTerminalFontFamily());
  f.setPointSizeF(SettingsManager::getTerminalFontPointSize());
  setTerminalFont(f);

  m_actionZoomIn = new QAction(this);
  m_actionZoomIn->setText(StrConstants::getZoomIn());
  m_actionZoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
  connect(m_actionZoomIn, &QAction::triggered, this, &TermWidget::onZoomIn);

  m_actionZoomOut = new QAction(this);
  m_actionZoomOut->setText(StrConstants::getZoomOut());
  m_actionZoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
  connect(m_actionZoomOut, &QAction::triggered, this, &TermWidget::onZoomOut);

  m_actionMaximize = new QAction(this);
  m_actionMaximize->setText(StrConstants::getMaximize());
  m_actionMaximize->setShortcut(QKeySequence(Qt::Key_F11));
  connect(m_actionMaximize, &QAction::triggered, this, &TermWidget::onKeyF11);

  m_actionCopy = new QAction(this);
  m_actionCopy->setText(StrConstants::getCopy());
  connect(m_actionCopy, &QAction::triggered, this, &TermWidget::onCopy);

  m_actionPaste = new QAction(this);
  m_actionPaste->setText(StrConstants::getPaste());
  connect(m_actionPaste, &QAction::triggered, this, &TermWidget::onPaste);

  addAction(m_actionZoomIn);
  addAction(m_actionZoomOut);
  addAction(m_actionMaximize);

  connect(this, SIGNAL(receivedData(QString)), this, SLOT(parseOutput(QString)));
  connect(this, SIGNAL(termKeyPressed(QKeyEvent*)), this, SLOT(onKeyPressed(QKeyEvent*)));
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(execContextMenu(const QPoint&)));

  m_zoomFactor = SettingsManager::getConsoleFontSize();
  if (m_zoomFactor != 0)
  {
    if (m_zoomFactor < 0)
    {
      m_zoomFactor=-m_zoomFactor;
      for (int c=0; c<m_zoomFactor; c++)
      {
        emit zoomOut();
      }
    }
    else {
      for (int c=0; c<m_zoomFactor; c++)
      {
        emit zoomIn();
      }
    }
  }
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
  QString enter(QStringLiteral("\r"));
  this->sendText(enter);
}

/*
 * Whenever we find "Press any key to continue..."
 */
void TermWidget::parseOutput(QString str)
{
  //qDebug() << "terminal: " << str << Qt::endl;

  if ((str == StrConstants::getPressAnyKey() ||
       (str.contains(QRegularExpression(QStringLiteral("\\r*\\n*") + StrConstants::getPressAnyKey())) &&
       (!str.contains(QStringLiteral("echo '") + StrConstants::getPressAnyKey())))) ||

      str.contains(StrConstants::getSuspiciousExecutionDetected()) ||
      str.contains(StrConstants::getSuspiciousTransactionDetected()) ||
      str.contains(StrConstants::getCouldNotAttachToParent())
      )
  {
    emit onPressAnyKeyToContinue();
  }
  else if (str.contains(QLatin1String("quit: command not found")) ||
           str.contains(QLatin1String("close: command not found")))
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
}

/*
 * Creates terminal's custom context menu with some options to configure its behaviour
 */
void TermWidget::execContextMenu(const QPoint & pos)
{
  QMenu menu;
  menu.addAction(m_actionZoomIn);
  menu.addAction(m_actionZoomOut);
  menu.addAction(m_actionCopy);

  if (qApp->clipboard()->text().isEmpty())
    m_actionPaste->setEnabled(false);
  else {
    m_actionPaste->setEnabled(true);
  }

  menu.addAction(m_actionPaste);
  menu.addAction(m_actionMaximize);
  menu.exec(mapToGlobal(pos));
}

/*
 * Whenever user copies text to the clipboard
 */
void TermWidget::onCopy()
{
  QApplication::clipboard()->setText(this->selectedText());
}

/*
 * Calls paste() code
 */
void TermWidget::onPaste()
{
  paste(QClipboard::Clipboard);
}

/*
 * Whenever user selects zoomIn, we must increment zoom factor!
 */
void TermWidget::onZoomIn()
{
  m_zoomFactor++;
  emit zoomIn();
  if (m_zoomFactor != 0) SettingsManager::setConsoleFontSize(m_zoomFactor);
}

/*
 * Whenever user selects zoomIn, we must decrement zoom factor!
 */
void TermWidget::onZoomOut()
{
  m_zoomFactor--;
  emit zoomOut();
  if (m_zoomFactor != 0) SettingsManager::setConsoleFontSize(m_zoomFactor);
}

/*
 * Paste code extracted from project "lxqt/qterminal"
 */
void TermWidget::paste(QClipboard::Mode mode)
{
  // Paste Clipboard by simulating keypress events
  QString text = QApplication::clipboard()->text(mode);

  if ( ! text.isEmpty() )
  {
    text.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    text.replace(QLatin1Char('\n'), QLatin1Char('\r'));
    QString trimmedTrailingNl(text);
    trimmedTrailingNl.replace(QRegularExpression(QStringLiteral("\\r+$")), QString());
    bracketText(text);
    sendText(text);
    qApp->clipboard()->clear();
  }
}
