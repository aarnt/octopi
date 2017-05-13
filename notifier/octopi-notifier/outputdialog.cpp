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
#include "ui_outputdialog.h"
#include "../../src/pacmanexec.h"
#include "../../src/searchbar.h"
#include "../../src/uihelper.h"
#include "../../src/strconstants.h"

#include <QTextBrowser>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QCloseEvent>
#include <QMessageBox>

/*
 * Class that displays pacman output for system upgrade
 */

/*
 * The obligatory constructor...
 */
OutputDialog::OutputDialog(QWidget *parent):
	QDialog(parent),
	ui(new Ui::OutputDialog)
{
  ui->setupUi(this);
  init();
  m_upgradeRunning = false;
  m_debugInfo = false;
}

/*
 * Sets if pacmanExec will be called in debugMode or not
 */
void OutputDialog::setDebugMode(bool newValue)
{
  m_debugInfo = newValue;
}

/*
 * Let's build the main widgets...
 */
void OutputDialog::init()
{
  setWindowIcon(IconHelper::getIconSystemUpgrade());

  connect(ui->m_searchBar, SIGNAL(textChanged(QString)), this, SLOT(onSearchBarTextChanged(QString)));
  connect(ui->m_searchBar, SIGNAL(closed()), this, SLOT(onSearchBarClosed()));
  connect(ui->m_searchBar, SIGNAL(findNext()), this, SLOT(onSearchBarFindNext()));
  connect(ui->m_searchBar, SIGNAL(findPrevious()), this, SLOT(onSearchBarFindPrevious()));
  ui->m_searchBar->show();
  
  ui->m_progressBar->close();
}

/*
 * Calls PacmanExec to begin system upgrade
 */
void OutputDialog::doSystemUpgrade()
{
  m_pacmanExec = new PacmanExec();

  if (m_debugInfo)
    m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(onPencertange(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(onWriteOutput(QString)));

  m_upgradeRunning = true;
  m_pacmanExec->doSystemUpgrade();

  //m_pacmanExec->doInstall("octopi");  //TEST CODE!
}

/*
 * Centers the dialog in the screen
 */
void OutputDialog::show()
{
#if QT_VERSION >= 0x050000
  utils::positionWindowAtScreenCenter(this);
#endif
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
  if (percentage > 0 && !ui->m_progressBar->isVisible()) ui->m_progressBar->show();
  ui->m_progressBar->setValue(percentage);
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void OutputDialog::positionTextEditCursorAtEnd()
{
  QTextCursor tc = ui->m_textBrowser->textCursor();
  tc.clearSelection();
  tc.movePosition(QTextCursor::End);
  ui->m_textBrowser->setTextCursor(tc);
}

/*
 * A helper method which writes the given string to the textbrowser
 */
void OutputDialog::writeToTabOutput(const QString &msg, TreatURLLinks treatURLLinks)
{
  utils::writeToTextBrowser(ui->m_textBrowser, msg, treatURLLinks);
}

/*
 * Slot called whenever PacmanExec emits a new output
 */
void OutputDialog::onWriteOutput(const QString &output)
{
  utils::positionTextEditCursorAtEnd(ui->m_textBrowser);
  ui->m_textBrowser->insertHtml(output);
  ui->m_textBrowser->ensureCursorVisible();
}

/*
 * Helper method to find the given "findText" in a TextEdit
 */
bool OutputDialog::textInTabOutput(const QString& findText)
{
  return (utils::strInQTextEdit(ui->m_textBrowser, findText));
}

/*
 * Slot called whenever PacmanExec finishes its job
 */
void OutputDialog::pacmanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  ui->m_progressBar->close();

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
  utils::searchBarTextChangedInTextBrowser(ui->m_textBrowser, ui->m_searchBar, strToSearch);
}

/*
 * User closed the search bar
 */
void OutputDialog::onSearchBarClosed()
{
  utils::searchBarClosedInTextBrowser(ui->m_textBrowser, ui->m_searchBar);
}

/*
 * User requested next found string
 */
void OutputDialog::onSearchBarFindNext()
{
  utils::searchBarFindNextInTextBrowser(ui->m_textBrowser, ui->m_searchBar);
}

/*
 * User requested previous found string
 */
void OutputDialog::onSearchBarFindPrevious()
{
  utils::searchBarFindPreviousInTextBrowser(ui->m_textBrowser, ui->m_searchBar);
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
  {
    emit finished(0);
    event->accept();
  }
}

/*
 * Whenever user presses Ctrl+F, we show the searchbar again
 */
void OutputDialog::keyPressEvent(QKeyEvent *ke)
{
  if(ke->key() == Qt::Key_F && ke->modifiers() == Qt::ControlModifier)
  {
    ui->m_searchBar->show();
  }
  else if(ke->key() == Qt::Key_Escape)
  {
    reject();
  }
  else ke->accept();
}
