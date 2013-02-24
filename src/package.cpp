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

#include "package.h"
#include "unixcommand.h"
#include "stdlib.h"
#include <QTextStream>
#include <iostream>

InstalledPkgListSingleton *InstalledPkgListSingleton::m_pinstance = 0;
FrozenPkgListSingleton *FrozenPkgListSingleton::m_pinstance = 0;

QStringList Package::getInstalledPackageNames(){
  QDir d(ctn_PACKAGES_DIR);
  d.setSorting(QDir::Name);
  d.setFilter(QDir::Files);

  return d.entryList();
}

bool Package::isValid( const QString& pkgName ){
	QFileInfo fi(pkgName);
  if (fi.size() < 100) return false;
  else return true;
}

QString Package::getBaseName( const QString& p ){
	QString packageBaseName="";
	QString aux(p);
	int numberOfSegments = p.count('-')+1;
	int nameSegment = numberOfSegments-3;
	int a;

	for (int i=1; i<= nameSegment; i++){
		a=aux.indexOf("-");
		packageBaseName += aux.left(a);
		if (i<nameSegment) packageBaseName += "-";
		aux = aux.mid(a+1);
	}

	if (packageBaseName == "") packageBaseName += p.left(p.indexOf("-"));
	return packageBaseName;
}

QString Package::dumpInstalledPackageList(DumpInstalledPackageListOptions options){
	QDateTime now = QDateTime::currentDateTime();
  QString dumpFileName = ctn_DUMP_FILE + now.toString ("dd_MM_yyyy_hh_mm_ss") + ".txt"; 
  QFile file(SettingsManager::getDefaultDirectory() + QDir::separator() + dumpFileName);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return "";
  QTextStream out(&file);

  foreach (QString fileName, InstalledPkgListSingleton::instance()->getFileList()){
    if (options == ectn_WITH_MODIFIED_DATE){
      QFileInfo fi( ctn_PACKAGES_DIR + QDir::separator() + fileName );
      QDateTime lastModifiedDate = fi.lastModified();
      out << fileName << " " << lastModifiedDate.toString("dd/MM/yyyy hh:mm:ss") << "\n";
    }
    else if (options == ectn_NO_MODIFIED_DATE){
      out << fileName << "\n";
    }
	}

	file.flush();
	file.close();

  return SettingsManager::getDefaultDirectory() + QDir::separator() + dumpFileName;
}

//Here, we do the URL html link tag addition
QString Package::makeURLClickable( const QString &s ){
	QString sb = s;

	QRegExp rx("((ht|f)tp(s?))://(\\S)+[^\"|)|(|.|\\s|\\n]");
	QRegExp rx1("^|[\\s]+(www\\.)(\\S)+[^\"|)|(|.|\\s|\\n]"); 
	rx.setCaseSensitivity( Qt::CaseInsensitive );
	rx1.setCaseSensitivity( Qt::CaseInsensitive );

	int search = 0;
	int ini = 0;

	//First we search for the 1st pattern: rx
	while ( (ini = rx.indexIn( sb, search )) != -1 ){
		QString s1 = rx.cap();
		QString ns = "<a href=\"" + s1 + "\">" + s1 + "</a>";
		sb.replace( ini, s1.length(), ns);
		search = ini + (2*s1.length()) + 15;	
	}

	search = 0;
	ini = 0;
	//Now, we search for the 2nd pattern: rx1
	while ( (ini = rx1.indexIn( sb, search )) != -1 ){
		QString s1 = rx1.cap();
		QString ns;
		if (s1[0] == '\n') ns += "\n";

		int blanks = s1.count(	QRegExp("^|[\\s]+") );
		for (int i=0; i<blanks; i++) ns += " ";

		ns += "<a href=\"http://" + s1.trimmed() + "\">" + s1.trimmed() + "</a>";
		sb.replace( ini, s1.length(), ns);
		search = ini + (2*s1.length()) + 15;	
	}

	sb.replace(QRegExp("\n"), "<br>");
  return "<p align='left'>" + sb + "</p>"; //p style='white-space:pre
}

//Test if the file is indeed a Slackware compatible package
bool Package::isSlackPackage(const QString &filePath) {
  return UnixCommand::isSlackPackage(filePath);
}

