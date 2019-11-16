/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2019 Alexandre Albuquerque Arnt
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

#ifndef PKOCTOPIHELPER_H
#define PKOCTOPIHELPER_H

#include "../src/constants.h"
#include <QString>
#include <QProcess>
#include <QObject>
#include <QFile>
#include <QRandomGenerator>

class OctopiHelper: QObject
{
Q_OBJECT

private:
  int m_exitCode;
  QProcess *m_process;
  QProcessEnvironment getProcessEnvironment();
  QString getTransactionTempFileName();
  QString m_suspiciousChars;

  static QFile *m_temporaryFile;

  static QFile* generateTemporaryFile(){
    quint32 gen = QRandomGenerator::global()->generate();
    m_temporaryFile = new QFile(ctn_TEMP_ACTIONS_FILE + QString::number(gen));
    m_temporaryFile->open(QIODevice::ReadWrite|QIODevice::Text);
    m_temporaryFile->setPermissions(QFile::Permissions(QFile::ExeOwner|QFile::ReadOwner));

    return m_temporaryFile;
  }

public:
  OctopiHelper();
  virtual ~OctopiHelper();

  int executePkgTransactionWithSharedMem();
  //int executePkgTransaction();
  inline int getExitCode() { return m_exitCode; }
  bool isOctoToolRunning(const QString &octoToolName);
};

#endif // PKOCTOPIHELPER_H
