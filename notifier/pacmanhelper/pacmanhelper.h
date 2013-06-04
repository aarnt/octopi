#ifndef PACMANHELPER_H
#define PACMANHELPER_H

#include <QObject>
#include <QtDBus/QDBusContext>

class PacmanHelper : public QObject, protected QDBusContext
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.octopi.pacmanhelper")
public:
  explicit PacmanHelper(QObject *parent = 0);
  
public slots:
  void syncdb();

signals:
  void syncdbcompleted();
};

#endif // PACMANHELPER_H