// Regular expression for "http://" -> ^((ht|f)tp(s?))\://([0-9a-zA-Z\-]+\.)+[a-zA-Z]{2,6}(\:[0-9]+)?(/\S*)?$
QString Package::getInformation( QString pkgName, bool installed ) {
	QFileInfo fi( pkgName );    

	if ( installed == false ) {
    QString pname = Package::getBaseName( fi.fileName() );
    QString sb = UnixCommand::getPackageInformation(pkgName, installed);
    QString t = pname + ":";
    //tar.close();
    int x = sb.indexOf(t, 0);
   	if (x > 0) sb.remove(0, x);
   	int begin=0;
   	x = sb.indexOf(t);

    while(x >= 0){
   		if (x==0) sb.remove(x, x+t.length()+1);
   		else {
        if (!sb.mid(x-1, x).contains("\\S")) sb.remove(x, (x+t.length()+1)-x);
        else begin += pname.size();
   		}
    	x = sb.indexOf(t, begin);
    }	

		if ( !sb.isEmpty() ) return makeURLClickable(sb);
		else return 0;
	}
	else if ( installed == true ){
    QString pname = Package::getBaseName( fi.fileName() );
    QString sb = UnixCommand::getPackageInformation(pkgName, installed);
    QString pkgSize;
    bool sizeInMegaBytes = false;

		int y = sb.indexOf("UNCOMPRESSED PACKAGE SIZE", 0);
		if (y > 0) {
			y += 26;			
			while ( sb[y] != '\n' ){
        if ( sb[y].isDigit() || sb[y] == '.' )
          pkgSize += sb[y];
        else if ( !(sb[y].isSpace()) && (sb[y].isLetter()) && (sb[y] == 'M') )
          sizeInMegaBytes = true;

				y++;
			}
		}
		if (!pkgSize.isEmpty()){
			bool ok;
			double d = pkgSize.toDouble( &ok );
			if ( ok == true ){
				if ( d > 1024 ){
					d /= 1024;
					pkgSize = QString::number( d, '.', 2 ) + " MB";
				}
        else if (sizeInMegaBytes)
          pkgSize += " MB";
        else
          pkgSize += " KB";
			} 				
		}

    QString t = pname + ":";

    int x = sb.indexOf(t, 0);
  	if (x > 0) sb.remove(0, x);
      
    x = sb.indexOf("FILE LIST:", 0);
  	if (x > 0){
  		sb.remove(x, (sb.size()-1)-x);
   	}                      
   	
   	int begn=0;
   	x = sb.indexOf(t);
       
   	if (x == -1){
			if (pkgSize.isEmpty()) return "";
			else return QObject::tr("Installed size: %1").arg(pkgSize);
   	}
   		
   	while(x >= 0){        
   		if (x==0) sb.remove(x, (x+t.size()+1)-x);
   		else {
   			if (!sb.mid(x-1, x).contains("\\S")) sb.remove(x, (x+t.length()+1)-x);
   			else begn += pname.length();
   		}
     	x = sb.indexOf(t, begn);
    }	

		int z = sb.lastIndexOf(QRegExp("[\\n]{2,}"));
		if (z >= 0) sb.remove(z, sb.size()-z);
		
		if (!pkgSize.isEmpty()) sb += "\n\n" + QObject::tr("Installed size: %1").arg(pkgSize);
		
    if ( !sb.isEmpty() )
      return makeURLClickable(sb);
		else return 0;
	}
	
	return 0;
}

QString Package::showRegExp( const QString& a, const QString& re ){
	QRegExp rex(re);	
  QString res = rex.indexIn(a) > -1 ? rex.cap(0) : ctn_NO_MATCH;
	return res;
}

bool Package::isValidArch(const QString &packageArch){
  bool result = false;
  for(uint c=0; c<sizeof(ctn_KNOWN_ARCHS)/sizeof(ctn_KNOWN_ARCHS[0]); c++){
    if (packageArch == ctn_KNOWN_ARCHS[c]){
      result = true;
      break;
    }
  }
  return result;
}

/**
 * This function was copied from ArchLinux Pacman project
 *
 * Compare alpha and numeric segments of two versions.
 * return 1: a is newer than b
 *        0: a and b are the same version
 *       -1: b is newer than a
 */
