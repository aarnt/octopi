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

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QTimer>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QProcess>

#include "constants.h"

class QTextEdit;
class QTextBrowser;
class SearchBar;

namespace utils{

class ProcessWrapper : public QObject
{
  Q_OBJECT

private:
  int m_pidTerminal;
  int m_pidSH;
  int m_pidAUR;
  QProcess *m_process;
  QTimer *m_timer;
  QTimer *m_timerSingleShot;

public:
  explicit ProcessWrapper(QObject *parent = 0);
  void executeCommand(QString command);

signals:
  void startedTerminal();
  void finishedTerminal(int, QProcess::ExitStatus);
  
private slots:
  void onSingleShot();
  void onTimer();
  void onProcessStarted();
};

//TreeView related
QString showFullPathOfItem( const QModelIndex &index );
QList<QModelIndex> * findFileInTreeView( const QString& name, const QStandardItemModel *sim);

//RSS related
QString retrieveDistroNews(bool searchForLatestNews);
QString parseDistroNews();

//QTextBrowser related
bool strInQTextEdit(QTextBrowser *text, const QString& findText);
void positionTextEditCursorAtEnd(QTextEdit *textEdit);
void writeToTextBrowser(QTextBrowser* text, const QString &str, TreatURLLinks treatURLLinks = ectn_TREAT_URL_LINK);

//SearchBar related
void positionInFirstMatch(QTextBrowser *tb, SearchBar *sb);
void searchBarTextChangedInTextBrowser(QTextBrowser *tb, SearchBar *sb, const QString &textToSearch);
void searchBarFindNextInTextBrowser(QTextBrowser *tb, SearchBar *sb);
void searchBarFindPreviousInTextBrowser(QTextBrowser *tb, SearchBar *sb);
void searchBarClosedInTextBrowser(QTextBrowser *tb, SearchBar *sb);

//QDialog related
void positionWindowAtScreenCenter(QWidget *w);

} //namespace utils

#endif // UTILS_H
