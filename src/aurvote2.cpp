#include "aurvote2.h"

AurVote2::AurVote2()
{
  manager.setCookieJar(new QNetworkCookieJar(&manager));
}

void AurVote2::setUserName(const QString& user)
  {
    m_username = user;
  }

  void AurVote2::setPassword(const QString& pass)
  {
    m_password = pass;
  }

  bool AurVote2::testLogin()
  {
    return login();
  }

  bool AurVote2::isLoggedIn()
  {
    QString html = get(LOGIN_URL);

    return html.contains(QStringLiteral("/logout"));
  }

  bool AurVote2::login()
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
  int AurVote2::isPkgVoted(const QString& pkg)
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

  bool AurVote2::voteForPkg(const QString& pkgBase)
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

  bool AurVote2::unvoteForPkg(const QString& pkgBase)
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

  QStringList AurVote2::getVotedPackages()
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


  QString AurVote2::get(const QString& url)
  {
    const QNetworkRequest req{QUrl(url)};

    QNetworkReply* reply = manager.get(req);

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

  QString AurVote2::post(const QNetworkRequest& req, const QByteArray& data)
  {
    QNetworkReply* reply = manager.post(req, data);

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
