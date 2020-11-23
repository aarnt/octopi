/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2020 Alexandre Albuquerque Arnt
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

#ifndef PROPERTIESTABWIDGET_H
#define PROPERTIESTABWIDGET_H

#include <QTabWidget>

class TermWidget;
class QStandardItemModel;
class QTextBrowser;
class QTreeView;
class SearchBar;

class PropertiesTabWidget : public QTabWidget
{
  Q_OBJECT

public:
  PropertiesTabWidget(QWidget *parent);

  void initTabInfo();
  void initTabFiles();
  void initTabActions();
  void initTabNews();
  void initTabOutput();
  void initTabHelpUsage();
  void initTabTerminal();

  QStandardItemModel *getModelTransaction() { return m_modelTransaction; };
  void setConsole(TermWidget *console);
  void setHelpUsageText(QString text);  

  inline QTextBrowser* getTextInfo() { return m_textInfo; };
  inline SearchBar* getSearchBarInfo() { return m_searchBarInfo; };

  inline QTreeView* getTvPkgFileList() { return m_tvPkgFileList; };
  inline SearchBar* getSearchBarFiles() { return m_searchBarFiles; };

  inline QTreeView* getTvTransaction() { return m_tvTransaction; };

  inline QTextBrowser* getTextNews() { return m_textNews; };
  inline SearchBar* getSearchBarNews() { return m_searchBarNews; };

  inline QTextBrowser* getTextOutput() { return m_textOutput; };
  inline SearchBar* getSearchBarOutput() { return m_searchBarOutput; };

  inline SearchBar* getSearchBarHelpUsage() { return m_searchBarHelpUsage; };

protected:
  bool eventFilter(QObject* obj, QEvent* event);

private:
  QString m_helpUsageText;
  TermWidget *m_console;
  QStandardItemModel *m_modelTransaction;

  QTextBrowser* m_textInfo;
  SearchBar* m_searchBarInfo;

  QTreeView* m_tvPkgFileList;
  SearchBar* m_searchBarFiles;

  QTreeView* m_tvTransaction;

  QTextBrowser* m_textNews;
  SearchBar* m_searchBarNews;

  QTextBrowser* m_textOutput;
  SearchBar* m_searchBarOutput;

  SearchBar* m_searchBarHelpUsage;

};

#endif // PROPERTIESTABWIDGET_H
