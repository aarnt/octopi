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
#include "ui_searchbar.h"
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
  QWidget(parent),
  ui(new Ui::SearchBar)
{
  ui->setupUi(this);
  init();
}

/*
 * Obligatory initialization code.
 */
void SearchBar::init()
{
  setVisible(false);

  QAction *m_previousAction = new QAction(this);
  QAction *m_nextAction = new QAction(this);

  m_previousAction->setText("< " + tr("Previous"));
  m_previousAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F3));
  m_nextAction->setText(tr("Next") + " >");
  m_nextAction->setShortcut(Qt::Key_F3);

  ui->m_previousButton->setDefaultAction(m_previousAction);
  ui->m_nextButton->setDefaultAction(m_nextAction);

  ui->m_searchLineEdit->setFocus();

  connect(ui->tbClose, SIGNAL(clicked()), this, SLOT(close()));
  connect(ui->m_searchLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
  connect(m_previousAction, SIGNAL(triggered()), this, SIGNAL(findPrevious()));
  connect(m_nextAction, SIGNAL(triggered()), this, SIGNAL(findNext()));
}

/*
 * Whenever the user presses the escape or clicks the close icon...
 */
void SearchBar::close()
{
  hide();
  disconnect(ui->m_searchLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
  ui->m_searchLineEdit->setText("");
  connect(ui->m_searchLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
  emit closed();
}

/*
 * Helper method to clean SearchLineEdit's content
 */
void SearchBar::clear()
{
	ui->m_searchLineEdit->setText("");
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
  if(!ui->m_searchLineEdit->text().isEmpty() && (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return))
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
  ui->m_searchLineEdit->selectAll();
  ui->m_searchLineEdit->setFocus();
}