static int rpmvercmp(const char *a, const char *b){
  char oldch1, oldch2;
  char *str1, *str2;
  char *ptr1, *ptr2;
  char *one, *two;
  int rc;
  int isnum;
  int ret = 0;

  /* easy comparison to see if versions are identical */
  if(strcmp(a, b) == 0) return 0;

  str1 = strdup(a);
  str2 = strdup(b);

  one = ptr1 = str1;
  two = ptr2 = str2;

  /* loop through each version segment of str1 and str2 and compare them */
  while (*one && *two) {
    while (*one && !isalnum((int)*one)) one++;
    while (*two && !isalnum((int)*two)) two++;

    /* If we ran to the end of either, we are finished with the loop */
    if (!(*one && *two)) break;

    /* If the separator lengths were different, we are also finished */
    if ((one - ptr1) != (two - ptr2)) {
      return (one - ptr1) < (two - ptr2) ? -1 : 1;
    }

    ptr1 = one;
    ptr2 = two;

    /* grab first completely alpha or completely numeric segment */
    /* leave one and two pointing to the start of the alpha or numeric */
    /* segment and walk ptr1 and ptr2 to end of segment */
    if (isdigit((int)*ptr1)) {
      while (*ptr1 && isdigit((int)*ptr1)) ptr1++;
      while (*ptr2 && isdigit((int)*ptr2)) ptr2++;
      isnum = 1;
    } else {
      while (*ptr1 && isalpha((int)*ptr1)) ptr1++;
      while (*ptr2 && isalpha((int)*ptr2)) ptr2++;
      isnum = 0;
    }

    /* save character at the end of the alpha or numeric segment */
    /* so that they can be restored after the comparison */
    oldch1 = *ptr1;
    *ptr1 = '\0';
    oldch2 = *ptr2;
    *ptr2 = '\0';

    /* this cannot happen, as we previously tested to make sure that */
    /* the first string has a non-null segment */
    if (one == ptr1) {
      ret = -1;       /* arbitrary */
      goto cleanup;
    }

    /* take care of the case where the two version segments are */
    /* different types: one numeric, the other alpha (i.e. empty) */
    /* numeric segments are always newer than alpha segments */
    /* XXX See patch #60884 (and details) from bugzilla #50977. */
    if (two == ptr2) {
      ret = isnum ? 1 : -1;
      goto cleanup;
    }

    if (isnum) {
      /* this used to be done by converting the digit segments */
      /* to ints using atoi() - it's changed because long  */
      /* digit segments can overflow an int - this should fix that. */

      /* throw away any leading zeros - it's a number, right? */
      while (*one == '0') one++;
      while (*two == '0') two++;

      /* whichever number has more digits wins */
      if (strlen(one) > strlen(two)) {
        ret = 1;
        goto cleanup;
      }
      if (strlen(two) > strlen(one)) {
        ret = -1;
        goto cleanup;
      }
    }

    /* strcmp will return which one is greater - even if the two */
    /* segments are alpha or if they are numeric.  don't return  */
    /* if they are equal because there might be more segments to */
    /* compare */
    rc = strcmp(one, two);
    if (rc) {
      ret = rc < 1 ? -1 : 1;
      goto cleanup;
    }

    /* restore character that was replaced by null above */
    *ptr1 = oldch1;
    one = ptr1;
    *ptr2 = oldch2;
    two = ptr2;
  }

  /* this catches the case where all numeric and alpha segments have */
  /* compared identically but the segment separating characters were */
  /* different */
  if ((!*one) && (!*two)) {
    ret = 0;
    goto cleanup;
  }

  /* the final showdown. we never want a remaining alpha string to
         * beat an empty string. the logic is a bit weird, but:
         * - if one is empty and two is not an alpha, two is newer.
         * - if one is an alpha, two is newer.
         * - otherwise one is newer.
         * */
  if ( (!*one && !isalpha((int)*two))
       || isalpha((int)*one) ) {
    ret = -1;
  } else {
    ret = 1;
  }

cleanup:
  free(str1);
  free(str2);
  return ret;
}

