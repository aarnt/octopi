/*
* This file is part of Octopi, an open-source GUI for ArchLinux pacman.
* Copyright (C) 2013  Alexandre Albuquerque Arnt
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

#include "treeviewpackagesitemdelegate.h"
#include "package.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "uihelper.h"
#include <QtGui>
#include <QFutureWatcher>
#include <QTreeWidget>
#include <iostream>

QPoint gPoint;
QFutureWatcher<QString> fw;
using namespace QtConcurrent;

QString showPackageInfo(QString pkgName, bool foreignPackage = false){
  return(Package::getInformationDescription(pkgName, foreignPackage));
}

TreeViewPackagesItemDelegate::TreeViewPackagesItemDelegate(QObject *parent): QStyledItemDelegate(parent){  
}

bool TreeViewPackagesItemDelegate::helpEvent ( QHelpEvent *event, QAbstractItemView*,
		const QStyleOptionViewItem&, const QModelIndex &index ){

  if (this->parent()->objectName() == "tvPackages")
  {
    QTreeView* tvPackages = qobject_cast<QTreeView*>(this->parent());
    QSortFilterProxyModel *sfp = qobject_cast<QSortFilterProxyModel*>(tvPackages->model());
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(sfp->sourceModel());

    if (sim->rowCount() == 0) return false;

    QModelIndex ind = sfp->mapToSource(index);
    QStandardItem *si = sim->itemFromIndex(ind);

    if (si)
    {
      //If the user's mouse is not positioned above the name column, let's give him a little help...
      if (si->column() != ctn_PACKAGE_NAME_COLUMN)
      {
        QModelIndex miName = sim->index(si->row(), ctn_PACKAGE_NAME_COLUMN);
        si = sim->itemFromIndex(miName);
      }

      QPoint p;
      gPoint = tvPackages->mapToGlobal(event->pos());
      QFuture<QString> f;

      QModelIndex mi = sim->index(si->row(), ctn_PACKAGE_REPOSITORY_COLUMN);
      QStandardItem *siRepo = sim->itemFromIndex(mi);

      bool foreignPackage = false;
      if(siRepo)
      {
        if (siRepo->text().isEmpty()) foreignPackage = true;
      }

      f = run(showPackageInfo, si->text(), foreignPackage);
      fw.setFuture(f);
      connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
    }
    else return false;
  }
  else if (this->parent()->objectName() == "tvTransaction")
  {
    QTreeView* tvTransaction = qobject_cast<QTreeView*>(this->parent());
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tvTransaction->model());

    if (sim->rowCount() == 0) return false;

    QStandardItem *si = sim->itemFromIndex(index);

    if (si)
    {
      if (si->icon().isNull()) //If it's really a package in the Transaction treeview...
      {
        QStandardItemModel *modelPackages = MainWindow::returnMainWindow()->getModelPackages();
        QList<QStandardItem*> foundItems = modelPackages->findItems(si->text(), Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);

        if (foundItems.count() > 0)
        {
          QStandardItem *siFound = foundItems.at(0);
          QStandardItem *siRepository = modelPackages->item(siFound->row(), ctn_PACKAGE_REPOSITORY_COLUMN);

          bool foreignPackage = (siRepository->text().isEmpty());

          QPoint p;
          gPoint = tvTransaction->mapToGlobal(event->pos());
          QFuture<QString> f;

          f = run(showPackageInfo, siFound->text(), foreignPackage);
          fw.setFuture(f);
          connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
        }
      }
      else
      {
        QToolTip::hideText();
      }
    }
  }

  return true;

/*
  if ( ((this->parent()->objectName() == "tvPackage") ||
        (this->parent()->objectName() == "tvInstalledPackages")) && (index.isValid()) ) {
		QTreeView* tvPackages = qobject_cast<QTreeView*>(this->parent());
		QSortFilterProxyModel *sfp = qobject_cast<QSortFilterProxyModel*>(tvPackages->model());		
		QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(sfp->sourceModel());		

    if (sim->rowCount() == 0) return false;

		QModelIndex ind = sfp->mapToSource(index);
		QStandardItem *si = sim->itemFromIndex(ind);
		SelectedPackage sp;   
    QFileInfo fi;

    if (this->parent()->objectName() == "tvPackage"){
      sp = SelectedPackage( MainWindowImpl::returnMainWindow()->getSelectedDirectory(), si->text() );
      fi = QFileInfo(sp.getCompleteFileName());
      if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP)
        m_PkgClassification = getPackageClassification(sp.getFileName());
    }
    else{      
      sp = SelectedPackage( ctn_PACKAGES_DIR, si->text() );
      QFileInfo fi(sp.getCompleteFileName());
      if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP)
        m_PkgClassification = getPackageClassification(sp.getFileName());
    }
		
		gPoint = tvPackages->mapToGlobal(event->pos()); 
		QFuture<QString> f;
    if (this->parent()->objectName() == "tvPackage"){
      fi = QFileInfo(sp.getCompleteFileName());
      if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP)
        f = run(showPackageInfo, sp.getCompleteFileName(), false);
      else return true;
    }
		else f = run(showPackageInfo, sp.getCompleteFileName(), true);

    if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP){
      fw.setFuture(f);
      connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
    }
	}
  else if (this->parent()->objectName() == "twFindResults" && index.isValid()) {
    FindDialogImpl *findDialog = qobject_cast<FindDialogImpl*>(this->parent()->parent());
    QTreeWidget* twFind = qobject_cast<QTreeWidget*>(this->parent());
    QTreeWidgetItem* twItem = twFind->itemAt(event->pos());

    gPoint = twFind->mapToGlobal(event->pos());
    QFuture<QString> f;

    if (findDialog->getSearchPlace() == ectn_INSIDE_DIRECTORY){
      if (twItem->icon(0).pixmap(QSize(22,22)).toImage() != IconHelper::getIconFolder().pixmap(22,22).toImage() ){
        QString packageDirectory = twItem->parent()->text(0);
        QString packageName = twItem->text(0);

        QFileInfo fi(packageDirectory + QDir::separator() + packageName);
        if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP){
          m_PkgClassification = getPackageClassification(packageName);
          f = run(showPackageInfo, packageDirectory + QDir::separator() + packageName, false);
          fw.setFuture(f);
          connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
        }
      }
    }
    else if (findDialog->getSearchPlace() == ectn_INSIDE_INSTALLED_PACKAGES){
      if (twItem->icon(0).pixmap(QSize(22,22)).toImage() == IconHelper::getIconUnFrozen().pixmap(22,22).toImage()
          || twItem->icon(0).pixmap(QSize(22,22)).toImage() == IconHelper::getIconFrozen().pixmap(22,22).toImage()){

        QString packageDirectory = ctn_INSTALLED_PACKAGES_DIR;
        QString packageName = twItem->text(0);

        m_PkgClassification = getPackageClassification(packageName);

        f = run(showPackageInfo, packageDirectory + QDir::separator() + packageName, true);
        fw.setFuture(f);
        connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
      }
    }
  }
  else if (this->parent()->objectName() == "tvTODO" && index.isValid()){
    QTreeView* tvTodo = qobject_cast<QTreeView*>(this->parent());
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tvTodo->model());

    if (sim->rowCount() == 0) return false;

    QStandardItem *si = sim->itemFromIndex(index);
    QPoint p;
    gPoint = tvTodo->mapToGlobal(event->pos());
    QFuture<QString> f;
    QFileInfo fi (si->text());

    if (si->text().indexOf(ctn_PACKAGES_DIR) == 0){
      m_PkgClassification = ectn_INSTALLED;
      f = run(showPackageInfo, si->text(), true);
    }
    else{
      if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP){
        m_PkgClassification = getPackageClassification(fi.fileName());
        f = run(showPackageInfo, si->text(), false);
      }
    }

    if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP){
      fw.setFuture(f);
      connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
    }
  }

  return true;*/  
}

