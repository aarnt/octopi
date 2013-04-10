/*
* This file is part of QTGZManager, an open-source GUI for Slackware pkgtools.
* Copyright (C) 2006  Alexandre Albuquerque Arnt
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
* Contact e-mail : Alexandre Albuquerque Arnt <qtgzmanager@gmail.com>
* Program URL   : http://jtgzmanager.sf.net
*
*/

#include "searchbar.h"
#include "searchlineedit.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QStyleOption>
#include <QPainter>
#include <QKeyEvent>
#include <QEvent>
#include <QSpacerItem>

SearchBar::SearchBar(QWidget *parent) :
  QWidget(parent)
{
  init();
}

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
  QToolButton *m_previousButton = new QToolButton();
  QToolButton *m_nextButton = new QToolButton();

  m_previousButton->setText("< " + tr("Previous"));
  m_previousButton->setAutoRaise(true);
  m_nextButton->setText(tr("Next") + " >");
  m_nextButton->setAutoRaise(true);

  QToolButton *tbClose = new QToolButton();
  tbClose->setIcon(QIcon(":/resources/images/window_close.png"));

  tbClose->setAutoRaise(true);
  tbClose->setStyleSheet("QToolButton{ font-size: 16px; font-family: verdana; border-radius: 4px; } "
                         "QToolButton:hover{ background-color: palette(light); }"
                         "QToolButton::pressed{ background-color: palette(mid); }");

  tbClose->setToolTip(tr("Close"));

  layout->addWidget(tbClose, 1, Qt::AlignLeft);
  layout->addSpacing(3);
  layout->addWidget(m_searchLineEdit, 0, Qt::AlignLeft);
  layout->addSpacing(2);
  layout->addWidget(m_previousButton, 1, Qt::AlignLeft);
  layout->addWidget(m_nextButton, 20, Qt::AlignLeft);

  setLayout(layout);
  m_searchLineEdit->installEventFilter(this);
  m_searchLineEdit->setFocus();

  connect(tbClose, SIGNAL(clicked()), this, SLOT(close()));
  connect(m_searchLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
  connect(m_previousButton, SIGNAL(clicked()), this, SIGNAL(findPreviousButtonClicked()));
  connect(m_nextButton, SIGNAL(clicked()), this, SIGNAL(findNextButtonClicked()));
}

void SearchBar::close()
{
  hide();
  m_searchLineEdit->setText("");
  emit closed();
}

//It's necessary to override this event in order to get stylesheets working in QWidget derived classes.
void SearchBar::paintEvent(QPaintEvent *)
{
  QStyleOption styleOption;
  styleOption.init(this);
  QPainter painter(this);
  style()->drawPrimitive(QStyle::PE_Widget, &styleOption, &painter, this);
}

bool SearchBar::eventFilter(QObject *obj, QEvent *event)
{
  if (obj->objectName() == m_searchLineEdit->objectName()){
    if (event->type() == QEvent::KeyPress)   {
      QKeyEvent *ke = static_cast<QKeyEvent*>(event);
      if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
        emit findNext();
      else if (ke->key() == Qt::Key_Escape)
        close();
    }
  }

  return false;
}

void SearchBar::show()
{
  setVisible(true);

  m_searchLineEdit->selectAll();
  m_searchLineEdit->setFocus();
}