Result Package::getStatus( const QString& pkgToVerify ){
  bool newPackageExtension = false;
  QString arqBaseName = Package::getBaseName(pkgToVerify);

  if ( FrozenPkgListSingleton::instance()->indexOf( QRegExp(QRegExp::escape(arqBaseName)), 0 ) != -1){
    QString installedPackage = InstalledPkgListSingleton::instance()->getFileList().
                               filter( QRegExp(QRegExp::escape(arqBaseName)) )[0];

    int i = InstalledPkgListSingleton::instance()->getFileList().indexOf(
        QRegExp( QRegExp::escape(installedPackage)) );

    return(Result(ectn_FROZEN, InstalledPkgListSingleton::instance()->getFileList().value(i)));
	}

  //If it's a dumped snapshot list of installed packages file...
  if (pkgToVerify.startsWith(ctn_DUMP_FILE)){
    return(Result(ectn_DUMP_FILE, ""));
  }
  else if (pkgToVerify.endsWith(ctn_RPM_PACKAGE_EXTENSION)){
    return(Result(ectn_RPM, ""));
  }
  else{
    if (pkgToVerify.endsWith(ctn_TXZ_PACKAGE_EXTENSION))
      newPackageExtension = true;
    else if (pkgToVerify.endsWith(ctn_TGZ_PACKAGE_EXTENSION))
      newPackageExtension = false;

    else{ //This package has no extension, which means it is probably in the Installed Package tv
      int i = InstalledPkgListSingleton::instance()->getFileList().indexOf(pkgToVerify);
      if (i != -1)
        return(Result(ectn_INSTALLED, InstalledPkgListSingleton::instance()->getFileList().value(i)));
    }

    int i = InstalledPkgListSingleton::instance()->getFileList().indexOf(pkgToVerify.left(pkgToVerify.length()-4));
    if (i != -1){
      return(Result(ectn_INSTALLED, InstalledPkgListSingleton::instance()->getFileList().value(i)));
    }
  }

  Result res(ectn_NOT_INSTALLED, "");
  bool goAhead=false;

  QString res1 = showRegExp(pkgToVerify, ctn_ER);
  if (res1 == ctn_NO_MATCH) {
    return Result(ectn_INTERNAL_ERROR, "");
  }

	QRegExp raux;
  QStringList lPackages2 = InstalledPkgListSingleton::instance()->getFileList().
                           filter( QRegExp(raux.escape( arqBaseName ) ));

  foreach ( QString installedPackage, lPackages2 ){
    if ((pkgToVerify == ".") || (pkgToVerify == "..")) continue;
               
    QString res2 = showRegExp(installedPackage, ctn_ER3);

    int a = res1.indexOf(res2);        
    if (a == 0){
      //It must first make the "brake string" test... 
      QStringList c = pkgToVerify.split("-");
      QStringList d = installedPackage.split("-");

      //If it doens't have: name, version, arch and release, then we discard this package!
      if (c.count() < 4)
        return Result(ectn_INTERNAL_ERROR, "");

      //Are we dealing with a package from a different architecture?
      QString arqBaseNameInstalled = getBaseName(installedPackage);

      if (arqBaseName == arqBaseNameInstalled){
        QString arq1 = c[c.size()-2];
        QString arq2 = d[c.size()-2];
        //if the architectures are different, return immediately!
        if ((arq1 != arq2) && isValidArch(arq1))
          return Result(ectn_OTHER_ARCH, installedPackage);
        else if ((arq1 != arq2) && !isValidArch(arq1))
          return Result(ectn_INTERNAL_ERROR, installedPackage);
      }

      //If the architectures are the same, we can test versions...
      if ((c.size() != 0) && (d.size() != 0) && (c.size() == d.size())){
        goAhead=true;
        for (int k = 0; k < c.size()-3; k++) {
	        if (c[k] != (d[k])) goAhead = false;  
        }

        if (goAhead){
          QString st1(c[c.size()-3]);
          QString st2(d[c.size()-3]);

          int versionTest = rpmvercmp(st1.toAscii().data(), st2.toAscii().data());

          if (versionTest == 1)
            return Result(ectn_SUPERIOR_VERSION, installedPackage);
          else if (versionTest == -1)
            return Result(ectn_INFERIOR_VERSION, installedPackage);
          else if (versionTest == 0){
            //Here comes the release test!
            if (newPackageExtension)
              c[c.size()-1] = c[c.size()-1].replace(ctn_TXZ_PACKAGE_EXTENSION, "");
            else
              c[c.size()-1] = c[c.size()-1].replace(ctn_TGZ_PACKAGE_EXTENSION, "");

            QString sn1, sn2;
            sn1 = showRegExp(c[c.size()-1], "[0-9]*");
            sn2 = showRegExp(d[c.size()-1], "[0-9]*");

            int releaseTest = rpmvercmp(sn1.toAscii().data(), sn2.toAscii().data());

            if (releaseTest == 1)
              return Result(ectn_SUPERIOR_VERSION, installedPackage);
            else if (releaseTest == -1)
              return Result(ectn_INFERIOR_VERSION, installedPackage);
          }
        }
      }
    }
	}        
  
  return res; 						
}

