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

#include "propertiestabwidget.h"
#include "constants.h"
#include "strconstants.h"
#include "searchbar.h"
#include "termwidget.h"
#include "uihelper.h"
#include "treeviewpackagesitemdelegate.h"

#include <QObject>
#include <QTabBar>
#include <QEvent>
#include <QMouseEvent>
#include <QTextBrowser>
#include <QGridLayout>
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QDesktopServices>

/*
 * This class implements Octopi's tab bar that holds "Info", "Files", "Actions", "Output", "News", "Help" and "Terminal"
 */

PropertiesTabWidget::PropertiesTabWidget(QWidget *parent):QTabWidget(parent)
{
  tabBar()->installEventFilter(this);
}

void PropertiesTabWidget::initTabInfo()
{
  QWidget *tabInfo = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout ( tabInfo );
  gridLayoutX->setSpacing ( 0 );

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  gridLayoutX->setMargin(0);
#endif

  m_textInfo = new QTextBrowser(tabInfo);
  m_textInfo->setObjectName(QStringLiteral("textBrowser"));
  m_textInfo->setReadOnly(true);
  m_textInfo->setFrameShape(QFrame::NoFrame);
  m_textInfo->setFrameShadow(QFrame::Plain);
  m_textInfo->setOpenLinks(false);

  gridLayoutX->addWidget ( m_textInfo, 0, 0, 1, 1 );

  QString tabName(StrConstants::getTabInfoName());
  removeTab(ctn_TABINDEX_INFORMATION);
  insertTab(ctn_TABINDEX_INFORMATION, tabInfo, QApplication::translate (
      "MainWindow", tabName.toUtf8().constData(), nullptr));
  setUsesScrollButtons(false);

  m_searchBarInfo = new SearchBar(this);
  m_searchBarInfo->setFocusPolicy(Qt::NoFocus);
  gridLayoutX->addWidget(m_searchBarInfo, 1, 0, 1, 1);

  setCurrentIndex(ctn_TABINDEX_INFORMATION);
  m_textInfo->show();
  m_textInfo->setFocus();
}

void PropertiesTabWidget::initTabFiles()
{
  QWidget *tabPkgFileList = new QWidget(this);
  QGridLayout *gridLayoutX = new QGridLayout ( tabPkgFileList );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setContentsMargins(0, 0, 0, 0);

  QStandardItemModel *modelPkgFileList = new QStandardItemModel(this);
  m_tvPkgFileList = new QTreeView(tabPkgFileList);
  m_tvPkgFileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tvPkgFileList->setDropIndicatorShown(false);
  m_tvPkgFileList->setAcceptDrops(false);
  m_tvPkgFileList->header()->setSortIndicatorShown(false);
  m_tvPkgFileList->header()->setSectionsClickable(false);
  m_tvPkgFileList->header()->setSectionsMovable(false);
  m_tvPkgFileList->header()->setSectionResizeMode(QHeaderView::Fixed);
  m_tvPkgFileList->setFrameShape(QFrame::NoFrame);
  m_tvPkgFileList->setFrameShadow(QFrame::Plain);
  m_tvPkgFileList->setObjectName(QStringLiteral("tvPkgFileList"));
  //m_tvPkgFileList->setStyleSheet(StrConstants::getTreeViewCSS());

  modelPkgFileList->setSortRole(0);
  modelPkgFileList->setColumnCount(0);
  gridLayoutX->addWidget(m_tvPkgFileList, 0, 0, 1, 1);
  m_tvPkgFileList->setModel(modelPkgFileList);

  QString aux(StrConstants::getTabFilesName());
  removeTab(ctn_TABINDEX_FILES);
  insertTab(ctn_TABINDEX_FILES, tabPkgFileList, QApplication::translate (
                                                  "MainWindow", aux.toUtf8().constData(), nullptr/*, QApplication::UnicodeUTF8*/ ) );
  m_tvPkgFileList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_searchBarFiles = new SearchBar(this);
  m_searchBarFiles->setFocusPolicy(Qt::NoFocus);

  gridLayoutX->addWidget(m_searchBarFiles, 1, 0, 1, 1);
}

