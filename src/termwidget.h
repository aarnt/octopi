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

#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include "qtermwidget5/qtermwidget.h"
#include <QClipboard>

class QWidget;
class QKeyEvent;

class TermWidget : public QTermWidget
{
  Q_OBJECT

private:
  QAction *m_actionZoomIn, *m_actionZoomOut, *m_actionMaximize, *m_actionCopy, *m_actionPaste;
  int m_zoomFactor;

  void paste(QClipboard::Mode);

private slots:
  void parseOutput(QString str);
  void onKeyPressed(QKeyEvent *ke);
  void execContextMenu(const QPoint &);
  void onCopy();
  void onPaste();
  void onZoomIn();
  void onZoomOut();

public:
  explicit TermWidget(QWidget *parent);
  void execute(QString command);
  void enter();

signals:
  void onPressAnyKeyToContinue();
  void onCancelControlKey();
  void onKeyQuit();
  void onKeyF11();
};

#endif // TERMWIDGET_H
