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
#include "unixcommand.h"
#include "strconstants.h"

#include <QDirIterator>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QCoreApplication>
#include <QTextStream>
#include <QCryptographicHash>
#include <QDomDocument>

/*
 * This is a controller class that provides search services, using Package methods.
 */

/*
 * Returns the full path of a tree view item (normaly a file in a directory tree)
 */
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

  QFileInfo fileInfo(str);
  if (fileInfo.isDir())
  {
    str += QDir::separator();
  }

  return str;
}

/*
 * Given a filename 'name', searches for it inside a QStandard item model
 * Result is a list containing all QModelIndex occurencies
 */
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
    //if (item->accessibleDescription().contains("directory")) continue;

    res->append(item->index());
  }

  return res;
}

/*
 * Retrieves the distro RSS news feed from its respective site
 * If it fails to connect to the internet, uses the available "./.config/octopi/distro_rss.xml"
 * The result is a QString containing the RSS News Feed XML code
 */
QString PackageController::retrieveDistroNews(bool searchForLatestNews)
{
  const QString ctn_ARCH_LINUX_RSS = "https://www.archlinux.org/feeds/news/";
  const QString ctn_CHAKRA_RSS = "http://chakraos.org/news/index.php?/feeds/index.rss2";
  const QString ctn_KAOS_RSS = "http://kaosx.us/feed/";
  const QString ctn_MANJARO_LINUX_RSS = "http://manjaro.org/feed/";

  LinuxDistro distro = UnixCommand::getLinuxDistro();
  QString res;
  QString tmpRssPath = QDir::homePath() + QDir::separator() + ".config/octopi/.tmp_distro_rss.xml";
  QString rssPath = QDir::homePath() + QDir::separator() + ".config/octopi/distro_rss.xml";
  QString contentsRss;

  QFile fileRss(rssPath);
  if (fileRss.exists())
  {
    if (!fileRss.open(QIODevice::ReadOnly | QIODevice::Text)) res = "";
    QTextStream in2(&fileRss);
    contentsRss = in2.readAll();
    fileRss.close();
  }

  if(searchForLatestNews && UnixCommand::hasInternetConnection() && distro != ectn_UNKNOWN)
  {
    QString curlCommand = "curl %1 -o %2";

    if (distro == ectn_ARCHLINUX || distro == ectn_ARCHBANGLINUX || distro == ectn_MOOOSLINUX)
    {
      curlCommand = curlCommand.arg(ctn_ARCH_LINUX_RSS).arg(tmpRssPath);
    }
    else if (distro == ectn_CHAKRA)
    {
      curlCommand = curlCommand.arg(ctn_CHAKRA_RSS).arg(tmpRssPath);
    }
    else if (distro == ectn_KAOS)
    {
      curlCommand = curlCommand.arg(ctn_KAOS_RSS).arg(tmpRssPath);
    }
    else if (distro == ectn_MANJAROLINUX)
    {
      curlCommand = curlCommand.arg(ctn_MANJARO_LINUX_RSS).arg(tmpRssPath);
    }

    if (UnixCommand::runCurlCommand(curlCommand).isEmpty())
    {
      QFile fileTmpRss(tmpRssPath);
      QFile fileRss(rssPath);

      if (!fileRss.exists())
      {
        fileTmpRss.rename(tmpRssPath, rssPath);

        if (!fileRss.open(QIODevice::ReadOnly | QIODevice::Text)) res = "";
        QTextStream in2(&fileRss);
        contentsRss = in2.readAll();
        fileRss.close();

        res = contentsRss;
      }
      else
      {
        //A rss file already exists. We have to make a SHA1 hash to compare the contents
        QString tmpRssSHA1;
        QString rssSHA1;
        QString contentsTmpRss;

        QFile fileTmpRss(tmpRssPath);
        if (!fileTmpRss.open(QIODevice::ReadOnly | QIODevice::Text)) res = "";
        QTextStream in(&fileTmpRss);
        contentsTmpRss = in.readAll();
        fileTmpRss.close();

        tmpRssSHA1 = QCryptographicHash::hash(contentsTmpRss.toLatin1(), QCryptographicHash::Sha1);
        rssSHA1 = QCryptographicHash::hash(contentsRss.toLatin1(), QCryptographicHash::Sha1);

        if (tmpRssSHA1 != rssSHA1){
          fileRss.remove();
          fileTmpRss.rename(tmpRssPath, rssPath);

          res = "*" + contentsTmpRss; //The asterisk indicates there is a MORE updated rss!
        }
        else
        {
          fileTmpRss.remove();
          res = contentsRss;
        }
      }
    }
  }

  //Either we don't have internet or we weren't asked to retrieve the latest news
  else
  {
    QFile fileRss(rssPath);

    //Maybe we have a file in "./.config/octopi/distro_rss.xml"
    if (fileRss.exists())
    {
      res = contentsRss;
    }
    else if (searchForLatestNews)
    {
      res = "<h3><font color=\"#E55451\">" + StrConstants::getInternetUnavailableError() + "</font></h3>";
    }
    else if (distro != ectn_UNKNOWN)
    {
      res = "<h3><font color=\"#E55451\">" + StrConstants::getNewsErrorMessage() + "</font></h3>";
    }
    else
    {
      res = "<h3><font color=\"#E55451\">" + StrConstants::getIncompatibleLinuxDistroError() + "</font></h3>";
    }
  }

  return res;
}

