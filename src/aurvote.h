#ifndef AURSITE_H
#define AURSITE_H

#include <QObject>

class QNetworkAccessManager;

class AurVote : public QObject
{
  Q_OBJECT

private:
  QString m_loginUrl;
  QString m_voteUrl;
  QString m_unvoteUrl;
  QString m_pkgUrl;
  QString m_userName;
  QString m_password;

  QNetworkAccessManager *m_networkManager;

public:
  explicit AurVote(QObject *parent = nullptr);

  bool login();
  bool isLoggedIn();
  bool isPkgVoted(const QString &pkgName);
  void voteOnPkg(const QString &pkgName);
  void unvoteOnPkg(const QString &pkgName);

  void setUserName(const QString &userName);
  void setPassword(const QString &password);
};

#endif // AURSITE_H
