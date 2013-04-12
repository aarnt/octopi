/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2013 Alexandre Albuquerque Arnt
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#include <iostream>
#include "packagecontroller.h"
#include "package.h"
#include <QDirIterator>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QCoreApplication>

QString PackageController::showFullPathOfItem( const QModelIndex &index ){
  QString str;
  if (!index.isValid()) return str;

  const QStandardItemModel *sim = qobject_cast<const QStandardItemModel*>( index.model() );
  QStringList sl;
  QModelIndex nindex;
  sl << sim->itemFromIndex( index )->text();

  nindex = index;
  while (1){
    nindex = sim->parent( nindex );
    if ( nindex != sim->invisibleRootItem()->index() ) sl << sim->itemFromIndex( nindex )->text();
    else break;
  }
  str = QDir::separator() + str;

  for ( int i=sl.count()-1; i>=0; i-- ){
    if ( i < sl.count()-1 ) str += QDir::separator();
    str += sl[i];
  }

  return str;
}

QMap<QString, QStringList> PackageController::findFile( const QString& name ){
  FILE *file;
  char linebuf[1024];
  QString st;
  QString fn, dr = ctn_INSTALLED_PACKAGES_DIR;
  QMap<QString, QStringList> m;

  QDir d(ctn_INSTALLED_PACKAGES_DIR);
  if ( d.exists() ){

      QStringList slAux;
      const QFileInfoList list = d.entryInfoList();
      foreach ( QFileInfo fi, list )      {
        QCoreApplication::processEvents();

        if ( !fi.isDir() && fi.isReadable() ){
          fn = dr + fi.fileName();
          file = fopen ( QFile::encodeName ( fn ),"r" );
          if ( file ){
            QStringList sl;
            while ( fgets ( linebuf,sizeof ( linebuf ),file ) ){
              if ( !strcmp ( linebuf, ctn_FILELIST ) ){
                break;
              }
            }
            while ( fgets ( linebuf,sizeof ( linebuf ),file ) ){
              QString fileName = QString::fromLocal8Bit ( linebuf );
              QFileInfo f(fileName);
              if (f.fileName().simplified().indexOf(QRegExp(name, Qt::CaseInsensitive) ) != -1){
                st = "/";
                st += linebuf;
                st.truncate ( st.length() -1 );
                if ( st.left ( 8 ) != "/install" ){
                  if (m.contains(fi.fileName())){
                    slAux = m.value(fi.fileName());
                    slAux.append(st);
                    m.insert(fi.fileName(), slAux);
                  }
                  else
                    m.insert(fi.fileName(), sl << st);

                  sl.clear();
                }
              }
            }
            fclose ( file );
          }
        }
      }
    }

  return m;
}

QMap<QString, QStringList> PackageController::findFile( const QString& name, const QStandardItemModel *sim){
  QMap<QString, QStringList> m;
  QList<QStandardItem *> foundItems;
  QString fileName;
  QStringList listAux;

  if (sim->rowCount() == 0) return m;

  foundItems = sim->findItems(name, Qt::MatchRegExp|Qt::MatchRecursive, 0);
  QString fullPath;

  foreach(QStandardItem *item, foundItems){
    QCoreApplication::processEvents();

    if (item->accessibleDescription().contains("directory")) continue;
    if (item->parent()){
      fileName = item->text();
      fullPath = showFullPathOfItem(item->parent()->index());
    }
    if (item->parent() && item->parent()->hasChildren() &&
        item->parent() != item->model()->invisibleRootItem() &&
        m.contains(fullPath)){

      listAux = m.value(fullPath);
      listAux << fileName;
      m.insert(fullPath, listAux);
    }
    else if (item->parent() && item->parent() != item->model()->invisibleRootItem())
      m.insert(fullPath, listAux << fileName);

    listAux.clear();
  }

  return m;
}

QList<QModelIndex> * PackageController::findFileEx( const QString& name, const QStandardItemModel *sim)
{
  QList<QModelIndex> * res = new QList<QModelIndex>();
  QList<QStandardItem *> foundItems;

  if (name.isEmpty() || sim->rowCount() == 0)
  {
    return res;
  }

  foundItems = sim->findItems(Package::parseSearchString(name), Qt::MatchRegExp|Qt::MatchRecursive);
  foreach(QStandardItem *item, foundItems)
  {
    //QCoreApplication::processEvents();

    if (item->accessibleDescription().contains("directory")) continue;

    res->append(item->index());
  }

  return res;
}

/*QMap<QString, QStringList> PackageController::findPackage( const QString& name, const QString& searchDir ){
  QMap<QString, QStringList> m;

  QStringList nameFilters;
  nameFilters << "*" + ctn_TGZ_PACKAGE_EXTENSION <<
                 "*" + ctn_TXZ_PACKAGE_EXTENSION <<
                 "*" + ctn_RPM_PACKAGE_EXTENSION;

  QDirIterator *targetDir = new QDirIterator(searchDir, nameFilters,
                                             QDir::AllEntries,
                                             QDirIterator::Subdirectories);
  QString res;
  QString dir;
  QStringList fileNames, slAux;

  while (targetDir->hasNext()){
    QCoreApplication::processEvents();
    targetDir->next();
    res = targetDir->fileName();
    QFileInfo fi = targetDir->fileInfo();
    dir = fi.absolutePath();

    if (res.indexOf( QRegExp(name, Qt::CaseInsensitive)) != -1){
      fileNames << res;
      if (m.contains(dir)){
        slAux = m.value(dir);
        slAux.append(res);
        m.insert(dir, slAux);
      }
      else m.insert(dir, fileNames);

      fileNames.clear();
    }
  }

  delete targetDir;
  return m;
}*/

void PackageController::testSearchInDir(){
  /*QMap<QString, QStringList> map = findPackage("qt-", "/home/arnt/Packages");

  foreach (QString k, map.keys()){
    foreach( QStringList sl, map.values(k) ){
      for ( int c=0; c<sl.count(); c++ ){
        QString found(sl[c]);
        std::cout << "Found!" << k.toAscii().data() << "/" << found.toAscii().data() << std::endl;
      }
    }
  }*/
}
