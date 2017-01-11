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

#ifndef OUTPUTDIALOG_H
#define OUTPUTDIALOG_H

#include "../../src/constants.h"

#include <QDialog>
#include <QProcess>
#include <QFrame>

class PacmanExec;
class QString;
class QTextBrowser;
class QVBoxLayout;
class QProgressBar;
class SearchBar;
class QWidget;
class QCloseEvent;
class QKeyEvent;

class OutputDialog : public QDialog
{
  Q_OBJECT

  Q_PROPERTY(QFrame::Shape frameShape READ frameShape WRITE setFrameShape USER true)

private:
  QTextBrowser *m_textBrowser;
  QProgressBar *m_progressBar;
  QVBoxLayout *m_mainLayout;
  PacmanExec *m_pacmanExec;
  SearchBar *m_searchBar;
  bool m_upgradeRunning;
  bool m_debugInfo;

  void init();

  void doSystemUpgrade();
  void positionTextEditCursorAtEnd();
  bool textInTabOutput(const QString& findText);
  void writeToTabOutput(const QString &msg, TreatURLLinks treatURLLinks = ectn_TREAT_URL_LINK);

private slots:
  void onPencertange(int percentage);
  void onWriteOutput(const QString &output);
  void pacmanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  //SearchBar slots
  void onSearchBarTextChanged(QString strToSearch);
  void onSearchBarClosed();
  void onSearchBarFindNext();
  void onSearchBarFindPrevious();

protected:
  virtual void closeEvent(QCloseEvent * event);
  virtual void keyPressEvent(QKeyEvent * ke);

public:
  explicit OutputDialog(QWidget *parent = 0);
  void setDebugMode(bool newValue);
  QFrame::Shape frameShape();

public slots:
  void show();
  void reject();
  void setFrameShape(QFrame::Shape shape);
};

#endif // OUTPUTDIALOG_H
