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
#include "../src/pacmanexec.h"
#include "../src/searchbar.h"
#include "../src/uihelper.h"
#include "../src/utils.h"
#include "../src/strconstants.h"
#include "../src/termwidget.h"

#include <QTextBrowser>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QAction>
#include <QToolButton>

/*
 * Class that displays pacman output for system upgrade
 */

/*
 * The obligatory constructor...
 */
OutputDialog::OutputDialog(QWidget *parent): QDialog(parent)
{
  m_exitCode = 0;
  m_upgradeRunning = false;
  m_debugInfo = false;
  m_AURUpgradeExecuting = false;
  m_sharedMemory = new QSharedMemory(QStringLiteral("org.arnt.octopi"), this);
}

/*
 * Sets if pacmanExec will be called in debugMode or not
 */
void OutputDialog::setDebugMode(bool newValue)
{
  m_debugInfo = newValue;
}

/*
 * Sets the list of AUR packages that need to be upgraded
 */
void OutputDialog::setListOfAURPackagesToUpgrade(const QString &list)
{
  m_listOfAURPackagesToUpgrade = list;
}

/*
 * Controls if this dialog was called for Pacman or AUR upgrade
 */
void OutputDialog::setViewAsTextBrowser(bool value)
{
  m_viewAsTextBrowser = value;
}

/*
 * Let's build the main widgets for Pacman System Upgrade...
 */
void OutputDialog::initAsTextBrowser()
{
  this->resize(650, 500);
  setWindowTitle(QCoreApplication::translate("MainWindow", "System upgrade"));
  setWindowIcon(IconHelper::getIconSystemUpgrade());

  m_actionStopTransaction = new QAction(this);
  m_actionStopTransaction->setIcon(IconHelper::getIconStop());
  m_actionStopTransaction->setText(StrConstants::getStop());
  connect(m_actionStopTransaction, SIGNAL(triggered()), this, SLOT(stopTransaction()));

  m_toolButtonStopTransaction = new QToolButton(this);
  m_toolButtonStopTransaction->setDefaultAction(m_actionStopTransaction);
  m_toolButtonStopTransaction->setVisible(false);
  m_toolButtonStopTransaction->setAutoRaise(true);

  m_mainLayout = new QVBoxLayout(this);
  m_horizLayout = new QHBoxLayout();
  m_textBrowser = new QTextBrowser(this);
  m_progressBar = new QProgressBar(this);

  m_horizLayout->addWidget(m_progressBar);
  m_horizLayout->addSpacing(2);
  m_horizLayout->addWidget(m_toolButtonStopTransaction);
  m_textBrowser->setGeometry(QRect(0, 0, 650, 500));
  m_textBrowser->setFrameShape(QFrame::NoFrame);

  m_mainLayout->addWidget(m_textBrowser);

  m_searchBar = new SearchBar(this);
  connect(m_searchBar, SIGNAL(textChanged(QString)), this, SLOT(onSearchBarTextChanged(QString)));
  connect(m_searchBar, SIGNAL(closed()), this, SLOT(onSearchBarClosed()));
  connect(m_searchBar, SIGNAL(findNext()), this, SLOT(onSearchBarFindNext()));
  connect(m_searchBar, SIGNAL(findPrevious()), this, SLOT(onSearchBarFindPrevious()));

  m_mainLayout->addLayout(m_horizLayout);
  m_mainLayout->addWidget(m_searchBar);
  m_mainLayout->setSpacing(0);
  m_mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
  m_mainLayout->setContentsMargins(2, 2, 2, 2);

  m_progressBar->setMinimum(0);
  m_progressBar->setMaximum(100);
  m_progressBar->setValue(0);
  m_progressBar->close();
}

/*
 * Let's build the main widgets for AUR Upgrade...
 */
void OutputDialog::initAsTermWidget()
{
  this->resize(650, 500);
  setWindowTitle(QCoreApplication::translate("MainWindow", "System upgrade"));
  setWindowIcon(IconHelper::getIconSystemUpgrade());

  m_mainLayout = new QVBoxLayout(this);
  m_console = new TermWidget(this);
  m_mainLayout->addWidget(m_console);

  m_mainLayout->setSpacing(0);
  m_mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
  m_mainLayout->setContentsMargins(2, 2, 2, 2);
  m_console->setFocus();  
  //m_console->toggleShowSearchBar();

  m_console->installEventFilter(this);
}

/*
 * When user wants to upgrade system using a terminal
 */
