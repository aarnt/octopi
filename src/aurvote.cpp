#include "aurvote.h"

#include <QEventLoop>
#include <QtNetwork/QNetworkReply>
#include <QUrlQuery>
#include <QRegularExpression>

AurVote::AurVote(QObject *parent) : QObject(parent),
  m_loginUrl("https://aur.archlinux.org/login/"),
  m_voteUrl("https://aur.archlinux.org/pkgbase/%1/vote/"),
  m_unvoteUrl("https://aur.archlinux.org/pkgbase/%1/unvote/"),
  m_pkgUrl("https://aur.archlinux.org/packages/%1/")
{
  m_networkManager = new QNetworkAccessManager(this);
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
  postData.addQueryItem("user", m_userName);
  postData.addQueryItem("passwd", m_password);
  postData.addQueryItem("remember_me", "on");

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

    if (res.contains("Logout"))
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

  if (res.contains("Logout"))
  {
    ret = true;
  }

  return ret;
}

bool AurVote::isPkgVoted(const QString &pkgName)
{
  bool ret = true;
  QEventLoop eventLoop;
  QNetworkRequest request(m_pkgUrl.arg(pkgName));
  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
  QNetworkReply *r = m_networkManager->get(request);
  connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  eventLoop.exec();
  disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));

  QString res = r->readAll();
  QRegularExpression re1("name=\"do_Vote\" value=\"Vote for this package\"");
  if (res.contains(re1))
  {
    ret = false;
  }

  return ret;
}

void AurVote::voteOnPkg(const QString &pkgName)
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
  QRegularExpression re("name=\"token\" value=\"(?<token>\\w+)\"");
  QRegularExpressionMatch rem;
  if (res.contains(re, &rem))
  {
    token = rem.captured("token");
  }

  QRegularExpression re1("name=\"do_Vote\" value=\"Vote for this package\"");
  if (res.contains(re1))
  {
    QUrlQuery postData;
    postData.clear();
    postData.addQueryItem("token", token);
    postData.addQueryItem("do_Vote", "Vote+for+this+package");

    request.setUrl(m_unvoteUrl.arg(pkgName));
    r = m_networkManager->post(request, postData.query().toUtf8());
    connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  }
}

void AurVote::unvoteOnPkg(const QString &pkgName)
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
  QRegularExpression re("name=\"token\" value=\"(?<token>\\w+)\"");
  QRegularExpressionMatch rem;
  if (res.contains(re, &rem))
  {
    token = rem.captured("token");
  }

  QRegularExpression re1("name=\"do_UnVote\" value=\"Remove vote\"");
  if (res.contains(re1))
  {
    QUrlQuery postData;
    postData.clear();
    postData.addQueryItem("token", token);
    postData.addQueryItem("do_UnVote", "Remove+vote");

    request.setUrl(m_unvoteUrl.arg(pkgName));
    r = m_networkManager->post(request, postData.query().toUtf8());
    connect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    disconnect(r, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  }
}
