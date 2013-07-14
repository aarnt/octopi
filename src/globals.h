#ifndef MAINWINDOW_GLOBALS_H
#define MAINWINDOW_GLOBALS_H

#include "strconstants.h"

#include <QStandardItem>
#include <QFutureWatcher>
#include <QtConcurrentRun>

extern QFutureWatcher<QString> g_fwToolTip;
extern QFutureWatcher<QList<PackageListData> *> g_fwPacman;
extern QFutureWatcher<QList<QString> *> g_fwPacmanGroup;
extern QFutureWatcher<QList<PackageListData> *> g_fwYaourt;
extern QFutureWatcher<QList<PackageListData> *> g_fwYaourtMeta;

QString showPackageInfo(QString pkgName);
QList<PackageListData> * searchPacmanPackages();
QList<QString> * searchPacmanPackagesFromGroup(QString groupName);
QList<PackageListData> * searchYaourtPackages(QString searchString);

#endif // MAINWINDOW_GLOBALS_H
