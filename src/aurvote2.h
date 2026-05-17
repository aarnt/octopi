#ifndef AURVOTE2_H
#define AURVOTE2_H

/*
 * Modern AUR login/vote helper
 * Qt5 / Qt6 compatible
 */

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QDebug>

class AurClient : public QObject
{
  Q_OBJECT

  public:
  explicit AurClient(QObject *parent = nullptr)
      : QObject(parent)
  {
    m_manager = new QNetworkAccessManager(this);

    m_manager->setCookieJar(
        new QNetworkCookieJar(this));

    m_manager->setRedirectPolicy(
        QNetworkRequest::NoLessSafeRedirectPolicy);
  }

  void setCredentials(const QString &user,
                      const QString &password)
  {
    m_user = user;
    m_password = password;
  }

  bool login()
  {
    //------------------------------------------
    // STEP 1: GET LOGIN PAGE
    //------------------------------------------

    QString loginUrl = QStringLiteral("https://aur.archlinux.org/login");

    QNetworkRequest getReq{QUrl(loginUrl)};

    setupHeaders(getReq);

    QNetworkReply *reply =
        blockingGet(getReq);

    if (!reply)
      return false;

    if (reply->error())
    {
      qDebug() << "GET login error:"
               << reply->errorString();

      reply->deleteLater();
      return false;
    }

    QString html =
        QString::fromUtf8(reply->readAll());

    reply->deleteLater();

    //------------------------------------------
    // STEP 2: EXTRACT CSRF TOKEN
    //------------------------------------------

    QString token =
        extractToken(html);

    if (token.isEmpty())
    {
      qDebug() << "Failed to extract CSRF token";
      return false;
    }

    qDebug() << "CSRF token:" << token;

    //------------------------------------------
    // STEP 3: POST LOGIN
    //------------------------------------------

    QUrlQuery postData;

    postData.addQueryItem(QStringLiteral("user"), m_user);
    postData.addQueryItem(QStringLiteral("passwd"), m_password);
    postData.addQueryItem(QStringLiteral("remember_me"), QStringLiteral("on"));
    postData.addQueryItem(QStringLiteral("next"), QStringLiteral("/"));
    postData.addQueryItem(QStringLiteral("token"), token);

    QByteArray body =
        postData.toString(
                    QUrl::FullyEncoded).toUtf8();

    QNetworkRequest postReq{QUrl(loginUrl)};

    setupHeaders(postReq);

    postReq.setHeader(
        QNetworkRequest::ContentTypeHeader,
        QStringLiteral("application/x-www-form-urlencoded"));

    reply =
        blockingPost(postReq, body);

    if (!reply)
      return false;

    if (reply->error())
    {
      qDebug() << "POST login error:"
               << reply->errorString();

      reply->deleteLater();
      return false;
    }

    QString response =
        QString::fromUtf8(reply->readAll());

    reply->deleteLater();

    //------------------------------------------
    // STEP 4: VERIFY LOGIN
    //------------------------------------------

    bool ok =
        response.contains(QStringLiteral("Logout"),
            Qt::CaseInsensitive);

    qDebug() << QStringLiteral("Login result:") << ok;

    return ok;
  }

  QStringList votedPackages()
  {
    QString url = QStringLiteral("https://aur.archlinux.org/packages/?O=0&SeB=nd&SB=w&SO=d&PP=250&do_Search=Go");

    QNetworkRequest req{QUrl(url)};

    setupHeaders(req);

    QNetworkReply *reply =
        blockingGet(req);

    QStringList result;

    if (!reply)
      return result;

    if (reply->error())
    {
      qDebug() << reply->errorString();

      reply->deleteLater();
      return result;
    }

    QString html =
        QString::fromUtf8(reply->readAll());

    reply->deleteLater();

    //------------------------------------------
    // Parse voted packages
    //------------------------------------------

    html.remove(QStringLiteral("\n"));
    html.remove(QStringLiteral("\t"));

    QStringList rows =
        html.split(QStringLiteral("<tr"));

    for (const QString &row : rows)
    {
      if (!row.contains(QStringLiteral(">Yes<")))
        continue;

      QRegularExpression rx(QStringLiteral("/packages/([^\"/]+)/"));
      //QRegularExpression rx(
      //    R"(/packages/([^"/]+)/)");

      QRegularExpressionMatch m =
          rx.match(row);

      if (m.hasMatch())
      {
        result << m.captured(1);
      }
    }

    return result;
  }

