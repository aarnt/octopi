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

#include "outputdialog.h"
#include "../../src/pacmanexec.h"
#include "../../src/searchbar.h"
#include "../../src/uihelper.h"
#include "../../src/strconstants.h"

#include <QTextBrowser>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QScreen>
#include <QCloseEvent>
#include <QMessageBox>
#include <QApplication>

/*
 * Class that displays pacman output for system upgrade
 */

/*
 * The obligatory constructor...
 */
OutputDialog::OutputDialog(QWidget *parent): QDialog(parent)
{
  init();
  m_upgradeRunning = false;
}

/*
 * Let's build the main widgets...
 */
void OutputDialog::init()
{
  this->resize(650, 500);

  setWindowTitle(QCoreApplication::translate("MainWindow", "System upgrade"));
  setWindowIcon(IconHelper::getIconSystemUpgrade());
  m_mainLayout = new QVBoxLayout(this);
  m_textBrowser = new QTextBrowser(this);
  m_progressBar = new QProgressBar(this);

  m_textBrowser->setGeometry(QRect(0, 0, 650, 500));

  m_mainLayout->addWidget(m_textBrowser);

  m_searchBar = new SearchBar(this);
  connect(m_searchBar, SIGNAL(textChanged(QString)), this, SLOT(onSearchBarTextChanged(QString)));
  connect(m_searchBar, SIGNAL(closed()), this, SLOT(onSearchBarClosed()));
  connect(m_searchBar, SIGNAL(findNext()), this, SLOT(onSearchBarFindNext()));
  connect(m_searchBar, SIGNAL(findPrevious()), this, SLOT(onSearchBarFindPrevious()));
  m_mainLayout->addWidget(m_searchBar);

  m_mainLayout->addWidget(m_progressBar);
  m_mainLayout->setSpacing(0);
  m_mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
  m_mainLayout->setContentsMargins(2, 2, 2, 2);

  m_progressBar->setMinimum(0);
  m_progressBar->setMaximum(100);
  m_progressBar->setValue(0);
  m_progressBar->close();
  m_searchBar->show();
}

/*
 * Calls PacmanExec to begin system upgrade
 */
void OutputDialog::doSystemUpgrade()
{
  m_pacmanExec = new PacmanExec();
  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(onPencertange(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(onWriteOutput(QString)));

  m_upgradeRunning = true;
  //m_pacmanExec->doSystemUpgrade();
  m_pacmanExec->doInstall("octopi");
}

/*
 * Centers the dialog in the screen
 */
void OutputDialog::show()
{
  QRect screen;

  foreach(QScreen *s, QGuiApplication::screens())
  {
    if (s->name() == QGuiApplication::primaryScreen()->name())
    {
      screen = s->geometry();
    }
  }

  int centerX = (screen.width() - this->width()) / 2;
  int centerY = (screen.height() - this->height()) / 2;
  move(QPoint(centerX, centerY));

  QDialog::show();
  doSystemUpgrade();
}

/*
 * Whenever the user presses the ESC key
 */
void OutputDialog::reject()
{
  if (!m_upgradeRunning)
  {
    QDialog::reject();
  }
}

/*
 * Slot called whenever PacmanExec emits a new percentage change
 */
void OutputDialog::onPencertange(int percentage)
{
  if (percentage > 0 && !m_progressBar->isVisible()) m_progressBar->show();
  m_progressBar->setValue(percentage);
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void OutputDialog::positionTextEditCursorAtEnd()
{
  QTextCursor tc = m_textBrowser->textCursor();

  tc.clearSelection();
  tc.movePosition(QTextCursor::End);
  m_textBrowser->setTextCursor(tc);
}

/*
 * A helper method which writes the given string to the textbrowser
 */
void OutputDialog::writeToTabOutput(const QString &msg, TreatURLLinks treatURLLinks)
{
  utils::writeToTextBrowser(m_textBrowser, msg, treatURLLinks);
}

/*
 * Slot called whenever PacmanExec emits a new output
 */
void OutputDialog::onWriteOutput(const QString &output)
{
  utils::positionTextEditCursorAtEnd(m_textBrowser);
  m_textBrowser->insertHtml(output);
  m_textBrowser->ensureCursorVisible();
}

/*
 * Helper method to find the given "findText" in a TextEdit
 */
bool OutputDialog::textInTabOutput(const QString& findText)
{
  return (utils::strInQTextEdit(m_textBrowser, findText));
}

/*
 * Slot called whenever PacmanExec finishes its job
 */
void OutputDialog::pacmanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  m_progressBar->close();

  if ((exitCode == 0) && exitStatus == QProcess::NormalExit)
  {
    writeToTabOutput("<br><b>" + StrConstants::getCommandFinishedOK() + "</b><br>");
  }
  else
  {
    writeToTabOutput("<br><b>" + StrConstants::getCommandFinishedWithErrors() + "</b><br>");
  }

  if (exitCode != 0 && (textInTabOutput("conflict"))) //|| _textInTabOutput("could not satisfy dependencies")))
  {
    int res = QMessageBox::question(this, StrConstants::getThereHasBeenATransactionError(),
                                    StrConstants::getConfirmExecuteTransactionInTerminal(),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
      m_pacmanExec->runLastestCommandInTerminal();
      return;
    }
  }

  delete m_pacmanExec;
  m_upgradeRunning = false;
}

/*
 * User changed text to search in the line edit
 */
void OutputDialog::onSearchBarTextChanged(QString strToSearch)
{
  utils::searchBarTextChangedInTextBrowser(m_textBrowser, m_searchBar, strToSearch);
}

/*
 * User closed the search bar
 */
void OutputDialog::onSearchBarClosed()
{
  utils::searchBarClosedInTextBrowser(m_textBrowser, m_searchBar);
}

/*
 * User requested next found string
 */
void OutputDialog::onSearchBarFindNext()
{
  utils::searchBarFindNextInTextBrowser(m_textBrowser, m_searchBar);
}

/*
 * User requested previous found string
 */
void OutputDialog::onSearchBarFindPrevious()
{
  utils::searchBarFindPreviousInTextBrowser(m_textBrowser, m_searchBar);
}

/*
 * Let's not exit the dialog if a system upgrade is running
 */
void OutputDialog::closeEvent(QCloseEvent *event)
{
  //We cannot quit while there is a running transaction!
  if(m_upgradeRunning)
  {
    event->ignore();
  }
  else
    event->accept();
}

/*
 * Whenever user presses Ctrl+F, we show the searchbar again
 */
void OutputDialog::keyPressEvent(QKeyEvent *ke)
{
  if(ke->key() == Qt::Key_F && ke->modifiers() == Qt::ControlModifier)
  {
    m_searchBar->show();
  }
  else if(ke->key() == Qt::Key_Escape)
  {
    reject();
  }
  else ke->accept();
}