/*
 * Parses the raw XML contents from the Distro RSS news feed
 * Creates and returns a string containing a HTML code with latest 10 news
 */
QString PackageController::parseDistroNews()
{
  QString html;

  LinuxDistro distro = UnixCommand::getLinuxDistro();
  if (distro == ectn_ARCHLINUX || distro == ectn_ARCHBANGLINUX || distro == ectn_MOOOSLINUX)
  {
    html = "<p align=\"center\"><h2>" + StrConstants::getArchLinuxNews() + "</h2></p><ul>";
  }
  else if (distro == ectn_CHAKRA)
  {
    html = "<p align=\"center\"><h2>" + StrConstants::getChakraNews() + "</h2></p><ul>";
  }
  else if (distro == ectn_KAOS)
  {
    html = "<p align=\"center\"><h2>" + StrConstants::getKaOSNews() + "</h2></p><ul>";
  }
  else if (distro == ectn_MANJAROLINUX)
  {
    html = "<p align=\"center\"><h2>" + StrConstants::getManjaroLinuxNews() + "</h2></p><ul>";
  }

  QString lastBuildDate;
  QString rssPath = QDir::homePath() + QDir::separator() + ".config/octopi/distro_rss.xml";
  QDomDocument doc("rss");
  int itemCounter=0;

  QFile file(rssPath);
  if (!file.open(QIODevice::ReadOnly)) return "";
  if (!doc.setContent(&file)) {
      file.close();
      return "";
  }
  file.close();

  QDomElement docElem = doc.documentElement(); //This is rss
  QDomNode n = docElem.firstChild(); //This is channel
  n = n.firstChild();

  while(!n.isNull()) {
    QDomElement e = n.toElement();

    if(!e.isNull())
    {
      if (e.tagName() == "lastBuildDate")
      {
        lastBuildDate = e.text();
      }
      else if(e.tagName() == "item")
      {
        //Let's iterate over the 10 lastest "item" news
        if (itemCounter == 10) break;

        QDomNode text = e.firstChild();

        QString itemTitle;
        QString itemLink;
        QString itemDescription;
        QString itemPubDate;

        while(!text.isNull())
        {
          QDomElement eText = text.toElement();

          if(!eText.isNull())
          {
            if (eText.tagName() == "title")
            {
              itemTitle = "<h3>" + eText.text() + "</h3>";
            }
            else if (eText.tagName() == "link")
            {
              itemLink = Package::makeURLClickable(eText.text());
              if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX) itemLink += "<br>";
            }
            else if (eText.tagName() == "description")
            {
              itemDescription = eText.text();
              //itemDescription = itemDescription.remove(QRegExp("\\n"));
              itemDescription += "<br>";
            }
            else if (eText.tagName() == "pubDate")
            {
              itemPubDate = eText.text();
              itemPubDate = itemPubDate.remove(QRegExp("\\n"));
              int pos = itemPubDate.indexOf("+");

              if (pos > -1)
              {
                itemPubDate = itemPubDate.mid(0, pos-1).trimmed() + "<br>";
              }
            }
          }

          text = text.nextSibling();
        }

        html += "<li><p>" + itemTitle + " " + itemPubDate + "<br>" + itemLink + itemDescription + "</p></li>";
        itemCounter++;
      }
    }

    n = n.nextSibling();
  }

  html += "</ul>";
  return html;
}
