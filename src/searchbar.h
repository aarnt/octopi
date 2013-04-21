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
#include <QSyntaxHighlighter>
#include <QApplication>

class SearchBar : public QWidget
{
  Q_OBJECT

private:
  SearchLineEdit *m_searchLineEdit;

protected:
  virtual void paintEvent(QPaintEvent *);
  virtual bool eventFilter(QObject *, QEvent *);

public:
  explicit SearchBar(QWidget *parent = 0);

  void init();
  inline SearchLineEdit *getSearchLineEdit(){ return m_searchLineEdit; }
  inline QString getTextToSearch(){ return m_searchLineEdit->text(); }
  inline bool hasFocus(){ return m_searchLineEdit->hasFocus(); }
  inline void initSearchLineEdit(){ m_searchLineEdit->initStyleSheet(); }

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

class MyHighlighter : public QSyntaxHighlighter{

  Q_OBJECT

private:
  bool m_hasFound;
  QString m_pattern;

public:
  MyHighlighter(QTextEdit *parent, QString pattern):QSyntaxHighlighter(parent){
    m_pattern = pattern;
    m_hasFound = false;
  }

  void setPattern(const QString pattern){ m_pattern = pattern; }
  bool hasFound(){ return m_hasFound; }

protected:
  virtual void highlightBlock(const QString &text)
  {
    if (m_pattern.isEmpty()) return;

    QTextCharFormat myClassFormat;
    myClassFormat.setFontWeight(QFont::Bold);
    myClassFormat.setBackground(QBrush(QColor(Qt::yellow).lighter(130)));
    QString expression = m_pattern;

    int index = text.indexOf(expression, 0, Qt::CaseInsensitive);
    if (index >= 0) m_hasFound = true;
    while (index >= 0) {
      int length = expression.length();
      setFormat(index, length, myClassFormat);
      index = text.indexOf(expression, index + length, Qt::CaseInsensitive);
    }
  }

public slots:
  void rehighlight(){
    m_hasFound = false;
    QSyntaxHighlighter::rehighlight();
  }
};

//This class is a workaround to keep each TextEdit close to its QSyntaxHighlighter object
class SyntaxHighlighterWidget : public QWidget
{
  Q_OBJECT

private:
  MyHighlighter *m_myHighlighter;

public:
  SyntaxHighlighterWidget(QWidget *parent, MyHighlighter *h):
    QWidget(parent){
    m_myHighlighter = h;
    setObjectName("syntaxHighlighterWidget");
    setMaximumSize(0, 0);
    setVisible(false);
  }

  MyHighlighter *getSyntaxHighlighter(){ return m_myHighlighter; }
};

#endif // SEARCHBAR_H
