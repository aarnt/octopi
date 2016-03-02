#ifndef OUTPUTDIALOG_H
#define OUTPUTDIALOG_H

#include "../../src/constants.h"

#include <QDialog>
#include <QProcess>

class PacmanExec;
class QString;
class QTextBrowser;
class QVBoxLayout;
class QProgressBar;
class SearchBar;
class QWidget;
class QCloseEvent;
class QKeyEvent;

/*
 * Class that displays pacman output for system upgrade
 */
class OutputDialog : public QDialog
{
  Q_OBJECT

private:
  QTextBrowser *m_textBrowser;
  QProgressBar *m_progressBar;
  QVBoxLayout *m_mainLayout;
  PacmanExec *m_pacmanExec;
  SearchBar *m_searchBar;
  bool m_upgradeRunning;

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

public slots:
  void show();
  void reject();
};

#endif // OUTPUTDIALOG_H