QStringList Package::getContents( const QString& pkgName, bool installed ){
  QStringList sl;
  if (pkgName.endsWith(ctn_RPM_PACKAGE_EXTENSION)) return sl;

  FILE *file;
  char linebuf[1024];
  QString st, fn; 

	if ( installed ){
		fn = pkgName;
		file = fopen ( QFile::encodeName ( fn ),"r" );
		if ( file ){
			while ( fgets ( linebuf,sizeof ( linebuf ),file ) ){
        if ( !strcmp ( linebuf, ctn_FILELIST ) ){
					break;
				}
			}
			while ( fgets ( linebuf,sizeof ( linebuf ),file ) ){
				st = "/";
				st += linebuf;
				st.truncate ( st.length() -1 );
				if ( st.left ( 8 ) != "/install" && st.left ( 3 ) != "/./" ){
					sl << st;
				}
			}
			fclose ( file );
		}
	}
	else{
    if (!isSlackPackage(pkgName)){
      return sl;
    }

    QStringList rsl;
    QByteArray result = UnixCommand::getPackageContents(pkgName);

    if (result == ctn_PKG_CONTENT_ERROR) return QStringList();

    QString aux(result);
		rsl = aux.split("\n", QString::SkipEmptyParts);
		
    if ( !rsl.isEmpty() ){
      if (rsl.at(0) == "./"){
        rsl.removeFirst();
      }
			rsl.replaceInStrings(QRegExp("^"), "/");
			rsl.sort();
    }
		
    if (!rsl.contains("/gzip: stdin: unexpected end of file") &&
        !rsl.contains("/xz: (stdin): Unexpected end of input") &&
        !rsl.contains("/tar: Child returned status 1")) return rsl;
		else{ 
			rsl.clear();
			return rsl;
		}
	}
	
	sl.sort();
	return sl;
}	

QString Package::parseSearchString(QString searchStr, bool exactMatch){
  if (searchStr.indexOf("*.") == 0){
    searchStr = searchStr.remove(0, 2);
    searchStr.insert(0, "\\S+\\.");
  }

  if (searchStr.indexOf("*") == 0){
    searchStr = searchStr.remove(0, 1);
    searchStr.insert(0, "\\S+");
  }

  if (searchStr.endsWith("*")){
    searchStr.remove(searchStr.length()-1, 1);
    searchStr.append("\\S*");
  }

  if (searchStr.indexOf("^") == -1 && searchStr.indexOf("\\S") != 0){
    if (!exactMatch) searchStr.insert(0, "\\S*");
    else searchStr.insert(0, "^");
  }

  if (searchStr.indexOf("$") == -1){
    if (!exactMatch && !searchStr.endsWith("\\S*")) searchStr.append("\\S*");
    else searchStr.append("$");
  }

  searchStr.replace("?", ".");
  return searchStr;
}

QDateTime Package::_getModificationDate(const QString packageName){
  QFileInfo fi(ctn_PACKAGES_DIR + QDir::separator() + packageName);
  if (fi.exists())
    return fi.lastModified();
  else
    return QDateTime();
}

QString Package::getModificationDate(const QString packageName){
  QDateTime md = _getModificationDate(packageName);
  if (md.isValid()){
    return md.toString("dd/MM/yyyy - hh:mm:ss");
  }
  else
    return "";
}

