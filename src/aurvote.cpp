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

#include "aurvote.h"

#include <QEventLoop>
#include <QtNetwork/QNetworkReply>
#include <QUrlQuery>
#include <QRegularExpression>

/*
 * This class provides a way to vote/unvote for AUR packages using a given AUR login/password
 */

AurVote::AurVote(QObject *parent) : QObject(parent),
  m_loginUrl(QStringLiteral("https://aur.archlinux.org/login/")),
  m_voteUrl(QStringLiteral("https://aur.archlinux.org/pkgbase/%1/vote/")),
  m_unvoteUrl(QStringLiteral("https://aur.archlinux.org/pkgbase/%1/unvote/")),
  m_pkgUrl(QStringLiteral("https://aur.archlinux.org/packages/%1/"))
{
  m_networkManager = new QNetworkAccessManager(this);
}

AurVote::~AurVote()
{

}

void AurVote::setUserName(const QString &userName)
{
  m_userName = userName;
}

void AurVote::setPassword(const QString &password)
{
  m_password = password;
}

bool AurVote::login()
{
  bool ret = false;
  QEventLoop eventLoop;
  if (m_userName.isEmpty() || m_password.isEmpty()) return false;

  QUrlQuery postData;
  postData.addQueryItem(QStringLiteral("user"), m_userName);
  postData.addQueryItem(QStringLiteral("passwd"), m_password);
  postData.addQueryItem(QStringLiteral("remember_me"), QStringLiteral("on"));

  QNetworkRequest request(m_loginUrl);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

  QNetworkReply *r = m_networkManager->post(request, postData.query().toUtf8());
  connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()) );
  eventLoop.exec();
  disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()) );
  if (r->error() > 0)
  {
    ret = false;
  }
  else
  {
    r = m_networkManager->post(request, postData.query().toUtf8());
    connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()) );
    eventLoop.exec();
    disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()) );

    QString res = r->readAll();

    if (res.contains(QLatin1String("Logout")))
    {
      ret = true;
    }
  }

  return ret;
}

bool AurVote::isLoggedIn()
{
  bool ret = false;
  QEventLoop eventLoop;
  QNetworkRequest request(m_loginUrl);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
  QNetworkReply *r = m_networkManager->get(request);
  connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()) );
  eventLoop.exec();
  disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()) );

  QString res = r->readAll();

  if (res.contains(QLatin1String("Logout")))
  {
    ret = true;
  }

  return ret;
}

/*
 * Checks if given package has been voted
 * Returns an int because of its 3-state nature:
 *
 *     0   if voted
 *     1   if not voted
 *     -1  if doesn't exist
 *
 */
int AurVote::isPkgVoted(const QString &pkgName)
{
  int ret = 0;
  QEventLoop eventLoop;
  QNetworkRequest request(m_pkgUrl.arg(pkgName));
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
  QNetworkReply *r = m_networkManager->get(request);
  connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  eventLoop.exec();
  disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));

  QString res = r->readAll();

  //If this package does not exist anymore...
  QRegularExpression re(QStringLiteral("Page Not Found"));
  if (res.contains(re)) return -1;

  QRegularExpression re1(QStringLiteral("name=\"do_Vote\" value=\"Vote for this package\""));
  if (res.contains(re1))
  {
    ret = 1;
  }

  return ret;
}

void AurVote::voteForPkg(const QString &pkgName)
{
  QEventLoop eventLoop;
  QNetworkRequest request(m_pkgUrl.arg(pkgName));
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
  QNetworkReply *r = m_networkManager->get(request);
  connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  eventLoop.exec();
  disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  QString token;

  QString res = r->readAll();

  //Get token
  QRegularExpression re(QStringLiteral("name=\"token\" value=\"(?<token>\\w+)\""));
  QRegularExpressionMatch rem;
  if (res.contains(re, &rem))
  {
    token = rem.captured(QStringLiteral("token"));
  }

  QRegularExpression re1(QStringLiteral("name=\"do_Vote\" value=\"Vote for this package\""));
  if (res.contains(re1))
  {
    QUrlQuery postData;
    postData.clear();
    postData.addQueryItem(QStringLiteral("token"), token);
    postData.addQueryItem(QStringLiteral("do_Vote"), QStringLiteral("Vote+for+this+package"));

    request.setUrl(m_unvoteUrl.arg(pkgName));
    r = m_networkManager->post(request, postData.query().toUtf8());
    connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  }
}

void AurVote::unvoteForPkg(const QString &pkgName)
{
  QEventLoop eventLoop;
  QNetworkRequest request(m_pkgUrl.arg(pkgName));
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
  QNetworkReply *r = m_networkManager->get(request);
  connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  eventLoop.exec();
  disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  QString token;

  QString res = r->readAll();

  //Get token
  QRegularExpression re(QStringLiteral("name=\"token\" value=\"(?<token>\\w+)\""));
  QRegularExpressionMatch rem;
  if (res.contains(re, &rem))
  {
    token = rem.captured(QStringLiteral("token"));
  }

  QRegularExpression re1(QStringLiteral("name=\"do_UnVote\" value=\"Remove vote\""));
  if (res.contains(re1))
  {
    QUrlQuery postData;
    postData.clear();
    postData.addQueryItem(QStringLiteral("token"), token);
    postData.addQueryItem(QStringLiteral("do_UnVote"), QStringLiteral("Remove+vote"));

    request.setUrl(m_unvoteUrl.arg(pkgName));
    r = m_networkManager->post(request, postData.query().toUtf8());
    connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  }
}

/*
 * Access AUR search site while logged in and retrieve user voted package names
 */
QStringList AurVote::getVotedPackages()
{
  QString searchUrl=QStringLiteral("https://aur.archlinux.org/packages/?O=0&SeB=nd&SB=w&SO=d&PP=250&do_Search=Go");
  QEventLoop eventLoop;
  QNetworkRequest request(searchUrl);
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
  QNetworkReply *r = m_networkManager->get(request);
  connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  eventLoop.exec();
  disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));

  QString res = r->readAll();
  res = res.remove(QRegularExpression(QStringLiteral("\\t")));
  res = res.remove(QRegularExpression(QStringLiteral("\\n")));

  QString packageToSearch=QStringLiteral("<td><a href=\"/packages/(.*)/\">(?<pkgName>\\S+)</a></td>");
  QString blockToSearch=QStringLiteral("<tr class=\"(odd|even)\">(.*)</tr>");
  QRegularExpression re(blockToSearch);
  QStringList votedPackages;

  QStringList rows = res.split(QRegularExpression(QStringLiteral("<tr class=\"(even|odd)\">")));
  foreach(QString row, rows)
  {
    if (row.contains(QLatin1String("<td>Yes</td>")))
    {
      QRegularExpression re(packageToSearch);
      QRegularExpressionMatch rem;

      if (row.contains(re, &rem))
      {
        QString votedPackage = rem.captured(QStringLiteral("pkgName"));
        votedPackages << votedPackage;
      }
    }
  }

  return votedPackages;
}
