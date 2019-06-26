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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "ui_optionsdialog.h"
#include <QDialog>

namespace Options
{
  enum { ectn_BACKEND=0x1, ectn_ICON=0x2 }; typedef unsigned long result;
}

class OptionsDialog : public QDialog, public Ui_OptionsDialog
{
  Q_OBJECT

private:
  bool m_iconHasChanged;
  bool m_backendHasChanged;
  bool m_calledByOctopi;

  QString m_redIconPath;
  QString m_yellowIconPath;
  QString m_greenIconPath;
  QString m_busyIconPath;

  void initialize();
  void initButtonBox();
  void initGeneralTab();
  void initAURTab();
  void initBackendTab();
  void initIconTab();
  void initUpdatesTab();
  void removeTabByName(const QString &tabName);

protected:
  virtual void accept();

public:
  explicit OptionsDialog(QWidget *parent = 0);

signals:
  void AURToolChanged();
  void terminalChanged();
  void alternateRowColorsChanged();

private slots:
  void defaultIconChecked(bool checked);
  void selRedIconPath();
  void selYellowIconPath();
  void selGreenIconPath();
  void selBusyIconPath();
  void selectOnceADay();
  void selectOnceADayAt();
  void selectOnceEvery();
  void selectNever();
  void comboAURChanged(const QString &text);
};

#endif // OptionsDialog_H