void TreeViewPackagesItemDelegate::execToolTip(){
  if (fw.result().isEmpty()) return;

  gPoint.setX(gPoint.x() + 25);
	gPoint.setY(gPoint.y() + 25);

  /*if (m_PkgClassification == ectn_INSTALLED || m_PkgClassification == ectn_OTHER_ARCH )
    qApp->setStyleSheet(StrConstants::getToolTipNormalCSS());
  else if (m_PkgClassification == ectn_OTHER_VERSION)
    qApp->setStyleSheet(StrConstants::getToolTipYellowCSS());
  else if (m_PkgClassification == ectn_INFERIOR_VERSION)
    qApp->setStyleSheet(StrConstants::getToolTipRedCSS());
  else if (m_PkgClassification == ectn_SUPERIOR_VERSION)
    qApp->setStyleSheet(StrConstants::getToolTipGreenCSS());
  else if (m_PkgClassification == ectn_NOT_INSTALLED)
    qApp->setStyleSheet(StrConstants::getToolTipBlankCSS());
  else if (m_PkgClassification == ectn_FROZEN)
    qApp->setStyleSheet(StrConstants::getToolTipBlueCSS());*/

  qApp->setStyleSheet(StrConstants::getToolTipNormalCSS());
  QToolTip::showText(gPoint, fw.result());
}
