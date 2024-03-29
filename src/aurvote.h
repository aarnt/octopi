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

#ifndef AURSITE_H
#define AURSITE_H

#include <QObject>

class QNetworkAccessManager;

class AurVote : public QObject
{
private:
  bool m_debugInfo;
  QString m_loginUrl;
  QString m_voteUrl;
  QString m_unvoteUrl;
  QString m_pkgUrl;
  QString m_userName;
  QString m_password;

  QNetworkAccessManager *m_networkManager;

public:
  explicit AurVote(QObject *parent = nullptr);
  virtual ~AurVote();

  bool login();
  bool isLoggedIn();
  int isPkgVoted(const QString &pkgName);
  void voteForPkg(const QString &pkgName);
  void unvoteForPkg(const QString &pkgName);
  QStringList getVotedPackages();

  void setUserName(const QString &userName);
  void setPassword(const QString &password);
  void turnDebugInfoOn();
};

#endif // AURSITE_H