void PropertiesTabWidget::initTabActions()
{
  m_modelTransaction = new QStandardItemModel(this);
  QWidget *tabTransaction = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabTransaction);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setContentsMargins(0, 0, 0, 0);

  m_tvTransaction = new QTreeView(tabTransaction);
  m_tvTransaction->setObjectName(QStringLiteral("tvTransaction"));
  m_tvTransaction->setContextMenuPolicy(Qt::CustomContextMenu);
  m_tvTransaction->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tvTransaction->setDropIndicatorShown(false);
  m_tvTransaction->setAcceptDrops(false);
  m_tvTransaction->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_tvTransaction->setItemDelegate(new TreeViewPackagesItemDelegate(m_tvTransaction));
  m_tvTransaction->header()->setSortIndicatorShown(false);
  m_tvTransaction->header()->setSectionsClickable(false);
  m_tvTransaction->header()->setSectionsMovable(false);
  m_tvTransaction->header()->setSectionResizeMode(QHeaderView::Fixed);
  m_tvTransaction->setFrameShape(QFrame::NoFrame);
  m_tvTransaction->setFrameShadow(QFrame::Plain);
  //m_tvTransaction->setStyleSheet(StrConstants::getTreeViewCSS());
  m_tvTransaction->expandAll();

  m_modelTransaction->setSortRole(0);
  m_modelTransaction->setColumnCount(0);

  QStringList sl;
  m_modelTransaction->setHorizontalHeaderLabels(sl << StrConstants::getPackages());

  QStandardItem *siToBeRemoved = new QStandardItem(IconHelper::getIconToRemove(),
                                                   StrConstants::getTransactionRemoveText());
  QStandardItem *siToBeInstalled = new QStandardItem(IconHelper::getIconToInstall(),
                                                     StrConstants::getTransactionInstallText());

  m_modelTransaction->appendRow(siToBeRemoved);
  m_modelTransaction->appendRow(siToBeInstalled);

  gridLayoutX->addWidget(m_tvTransaction, 0, 0, 1, 1);

  m_tvTransaction->setModel(m_modelTransaction);

  QString aux(StrConstants::getActions());
  removeTab(ctn_TABINDEX_ACTIONS);
  insertTab(ctn_TABINDEX_ACTIONS, tabTransaction, QApplication::translate (
                                "MainWindow", aux.toUtf8().constData(), nullptr/*, QApplication::UnicodeUTF8*/ ));
}

void PropertiesTabWidget::initTabNews()
{
  QString aux(StrConstants::getTabNewsName());
  QWidget *tabNews = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabNews);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setContentsMargins(0, 0, 0, 0);

  m_textNews = new QTextBrowser(tabNews);
  m_textNews->setObjectName(QStringLiteral("textBrowser"));
  m_textNews->setReadOnly(true);
  m_textNews->setFrameShape(QFrame::NoFrame);
  m_textNews->setFrameShadow(QFrame::Plain);
  m_textNews->setOpenExternalLinks(true);
  gridLayoutX->addWidget(m_textNews, 0, 0, 1, 1);
  //m_textNews->show();

  int tindex = insertTab(ctn_TABINDEX_NEWS, tabNews, QApplication::translate (
      "MainWindow", aux.toUtf8().constData(), nullptr));
  setTabText(indexOf(tabNews), QApplication::translate(
      "MainWindow", aux.toUtf8().constData(), nullptr));

  m_searchBarNews = new SearchBar(this);
  m_searchBarNews->setFocusPolicy(Qt::NoFocus);

  gridLayoutX->addWidget(m_searchBarNews, 1, 0, 1, 1);

  m_textNews->show();
  setCurrentIndex(tindex);
  m_textNews->setFocus();
}

