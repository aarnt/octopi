/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2026 Alexandre Albuquerque Arnt
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

#ifndef AURVOTE2_H
#define AURVOTE2_H

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkCookieJar>
#include <QUrlQuery>
#include <QEventLoop>
#include <QRegularExpression>

static const QString LOGIN_URL = QStringLiteral(
    "https://aur.archlinux.org/login");

static const QString SEARCH_URL_TEMPLATE = QStringLiteral(
    "https://aur.archlinux.org/packages?SeB=nd&PP=%1&SB=w&O=0&SO=d");

static const QString PACKAGES_URL = QStringLiteral(
    "https://aur.archlinux.org/packages/%1");

static const QString VOTE_URL = QStringLiteral(
    "https://aur.archlinux.org/pkgbase/%1/vote");

static const QString UNVOTE_URL = QStringLiteral(
    "https://aur.archlinux.org/pkgbase/%1/unvote");

constexpr int PACKAGES_PER_PAGE = 250;

class AurVote2
{
  public:

  AurVote2();

  void setUserName(const QString& user);
  void setPassword(const QString& pass);

  bool testLogin();
  bool isLoggedIn();
  bool login();
  int isPkgVoted(const QString& pkg);

  //QString getPkgBase(const QString& pkg);

  bool voteForPkg(const QString& pkgBase);
  bool unvoteForPkg(const QString& pkgBase);
  QStringList getVotedPackages();

  private:

  QString m_username;
  QString m_password;
  QNetworkAccessManager manager;
  QString get(const QString& url);
  QString post(const QNetworkRequest& req, const QByteArray& data);
};

#endif // AURVOTE2_H