void OutputDialog::doSystemUpgradeInTerminal()
{
  m_pacmanExec = new PacmanExec(this);
  m_pacmanExec->setSharedMemory(m_sharedMemory);

  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this,
                   SLOT(onExecCommandInTabTerminal(QString)));
  m_upgradeRunning = true;
  m_pacmanExec->doSystemUpgradeInTerminal();
}

/*
 * When user wants to start an AUR upgrade transaction
 */
void OutputDialog::doAURUpgrade()
{
  m_AURUpgradeExecuting=true;
  m_pacmanExec = new PacmanExec(this);
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this,
                   SLOT(onExecCommandInTabTerminal(QString)));
  m_upgradeRunning = true;
  m_pacmanExec->doAURUpgrade(m_listOfAURPackagesToUpgrade);
}

/*
 * When there is a command to exec in the terminal
 */
void OutputDialog::onExecCommandInTabTerminal(QString command)
{
  disconnect(m_console, SIGNAL(onPressAnyKeyToContinue()), this, SLOT(onPressAnyKeyToContinue()));
  disconnect(m_console, SIGNAL(onCancelControlKey()), this, SLOT(onCancelControlKey()));
  disconnect(m_console, SIGNAL(onKeyQuit()), this, SLOT(reject()));
  connect(m_console, SIGNAL(onPressAnyKeyToContinue()), this, SLOT(onPressAnyKeyToContinue()));
  connect(m_console, SIGNAL(onCancelControlKey()), this, SLOT(onCancelControlKey()));
  connect(m_console, SIGNAL(onKeyQuit()), this, SLOT(reject()));

  m_console->execute(command);
  m_console->setFocus();
}

/*
 * Whenever the terminal transaction has finished, we can update the UI
 */
void OutputDialog::onPressAnyKeyToContinue()
{
  m_console->setFocus();

  if (!m_upgradeRunning) return;
  
    delete m_pacmanExec;

  if (m_sharedMemory->isAttached()) m_sharedMemory->detach();
  m_upgradeRunning = false;
}

/*
 * Whenever a user strikes Ctrl+C, Ctrl+D or Ctrl+Z in the terminal
 */
void OutputDialog::onCancelControlKey()
{
  if (m_upgradeRunning)
  {
    
      delete m_pacmanExec;

    if (m_sharedMemory->isAttached()) m_sharedMemory->detach();
    m_pacmanExec = nullptr;
    m_upgradeRunning = false;
  }
}

/*
 * Calls PacmanExec to begin system upgrade
 */
