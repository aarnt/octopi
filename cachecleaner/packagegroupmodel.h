/*
Copyright 2015 Michaël Lhomme

This file is part of AppSet.

AppSet is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

AppSet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with AppSet; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef PACKAGEGROUP_H
#define PACKAGEGROUP_H

#include <QObject>
#include <QSpinBox>
#include <QListWidget>
#include <QPushButton>

#include "../src/unixcommand.h"

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

    //connect(m_unixCommand, SIGNAL( started() ), SLOT( reset() ) );
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
 * Wrapper to encapsulate operation on package groups (installed and uninstalled)
 */
class PackageGroupModel : public QObject
{
    Q_OBJECT

public:
    PackageGroupModel(QString, QListWidget *, QSpinBox *, QPushButton *, QPushButton *);
    ~PackageGroupModel();

protected:
    QString m_optionsString;
    QListWidget *m_listView;
    QSpinBox *m_spinner;
    QPushButton *m_refreshButton;
    QPushButton *m_cleanButton;

    int m_oldKeepValue;

    UnixCommand *m_cmd;
    ProcessOutputAccumulator *m_acc;

    void processDryrunResult(QString);
    QString getOptions();

public slots:
    void refreshCacheView();
    void cleanCache();
    void updateKeepArchives();
    void keepArchivesChanged();

    void finishedDryrun(int, QProcess::ExitStatus);
    void finishedClean(int, QProcess::ExitStatus);
};

#endif // PACKAGEGROUP_H