  bool votePackage(const QString &pkg)
  {
    return performVoteAction(
        pkg,
        true);
  }

  bool unvotePackage(const QString &pkg)
  {
    return performVoteAction(
        pkg,
        false);
  }

  private:

  QString m_user;
  QString m_password;

  QNetworkAccessManager *m_manager {};

  //------------------------------------------
  // Helpers
  //------------------------------------------

  void setupHeaders(QNetworkRequest &req)
  {
    req.setHeader(
        QNetworkRequest::UserAgentHeader,
        QStringLiteral("Mozilla/5.0"));

    req.setRawHeader(
        "Accept", "text/html,application/xhtml+xml");

    req.setRawHeader(
        "Accept-Language",
        "en-US,en;q=0.9");

    req.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);
  }

  QString extractToken(const QString &html)
  {
    //QRegularExpression rx(
    //    R"(name="token"\s+value="([^"]+)")");

    QRegularExpression rx(QStringLiteral("name=\"token\"\\s+value=\"([^\"]+)\""));

    QRegularExpressionMatch m = rx.match(html);

    if (!m.hasMatch())
      return {};

    return m.captured(1);
  }

  QNetworkReply *blockingGet(
      const QNetworkRequest &req)
  {
    QEventLoop loop;

    QNetworkReply *reply =
        m_manager->get(req);

    connect(reply,
            &QNetworkReply::finished,
            &loop,
            &QEventLoop::quit);

    loop.exec();

    return reply;
  }

  QNetworkReply *blockingPost(
      const QNetworkRequest &req,
      const QByteArray &body)
  {
    QEventLoop loop;

    QNetworkReply *reply =
        m_manager->post(req, body);

    connect(reply,
            &QNetworkReply::finished,
            &loop,
            &QEventLoop::quit);

    loop.exec();

    return reply;
  }

  bool performVoteAction(
      const QString &pkg,
      bool vote)
  {
    //------------------------------------------
    // GET PACKAGE PAGE
    //------------------------------------------

    QString pkgUrl =
        QString(
                         QStringLiteral("https://aur.archlinux.org/packages/%1/"))
            .arg(pkg);

    QNetworkRequest req{QUrl(pkgUrl)};

    setupHeaders(req);

    QNetworkReply *reply =
        blockingGet(req);

    if (!reply)
      return false;

    if (reply->error())
    {
      qDebug() << reply->errorString();

      reply->deleteLater();
      return false;
    }

    QString html =
        QString::fromUtf8(reply->readAll());

    reply->deleteLater();

    //------------------------------------------
    // Extract token
    //------------------------------------------

    QString token =
        extractToken(html);

    if (token.isEmpty())
    {
      qDebug() << "Failed to get CSRF token";
      return false;
    }

    //------------------------------------------
    // POST vote/unvote
    //------------------------------------------

    QString actionUrl =
        QString(
            QStringLiteral("https://aur.archlinux.org/pkgbase/%1/%2/"))
            .arg(pkg)
                            .arg(vote ? QStringLiteral("vote")
                      : QStringLiteral("unvote"));

    QUrlQuery postData;

    postData.addQueryItem(
        QStringLiteral("token"),
        token);

    QByteArray body =
        postData.toString(
                    QUrl::FullyEncoded).toUtf8();

    QNetworkRequest postReq{
      QUrl(actionUrl)
    };

    setupHeaders(postReq);

    postReq.setHeader(
        QNetworkRequest::ContentTypeHeader,
        QStringLiteral("application/x-www-form-urlencoded"));

    reply =
        blockingPost(postReq, body);

    if (!reply)
      return false;

    bool ok =
        !reply->error();

    if (!ok)
    {
      qDebug() << reply->errorString();
    }

    QString response =
        QString::fromUtf8(reply->readAll());

    qDebug() << response.left(500);

    reply->deleteLater();

    return ok;
  }
};

#endif // AURVOTE2_H
