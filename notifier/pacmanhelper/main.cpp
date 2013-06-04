#include "pacmanhelper.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  
  PacmanHelper pacmanHelper;

  return a.exec();
}
