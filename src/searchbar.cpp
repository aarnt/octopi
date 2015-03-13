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

#include "searchbar.h"
#include "searchlineedit.h"
#include "uihelper.h"
#include <QAction>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStyleOption>
#include <QPainter>
#include <QKeyEvent>
#include <QEvent>
#include <QSpacerItem>

/*
 * The QWidget that holds the SearchLineEdit control and has that firefox's search sexy look!
 */

SearchBar::SearchBar(QWidget *parent) :
  QWidget(parent)
{
  init();
}

/*
 * Obligatory initialization code.
 */
void SearchBar::init()
{
  setVisible(false);
  setObjectName("searchbar");
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setSpacing(0);
  layout->setMargin(4);

  setStyleSheet("QWidget#searchbar{"
                "border-top-width: .6px;"
                "border-top-style: solid;"
                "border-top-color: darkgray;}");

  m_searchLineEdit = new SearchLineEdit(this);
  m_searchLineEdit->setMinimumWidth(300);
  QToolButton *m_previousButton = new QToolButton(this);
  QToolButton *m_nextButton = new QToolButton(this);
  m_previousButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_nextButton->setToolButtonStyle(Qt::ToolButtonTextOnly);

  QAction *m_previousAction = new QAction(this);
  QAction *m_nextAction = new QAction(this);

  m_previousAction->setText("< " + tr("Previous"));
  m_previousButton->setAutoRaise(true);
  m_previousAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F3));
  m_nextAction->setText(tr("Next") + " >");
  m_nextButton->setAutoRaise(true);
  m_nextAction->setShortcut(Qt::Key_F3);
  m_previousButton->setDefaultAction(m_previousAction);
  m_nextButton->setDefaultAction(m_nextAction);

  QToolButton *tbClose = new QToolButton();
  tbClose->setIcon(IconHelper::getIconClose());

  tbClose->setAutoRaise(true);
  tbClose->setStyleSheet("QToolButton{ font-size: 16px; font-family: verdana; border-radius: 4px; } "
                         "QToolButton:hover{ background-color: palette(light); }"
                         "QToolButton::pressed{ background-color: palette(mid); }");

  tbClose->setToolTip(tr("Close"));
  tbClose->setShortcut(Qt::Key_Escape);

  layout->addWidget(tbClose, 1, Qt::AlignLeft);
  layout->addSpacing(3);
  layout->addWidget(m_searchLineEdit, 0, Qt::AlignLeft);
  layout->addSpacing(2);
  layout->addWidget(m_previousButton, 1, Qt::AlignLeft);
  layout->addWidget(m_nextButton, 20, Qt::AlignLeft);

  setLayout(layout);
  m_searchLineEdit->setFocus();

  connect(tbClose, SIGNAL(clicked()), this, SLOT(close()));
  connect(m_searchLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
  connect(m_previousAction, SIGNAL(triggered()), this, SIGNAL(findPrevious()));
  connect(m_nextAction, SIGNAL(triggered()), this, SIGNAL(findNext()));
}

/*
 * Whenever the user presses the escape or clicks the close icon...
 */
void SearchBar::close()
{
  hide();
  m_searchLineEdit->setText("");
  emit closed();
}

/*
 * Helper method to clean SearchLineEdit's content
 */
void SearchBar::clear()
{
  m_searchLineEdit->setText("");
}

/*
 * Overriden in order to get stylesheets working in QWidget derived classes
 */
void SearchBar::paintEvent(QPaintEvent *)
{
  QStyleOption styleOption;
  styleOption.init(this);
  QPainter painter(this);
  style()->drawPrimitive(QStyle::PE_Widget, &styleOption, &painter, this);
}

/*
 * Overriden in order to respond to ENTER and RETURN key presses to find next itens
 */
void SearchBar::keyPressEvent(QKeyEvent *ke)
{
  if(!m_searchLineEdit->text().isEmpty() && (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return))
  {
    findNext();
  }
}

/*
 * Whenever SearchBar needs to be brought to UI...
 */
void SearchBar::show()
{
  setVisible(true);
  m_searchLineEdit->selectAll();
  m_searchLineEdit->setFocus();
}
