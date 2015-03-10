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

/*
 * This is MainWindow's searchbar slots related code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "searchbar.h"

#include <QTextBrowser>

/*
 * Every time the user changes the text to search inside a textBrowser...
 */
void MainWindow::searchBarTextChangedInTextBrowser(const QString textToSearch)
{
  qApp->processEvents();
  QTextBrowser *tb = ui->twProperties->currentWidget()->findChild<QTextBrowser*>("textBrowser");
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (tb){
    static int limit = 100;

    SearchBar *sb = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");
    if (textToSearch.isEmpty() || textToSearch.length() < 2){
      sb->getSearchLineEdit()->initStyleSheet();
      tb->setExtraSelections(extraSelections);
      QTextCursor tc = tb->textCursor();
      tc.clearSelection();
      tb->setTextCursor(tc);
      tb->moveCursor(QTextCursor::Start);
      if (sb && sb->isHidden()) tb->setFocus();
      return;
    }

    if (textToSearch.length() < 2) return;

    tb->setExtraSelections(extraSelections);
    tb->moveCursor(QTextCursor::Start);
    QColor color = QColor(Qt::yellow).lighter(130);

    while(tb->find(textToSearch)){
      QTextEdit::ExtraSelection extra;
      extra.format.setBackground(color);
      extra.cursor = tb->textCursor();
      extraSelections.append(extra);

      if (limit > 0 && extraSelections.count() == limit)
        break;
    }

    if (extraSelections.count()>0){
      tb->setExtraSelections(extraSelections);
      tb->setTextCursor(extraSelections.at(0).cursor);
      QTextCursor tc = tb->textCursor();
      tc.clearSelection();
      tb->setTextCursor(tc);
      positionInFirstMatch();
    }
    else sb->getSearchLineEdit()->setNotFoundStyle();
  }
}

/*
 * Every time the user changes the text to search inside a treeView...
 */
void MainWindow::searchBarTextChangedInTreeView(const QString textToSearch)
{
  m_foundFilesInPkgFileList->clear();
  m_indFoundFilesInPkgFileList = 0;

  QTreeView *tvPkgFileList =
    ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
  if (tvPkgFileList)
  {
    QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvPkgFileList->model());
    if (!sim) return;
    SearchBar *sb = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");

    if (!sb)
      return;
    else
      sb->getSearchLineEdit()->initStyleSheet();

    if (textToSearch.isEmpty()) return;

    m_foundFilesInPkgFileList = utils::findFileInTreeView(textToSearch, sim);

    if (m_foundFilesInPkgFileList->count() > 0)
    {
      tvPkgFileList->setCurrentIndex(m_foundFilesInPkgFileList->at(0));
      tvPkgFileList->scrollTo(m_foundFilesInPkgFileList->at(0));
      sb->getSearchLineEdit()->setFoundStyle();
    }
    else
    {
      tvPkgFileList->setCurrentIndex(sim->index(0,0));
      sb->getSearchLineEdit()->setNotFoundStyle();
    }
  }
}

/*
 * Every time the user presses Enter, Return, F3 or clicks Find Next inside a textBrowser...
 */
void MainWindow::searchBarFindNextInTextBrowser()
{
  QTextBrowser *tb = ui->twProperties->currentWidget()->findChild<QTextBrowser*>("textBrowser");
  SearchBar *sb = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");

  if (tb && sb && !sb->getTextToSearch().isEmpty()){
    if (!tb->find(sb->getTextToSearch())){
      tb->moveCursor(QTextCursor::Start);
      tb->find(sb->getTextToSearch());
    }
  }
}

/*
 * Every time the user presses Shift+F3 or clicks Find Previous inside a textBrowser...
 */
void MainWindow::searchBarFindPreviousInTextBrowser()
{
  QTextBrowser *tb = ui->twProperties->currentWidget()->findChild<QTextBrowser*>("textBrowser");
  SearchBar *sb = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");

  if (tb && sb && !sb->getTextToSearch().isEmpty()){
    if (!tb->find(sb->getTextToSearch(), QTextDocument::FindBackward)){
      tb->moveCursor(QTextCursor::End);
      tb->find(sb->getTextToSearch(), QTextDocument::FindBackward);
    }
  }
}

/*
 * Every time the user presses Enter, Return, F3 or clicks Find Next inside a treeView...
 */
void MainWindow::searchBarFindNextInTreeView()
{
  QTreeView *tvPkgFileList =
    ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");

  if (tvPkgFileList && tvPkgFileList->model()->rowCount() > 0 && m_foundFilesInPkgFileList->count() > 0)
  {
    if (m_indFoundFilesInPkgFileList+1 < m_foundFilesInPkgFileList->count())
    {
      m_indFoundFilesInPkgFileList = m_indFoundFilesInPkgFileList + 1;
    }
    else
    {
      m_indFoundFilesInPkgFileList = 0;
    }

    tvPkgFileList->setCurrentIndex(
          m_foundFilesInPkgFileList->at(m_indFoundFilesInPkgFileList));
    tvPkgFileList->scrollTo(
          m_foundFilesInPkgFileList->at(m_indFoundFilesInPkgFileList));
  }
}

/*
 * Every time the user presses Shift+F3 or clicks Find Previous inside a treeView...
 */
void MainWindow::searchBarFindPreviousInTreeView()
{
  QTreeView *tvPkgFileList =
    ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");

  if (tvPkgFileList && tvPkgFileList->model()->rowCount() > 0 && m_foundFilesInPkgFileList->count() > 0)
  {
    if (m_indFoundFilesInPkgFileList == 0)
    {
      m_indFoundFilesInPkgFileList = m_foundFilesInPkgFileList->count()-1;
    }
    else
      m_indFoundFilesInPkgFileList -= 1;

    tvPkgFileList->setCurrentIndex(
          m_foundFilesInPkgFileList->at(m_indFoundFilesInPkgFileList));
    tvPkgFileList->scrollTo(
          m_foundFilesInPkgFileList->at(m_indFoundFilesInPkgFileList));
  }
}

/*
 * Every time the user presses ESC or clicks the close button inside a textBrowser...
 */
void MainWindow::searchBarClosedInTextBrowser()
{
  searchBarTextChangedInTextBrowser("");
  QTextBrowser *tb = ui->twProperties->currentWidget()->findChild<QTextBrowser*>("textBrowser");

  if (tb)
    tb->setFocus();
}

/*
 * Every time the user presses ESC or clicks the close button inside a treeView...
 */
void MainWindow::searchBarClosedInTreeView()
{
  searchBarTextChangedInTreeView("");
  QTreeView *tb = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  if (tb) tb->setFocus();
}

/*
 * Helper to position in the first result when searching inside a textBrowser
 */
void MainWindow::positionInFirstMatch()
{
  QTextBrowser *tb = ui->twProperties->currentWidget()->findChild<QTextBrowser*>("textBrowser");
  SearchBar *sb = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");

  if (tb && sb && sb->isVisible() && !sb->getTextToSearch().isEmpty()){
    tb->moveCursor(QTextCursor::Start);
    if (tb->find(sb->getTextToSearch()))
      sb->getSearchLineEdit()->setFoundStyle();
    else
      sb->getSearchLineEdit()->setNotFoundStyle();
  }
}