void OutputDialog::doSystemUpgrade()
{
  //Is pacman being executed?
  if (UnixCommand::isPacmanDbLocked()) return;

  m_pacmanExec = new PacmanExec(this);
  m_pacmanExec->setSharedMemory(m_sharedMemory);

  if (m_debugInfo)
    m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(onPencertange(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(onWriteOutput(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));

  m_upgradeRunning = true;
  m_pacmanExec->doSystemUpgrade();
}

/*
 * Make the shared memory available again
 */
void OutputDialog::detachSharedMemory()
{
  m_sharedMemory->detach();
}

/*
 * Centers the dialog in the screen
 */
void OutputDialog::show()
{
  //If we are asking for a Pacman system upgrade...
  if (m_viewAsTextBrowser)
    initAsTextBrowser();
  else
    initAsTermWidget();

  //Let's restore the dialog size saved...
  restoreGeometry(SettingsManager::getOutputDialogWindowSize());

  QDialog::show();
}

/*
 * Whenever the user presses the ESC key
 */
void OutputDialog::reject()
{
  if (!m_upgradeRunning)
  {
    //Let's save the dialog size value before closing it.
    QByteArray windowSize=saveGeometry();
    SettingsManager::setOutputDialogWindowSize(windowSize);

    emit finished(m_exitCode);
    QDialog::reject();
  }
}

/*
 * Slot called whenever PacmanExec emits a new percentage change
 */
void OutputDialog::onPencertange(int percentage)
{
  if (percentage > 0 && !m_progressBar->isVisible())
  {
    m_progressBar->show();
    if (SettingsManager::getShowStopTransaction()) m_toolButtonStopTransaction->show();
  }

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
  m_exitCode = exitCode;

  m_progressBar->close();
  if (SettingsManager::getShowStopTransaction()) m_toolButtonStopTransaction->close();

  if ((exitCode == 0) && exitStatus == QProcess::NormalExit)
  {
    //If there are .pacnew files to print...
    QStringList dotPacnewFiles = m_pacmanExec->getDotPacnewFileList();
    if (dotPacnewFiles.count() > 0)
    {
      writeToTabOutput(QStringLiteral("<br>"));
      foreach(QString dotPacnewFile, dotPacnewFiles)
      {
        if (!dotPacnewFile.contains(QLatin1String("<br>")))
          writeToTabOutput( QLatin1String("<br>") + dotPacnewFile, ectn_DONT_TREAT_URL_LINK);
        else
          writeToTabOutput(dotPacnewFile, ectn_DONT_TREAT_URL_LINK);
      }
    }

    writeToTabOutput(QLatin1String("<br><b>") + StrConstants::getCommandFinishedOK() + QLatin1String("</b><br>"));
  }
  else
  {
    writeToTabOutput(QLatin1String("<br><b>") + StrConstants::getCommandFinishedWithErrors() + QLatin1String("</b><br>"));
  }

  if (exitCode != 0 && (textInTabOutput(QStringLiteral("conflict")))) //|| _textInTabOutput("could not satisfy dependencies")))
  {
    int res = QMessageBox::question(this, StrConstants::getThereHasBeenATransactionError(),
                                    StrConstants::getConfirmExecuteTransactionInTerminal(),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
      m_pacmanExec->runLatestCommandInTerminal();
      return;
    }
  }

  delete m_pacmanExec;
  if (m_sharedMemory->isAttached()) m_sharedMemory->detach();
  m_upgradeRunning = false;
}

/*
 * Whenever PacmanExec says we can show/close the stop transaction toolbutton...
 */
void OutputDialog::onCanStopTransaction(bool yesNo)
{
  if (yesNo && m_progressBar->isHidden()) return;
  if (SettingsManager::getShowStopTransaction()) m_toolButtonStopTransaction->setVisible(yesNo);
}

/*
 * Kills all pacman processes
 *
 * Returns octopi-sudo exit code
 */
int OutputDialog::stopTransaction()
{
  int res=0;

  if (!m_AURUpgradeExecuting)
  {
    res=m_pacmanExec->cancelProcess();
  }

  if (res != 1)
  {
    if (m_sharedMemory->isAttached()) m_sharedMemory->detach();
  }

  return res;
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
    int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                          StrConstants::getThereIsARunningTransaction() + QLatin1Char('\n') +
                          StrConstants::getDoYouReallyWantToQuit(),
                          QMessageBox::Yes | QMessageBox::No,
                          QMessageBox::No);
    if (res == QMessageBox::Yes)
    {
      if (m_viewAsTextBrowser)
      {
        int ret=stopTransaction();
        if (ret == 1)
        {
          event->ignore();
          return;
        }
      }

      if (m_sharedMemory->isAttached()) m_sharedMemory->detach();
      m_upgradeRunning = false;
      reject();
    }
    else
    {
      event->ignore();
    }
  }
  else
  {
    emit finished(m_exitCode);
    event->accept();
    //Let's save window size...
    reject();
  }
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
    if (m_upgradeRunning)
    {
      int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                          StrConstants::getThereIsARunningTransaction() + QLatin1Char('\n') +
                          StrConstants::getDoYouReallyWantToQuit(),
                          QMessageBox::Yes | QMessageBox::No,
                          QMessageBox::No);
      if (res == QMessageBox::Yes)
      {
        if (m_viewAsTextBrowser)
        {
          int ret=stopTransaction();
          if (ret == 1)
          {
            ke->ignore();
            return;
          }
        }

        m_upgradeRunning = false;
        reject();
      }
      else
      {
        ke->ignore();
      }
    }
    else reject();
  }
  else ke->accept();
}

/*
 * Filters keypressevents from Console
 */
bool OutputDialog::eventFilter(QObject *, QEvent *event)
{
  if(event->type() == QKeyEvent::KeyRelease)
  {
    QKeyEvent *ke = static_cast<QKeyEvent*>(event);
    if (ke->key() == Qt::Key_Escape)
    {
      if (m_upgradeRunning)
      {
        int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                        StrConstants::getThereIsARunningTransaction() + QLatin1Char('\n') +
                                        StrConstants::getDoYouReallyWantToQuit(),
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No);
        if (res == QMessageBox::Yes)
        {
          m_upgradeRunning = false;
          reject();
          return true;
        }
        else
        {
          ke->ignore();
          return true;
        }
      }
      else
      {
        reject();
        return true;
      }
    }
    else if(ke->key() == Qt::Key_F && ke->modifiers() == Qt::ControlModifier)
    {
      m_console->toggleShowSearchBar();
    }
  }

  return false;
}
