#include "pacmanhelper.h"
#include "pacmanhelperadaptor.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QProcess>
#include <QDebug>

PacmanHelper::PacmanHelper(QObject *parent) :
  QObject(parent)
{
  (void) new PacmanHelperAdaptor(this);

  if (!QDBusConnection::systemBus().registerService("org.octopi.pacmanhelper")) {
      qDebug() << "Another helper is already running!";
      QCoreApplication::instance()->quit();
  }

  if (!QDBusConnection::systemBus().registerObject("/", this)) {
      qDebug() << "Unable to register service interface to dbus!";
      QCoreApplication::instance()->quit();
  }
}

void PacmanHelper::syncdb()
{
  QProcess pacman;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  pacman.setProcessEnvironment(env);

  QString command = "pacman -Syy";
  pacman.execute(command);

  emit syncdbcompleted();
}
