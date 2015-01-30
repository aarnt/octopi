#ifndef CACHECLEANER_H
#define CACHECLEANER_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>

#include "../src/unixcommand.h"

namespace Ui {
class CacheCleaner;
}


/*
 * Helper class to accumulate UnixCommand standard and error output
 * on the fly
 */
class ProcessOutputAccumulator : QObject
{
  Q_OBJECT

public:
  ProcessOutputAccumulator(UnixCommand *cmd) {
    m_unixCommand = cmd;

    connect(m_unixCommand, SIGNAL( started() ), SLOT( reset() ) );
    connect(m_unixCommand, SIGNAL( readyReadStandardOutput() ), SLOT( standardOutputAvailable() ) );
    connect(m_unixCommand, SIGNAL( readyReadStandardError() ), SLOT( errorsOutputAvailable() ) );
  }

  QString getOutput() { return m_standardOutput; }
  QString getErrors() { return m_errorsOutput; }

public slots:

  void reset()
  {
    m_standardOutput.clear();
    m_errorsOutput.clear();
  }


  void standardOutputAvailable()
  {
    m_standardOutput.append(m_unixCommand->readAllStandardOutput());
  }


  void errorsOutputAvailable()
  {
    m_errorsOutput.append(m_unixCommand->readAllStandardError());
  }


protected:
  UnixCommand *m_unixCommand;
  QString m_standardOutput;
  QString m_errorsOutput;
};



/*
 * Main CacheCleaner window
 */
class CacheCleaner : public QMainWindow
{
    Q_OBJECT

private:
    Ui::CacheCleaner *ui;

    UnixCommand *m_cmdInstalled;
    UnixCommand *m_cmdUninstalled;

    ProcessOutputAccumulator *m_accumulatorInstalled;
    ProcessOutputAccumulator *m_accumulatorUninstalled;


public:
    explicit CacheCleaner(QWidget *parent = 0);
    ~CacheCleaner();

protected:
    int getKeepInstalledNumber();
    int getKeepUninstalledNumber();
    void processDryrunResult(QString output, QListWidget *list, QPushButton *cleanButton);

    QString getCleanInstalledOptions();
    QString getCleanUninstalledOptions();

    void closeEvent(QCloseEvent *);

public slots:
    void keepInstalledChanged();
    void keepUninstalledChanged();

    void refreshInstalledCache();
    void refreshUninstalledCache();

    void cleanInstalledCache();
    void cleanUninstalledCache();

    void finishedInstalled(int, QProcess::ExitStatus);
    void finishedUninstalled(int, QProcess::ExitStatus);

    void finishedDryrunInstalled(int, QProcess::ExitStatus);
    void finishedDryrunUninstalled(int, QProcess::ExitStatus);
};

#endif // CACHECLEANER_H
