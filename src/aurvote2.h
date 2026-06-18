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

  AurVote2()
  {
    manager.setCookieJar(new QNetworkCookieJar(&manager));
  }

  void setUserName(const QString& user)
  {
    m_username = user;
  }

  void setPassword(const QString& pass)
  {
    m_password = pass;
  }

  bool testLogin()
  {
    return login();
  }

  bool isLoggedIn()
  {
    QString html = get(LOGIN_URL);

    return html.contains(QStringLiteral("/logout"));
  }

  bool login()
  {
    QUrl url(LOGIN_URL);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("user"), m_username);
    query.addQueryItem(QStringLiteral("passwd"), m_password);
    query.addQueryItem(QStringLiteral("next"), QStringLiteral("/"));

    QByteArray data =
        query.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest req(url);

    req.setHeader(
        QNetworkRequest::ContentTypeHeader,
        QStringLiteral("application/x-www-form-urlencoded"));

    req.setRawHeader(
        "referer",
        LOGIN_URL.toLatin1());

    QString html = post(req, data);

    return html.contains(QStringLiteral("/logout"));
  }

  /*
   * Returns
   *    0  if pkg is voted
   *    1  if pkg is not voted
   *    -1 if pkg is not found
   */
  int isPkgVoted(const QString& pkg)
  {
    int ret = -1;
    bool logged = isLoggedIn();

    if (!logged)
      logged = login();

    if (logged)
    {
      QString html = get(PACKAGES_URL.arg(pkg));

      if (html.contains(QStringLiteral("error-page")))
        return -1;

      if (html.contains(QStringLiteral("Remove vote"))) ret = 0;
      else ret = 1;
    }

    return ret;
  }

  /*QString getPkgBase(const QString& pkg)
  {
    QString html =
        get(PACKAGES_URL.arg(pkg));

    if (html.contains(QStringLiteral("error-page")))
      return QStringLiteral("");

    QRegularExpression re(
        R"REGEX(href="/pkgbase/([^"]+)")REGEX");

    auto match = re.match(html);

    if (!match.hasMatch())
      return QStringLiteral("");

    return match.captured(1);
  }*/

  bool voteForPkg(const QString& pkgBase)
  {
    bool logged = isLoggedIn();

    if (!logged)
      logged = login();

    if (logged)
    {
      QNetworkRequest req(
          QUrl(VOTE_URL.arg(pkgBase)));

      QUrlQuery q;
      q.addQueryItem(
          QStringLiteral("do_Vote"),
          QStringLiteral("Vote for this package"));

      QByteArray data = q.toString(QUrl::FullyEncoded).toUtf8();

      QString html = post(req, data);

      return !html.isEmpty();
    }
    else return false;
  }

  bool unvoteForPkg(const QString& pkgBase)
  {
    bool logged = isLoggedIn();

    if (!logged)
      logged = login();

    if (logged)
    {
      QString s = UNVOTE_URL.arg(pkgBase);
      QNetworkRequest req(
          QUrl(UNVOTE_URL.arg(pkgBase)));

      QUrlQuery q;
      q.addQueryItem(
          QStringLiteral("do_UnVote"),
          QStringLiteral("Remove vote"));

      QByteArray data = q.toString(QUrl::FullyEncoded).toUtf8();

      QString html = post(req, data);

      return !html.isEmpty();
    }
    else return false;
  }

  QStringList getVotedPackages()
  {
    QStringList votedPackages;
    int offset = 0;
    bool logged = isLoggedIn();

    if (!logged)
      logged = login();

    if (logged)
    {
      QString html = get(SEARCH_URL_TEMPLATE.arg(offset));

      if (html.isEmpty())
        return votedPackages;

      html = html.remove(QRegularExpression(QStringLiteral("\\t")));
      html = html.remove(QRegularExpression(QStringLiteral("\\n")));

      //<td>\n                    <a href=\"/packages/zoneminder\">\n
      //<td>                    <a href="/packages/yay-bin">                        yay-bin                    </a>                </td
      QString packageToSearch = QStringLiteral("<a href=\"/packages/(?<pkgName>\\S+)\">");
      QString blockToSearch = QStringLiteral("<tr>(.*)</tr>");
      QRegularExpression re(blockToSearch);

      QStringList rows = html.split(QRegularExpression(QStringLiteral("<tr>")));
      for(const QString& row: rows)
      {
        if (row.contains(QRegularExpression(QStringLiteral("<td>\\s*Yes\\s*</td>"))))
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
    }

    if (!votedPackages.isEmpty())
      votedPackages.sort();

    return votedPackages;
  }

  private:

  QString m_username;
  QString m_password;
  QNetworkAccessManager manager;

  QString get(const QString& url)
  {
    const QNetworkRequest req{QUrl(url)};

    QNetworkReply* reply =
        manager.get(req);

    QEventLoop loop;

    QObject::connect(
        reply,
        &QNetworkReply::finished,
        &loop,
        &QEventLoop::quit);

    loop.exec();

    QString data = QString::fromUtf8(reply->readAll());

    reply->deleteLater();

    return data;
  }

  QString post(const QNetworkRequest& req,
               const QByteArray& data)
  {
    QNetworkReply* reply =
        manager.post(req, data);

    QEventLoop loop;

    QObject::connect(
        reply,
        &QNetworkReply::finished,
        &loop,
        &QEventLoop::quit);

    loop.exec();

    QString result = QString::fromUtf8(reply->readAll());

    reply->deleteLater();

    return result;
  }
};

#endif // AURVOTE2_H
