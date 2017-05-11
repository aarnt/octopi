/*
* This file is part of Octopi, an open-source GUI for pacman.
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
*/

#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include "searchlineedit.h"
#include <iostream>

#include <QWidget>
#include <QApplication>
#include <QTextEdit>

namespace Ui {
class SearchBar;
}

class SearchBar : public QWidget
{
  Q_OBJECT

private:
  Ui::SearchBar *ui;

protected:
  virtual void paintEvent(QPaintEvent *);
  virtual void keyPressEvent(QKeyEvent *);

public:
  static const char NAME[];
  explicit SearchBar(QWidget *parent = 0);

  void init();
  SearchLineEdit *getSearchLineEdit();
  QString getTextToSearch();
  bool hasFocus();
  void setFocus(Qt::FocusReason reason);
  void initSearchLineEdit();

signals:
  void closed();
  void textChanged(QString text);
  void findNext();
  void findPrevious();

public slots:
  void show();
  void close();

  void clear();

};

#endif // SEARCHBAR_H
