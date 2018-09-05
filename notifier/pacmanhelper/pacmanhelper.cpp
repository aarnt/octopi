#include "pacmanhelper.h"
#include "pacmanhelperadaptor.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QProcess>
#include <QDebug>

#include <QFile>

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
  env.insert("LANG", "en_US");
  pacman.setProcessEnvironment(env);

  QStringList params;
  params << "-Sy"; //-Syy or -Sy - eternal doubt!
  QString command = "/usr/bin/pacman";
  /*int code = */pacman.execute(command, params);

  /*QString out;
  out += "Return code of pacman command: " + QString::number(code) + "\n";
  out += "Return of pacman command (stdout): " + pacman.readAllStandardOutput() + "\n";
  out += "Return of pacman command (stderr): " + pacman.readAllStandardError() + "\n\n";

  QString filename = QDir::homePath() + QDir::separator() + "output_pacmanhelper.txt";
  QFile file( filename );
  if ( file.open(QIODevice::ReadWrite) )
  {
    QTextStream stream( &file );
    stream << out << endl;
  }*/

  emit syncdbcompleted();
}
