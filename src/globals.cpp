#include "globals.h"
#include "mainwindow.h"

#include <QStandardItem>
#include <QFutureWatcher>
#include <QtConcurrentRun>

QFutureWatcher<QString> g_fwToolTip;
QFutureWatcher<QList<PackageListData> *> g_fwPacman;
QFutureWatcher<QList<QString> *> g_fwPacmanGroup;
QFutureWatcher<QList<PackageListData> *> g_fwYaourt;
QFutureWatcher<QList<PackageListData> *> g_fwYaourtMeta;

/*
 * Given a packageName, returns its description
 */
QString showPackageInfo(QString pkgName)
{
  MainWindow *mw = MainWindow::returnMainWindow();
  QStandardItem * si = mw->getAvailablePackage(
    pkgName, ctn_PACKAGE_DESCRIPTION_COLUMN);

  if (si == 0) return "";

  QString description = si->text();

  int space = description.indexOf(" ");
  QString desc = description.mid(space+1);
  int size = desc.size();
  if (desc.size() > 120)
  {
    desc.chop(size - 120);
    desc = desc + " ...";
  }

  return desc;
}

/*
 * Starts the non blocking search for Pacman packages...
 */
QList<PackageListData> * searchPacmanPackages()
{
  return Package::getPackageList();
}

/*
 * Starts the non blocking search for Pacman packages...
 */
QList<QString> * searchPacmanPackagesFromGroup(QString groupName)
{
  return Package::getPackagesOfGroup(groupName);
}

/*
 * Starts the non blocking search for Yaourt packages...
 */
QList<PackageListData> * searchYaourtPackages(QString searchString)
{
  return Package::getYaourtPackageList(searchString);
}