void PropertiesTabWidget::initTabOutput()
{
  QWidget *tabOutput = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabOutput);
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setContentsMargins(0, 0, 0, 0);

  m_textOutput = new QTextBrowser(tabOutput);
  m_textOutput->setObjectName(QStringLiteral("textBrowser"));
  m_textOutput->setReadOnly(true);
  m_textOutput->setOpenLinks(false);
  m_textOutput->setFrameShape(QFrame::NoFrame);
  m_textOutput->setFrameShadow(QFrame::Plain);

  gridLayoutX->addWidget (m_textOutput, 0, 0, 1, 1);

  QString aux(StrConstants::getTabOutputName());
  removeTab(ctn_TABINDEX_OUTPUT);
  insertTab(ctn_TABINDEX_OUTPUT, tabOutput, QApplication::translate (
      "MainWindow", aux.toUtf8().constData(), nullptr) );

  m_searchBarOutput = new SearchBar(this);
  m_searchBarOutput->setFocusPolicy(Qt::NoFocus);

  gridLayoutX->addWidget(m_searchBarOutput, 1, 0, 1, 1);

  setCurrentIndex(ctn_TABINDEX_OUTPUT);
  m_textOutput->show();
  m_textOutput->setFocus();
}

void PropertiesTabWidget::initTabHelpUsage()
{
  QWidget *tabHelpUsage = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabHelpUsage);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setContentsMargins(0, 0, 0, 0);

  QTextBrowser *text = new QTextBrowser(tabHelpUsage);
  text->setObjectName(QStringLiteral("textBrowser"));
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget(text, 0, 0, 1, 1);

  text->setHtml(m_helpUsageText);

  int tindex = addTab(tabHelpUsage, StrConstants::getHelp() );
  setTabText(indexOf(tabHelpUsage), StrConstants::getHelp());

  m_searchBarHelpUsage = new SearchBar(this);
  m_searchBarHelpUsage->setFocusPolicy(Qt::NoFocus);

  gridLayoutX->addWidget(m_searchBarHelpUsage, 1, 0, 1, 1);

  text->show();
  setCurrentIndex(tindex);
  text->setFocus();
}

void PropertiesTabWidget::initTabTerminal()
{
  QWidget *tabTerminal = new QWidget(this);
  QGridLayout *gridLayoutX = new QGridLayout(tabTerminal);
  gridLayoutX->setSpacing ( 0 );

  gridLayoutX->setContentsMargins(0, 0, 0, 0);

  gridLayoutX->addWidget(m_console, 0, 0, 1, 1);
  removeTab(ctn_TABINDEX_TERMINAL);
  QString aux(StrConstants::getTabTerminal());
  insertTab(ctn_TABINDEX_TERMINAL, tabTerminal, QApplication::translate (
                                                  "MainWindow", aux.toUtf8().constData(), nullptr) );
  setCurrentIndex(ctn_TABINDEX_TERMINAL);
  m_console->setFocus();
}

void PropertiesTabWidget::setConsole(TermWidget *console)
{
  m_console = console;
}

void PropertiesTabWidget::setHelpUsageText(QString text)
{
  m_helpUsageText = text;
}

bool PropertiesTabWidget::eventFilter(QObject *obj, QEvent *event)
{
  bool isTabBar = (obj == tabBar());
  bool isMouseButtonPress = event->type() == QEvent::MouseButtonPress;

  if (isTabBar && isMouseButtonPress)
  {
    auto me = static_cast<QMouseEvent *>(event);

    if (me && me->buttons() == Qt::RightButton)
    {
      int tabIndex = tabBar()->tabAt(me->pos());
      if (tabIndex == ctn_TABINDEX_NEWS)
      {
        QMenu* menu = new QMenu(this);
        menu->addAction(StrConstants::getOpenNewsInBrowser(), []{
          QDesktopServices::openUrl(QUrl(SettingsManager::getDistroNewsSite()));
        });

        QPoint point = me->pos();
        QPoint pt2 = this->mapToGlobal(point);
        menu->exec(pt2);

        return true;
      }
    }
  }

  return QObject::eventFilter(obj, event);
}