SnapshotList Package::processSnapshotOfInstalledPackageList(QString pDumpedFile){
  SnapshotList snapList;
  QStringList resList;
  QStringList newPackages;
  QHash<QString, QString> dumpedList;
  QStringList dumpedListExt;

  //First, open the dumpedList
  QFile dumpedFile(pDumpedFile);
  dumpedFile.open(QIODevice::ReadOnly | QIODevice::Text);

  QTextStream in(&dumpedFile);

  while (!in.atEnd()){
    QString line = in.readLine();
    QString package;
    QString modificationDate;

    int pos = line.indexOf(" ");
    if (pos > 0){
      package = line.left(pos);
      modificationDate = line.mid(pos+1);
    }
    else
      package = line;

    dumpedList.insert(getBaseName(package), modificationDate);
    dumpedListExt.append(package);
  }

  const int value=65; //justification value

  //Print the list of installed packages
  int cRemoved=0;
  int cInstalled=0;
  int cReinstalled=0;
  int cUpgraded=0;
  int cDowngraded=0;
  int cOtherVersion=0;

  foreach (QString pkg, InstalledPkgListSingleton::instance()->getFileList()){
    if (!dumpedList.contains(getBaseName(pkg))){
      resList.append(pkg.leftJustified(value, ' ') + "[<i>++ " + QObject::tr("installed") + " ++</i>]");
      newPackages.append(pkg);
      cInstalled++;
    }
    //This package is already installed. But maybe it's been reinstalled...
    else if (dumpedListExt.contains(pkg)){
      QString datetime = dumpedList.value(getBaseName(pkg));
      if (QDateTime::fromString(datetime, "dd/MM/yyyy hh:mm:ss") < _getModificationDate(pkg)){
        resList.append(pkg.leftJustified(value, ' ') + " [ " + QObject::tr("reinstalled") + " ]");
        cReinstalled++;
      }
    }
  }

  //Print the rest of the list...
  foreach (QString pkg, dumpedListExt){
    Result res = Package::getStatus(pkg);

    switch(res.getClassification()){    

    case ectn_INFERIOR_VERSION :
      resList.append(res.getInstalledPackage().leftJustified(value, ' ') + " [ > " + QObject::tr("upgraded") + " ]");
      cUpgraded++;
      break;

    case ectn_SUPERIOR_VERSION:
      resList.append(res.getInstalledPackage().leftJustified(value, ' ') + " [ &lt; " + QObject::tr("downgraded") + " ]");
      cDowngraded++;
      break;

    case ectn_OTHER_VERSION:
      //resList.append(res.getInstalledPackage().leftJustified(value, ' ') + " [ ? " + QObject::tr("other version") + " ]");
      cOtherVersion++;
      break;

    case ectn_NOT_INSTALLED :
      resList.append(pkg.leftJustified(value, ' ') + " [ <s>" + QObject::tr("removed") + "</s> ]");
      cRemoved++;
      break;

    default:
      break;
    }
  }

  resList.sort();

  if (resList.count() >= 1)
    resList.append("= (" + QString::number(cInstalled) + " " + QObject::tr("installed") + ") + " +
                 "(" + QString::number(cReinstalled) + " " + QObject::tr("reinstalled") + ") + " +
                 "(" + QString::number(cDowngraded) + " " + QObject::tr("downgraded") + ") + " +
                 "(" + QString::number(cUpgraded) + " " + QObject::tr("upgraded") + ") + " +
                 //"(" + QString::number(cOtherVersion) + " " + QObject::tr("other version") + ") + " +
                 "(" + QString::number(cRemoved) + " " + QObject::tr("removed") + ") = " +
                 QString::number(cInstalled+cReinstalled+cDowngraded+cUpgraded+cOtherVersion+cRemoved) + " " +
                 QObject::tr("changes"));

  snapList.setNewPackagesList(newPackages);
  snapList.setDetails(resList);
  return snapList;
}

void Package::removeTempFiles(){
  QDir d(QDir::tempPath());
  QStringList sl;
  sl << ctn_TEMP_OPEN_FILE_PREFIX + "*";
  QFileInfoList il = d.entryInfoList(sl);

  foreach(QFileInfo fi, il){
    QFile::remove(fi.filePath());
  }
}

InstalledPkgListSingleton* InstalledPkgListSingleton::instance(){
  if (m_pinstance == 0) m_pinstance = new InstalledPkgListSingleton();
  return m_pinstance;
}

QStringList InstalledPkgListSingleton::getFileList(){
  return m_pkgList;
}

void InstalledPkgListSingleton::setFileSystemWatcher(QFileSystemWatcher* fsw){
  connect (fsw, SIGNAL(directoryChanged(const QString)),
                    this, SLOT(installedPkgDirChanged()));
}

InstalledPkgListSingleton::InstalledPkgListSingleton():QObject(){
  QDir scanPackages = QDir(ctn_PACKAGES_DIR);
  scanPackages.setFilter(QDir::Files);
  m_pkgList = scanPackages.entryList();
}

void InstalledPkgListSingleton::installedPkgDirChanged(){
  m_pinstance = 0;
}
