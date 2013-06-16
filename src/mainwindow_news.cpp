/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2013 Alexandre Albuquerque Arnt
*               2013 Manuel Tortosa
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

/*
 * This is a MainWindow's Distro news related code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "searchbar.h"
#include "uihelper.h"

#include <QTextBrowser>
#include <QTextStream>
#include <QCryptographicHash>
#include <QDomDocument>

/*
 * Retrieves the distro RSS news feed from its respective site
 * If it fails to connect to the internet, uses the available "./.config/octopi/distro_rss.xml"
 * The result is a QString containing the RSS News Feed XML code
 */
QString MainWindow::retrieveDistroNews(bool searchForLatestNews)
{
  const QString ctn_ARCH_LINUX_RSS = "https://www.archlinux.org/feeds/news/";
  const QString ctn_CHAKRA_RSS = "http://chakra-project.org/news/index.php?/feeds/index.rss2";
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

    if (distro == ectn_ARCHLINUX || distro == ectn_ARCHBANGLINUX)
    {
      curlCommand = curlCommand.arg(ctn_ARCH_LINUX_RSS).arg(tmpRssPath);
    }
    else if (distro == ectn_CHAKRA)
    {
      curlCommand = curlCommand.arg(ctn_CHAKRA_RSS).arg(tmpRssPath);
    }
    else if (distro == ectn_MANJAROLINUX)
    {
      curlCommand = curlCommand.arg(ctn_MANJARO_LINUX_RSS).arg(tmpRssPath);
    }

    if (UnixCommand::runCurlCommand(curlCommand) == 0)
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

        tmpRssSHA1 = QCryptographicHash::hash(contentsTmpRss.toAscii(), QCryptographicHash::Sha1);
        rssSHA1 = QCryptographicHash::hash(contentsRss.toAscii(), QCryptographicHash::Sha1);

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
QString MainWindow::parseDistroNews()
{
  QString html;

  LinuxDistro distro = UnixCommand::getLinuxDistro();
  if (distro == ectn_ARCHLINUX || distro == ectn_ARCHBANGLINUX)
  {
    html = "<p align=\"center\"><h2>" + StrConstants::getArchLinuxNews() + "</h2></p><ul>";
  }
  else if (distro == ectn_CHAKRA)
  {
    html = "<p align=\"center\"><h2>" + StrConstants::getChakraNews() + "</h2></p><ul>";
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

/*
 * This is the high level method that orquestrates the Distro RSS News printing in tabNews
 *
 * boolean parameter searchForLatestNews:
 *    controls whether this method tries to connect to the remote RSS News Feed (default is true)
 * boolean parameter gotoNewsTab:
 *    controls whether this method must set the mainwindow focus to NewsTab (default is true)
 */
void MainWindow::refreshDistroNews(bool searchForLatestNews, bool gotoNewsTab)
{
  qApp->processEvents();

  if (searchForLatestNews)
  {
    LinuxDistro distro = UnixCommand::getLinuxDistro();
    clearTabOutput();

    if (distro == ectn_ARCHLINUX || distro == ectn_ARCHBANGLINUX)
    {
      writeToTabOutputExt("<b>" +
                          StrConstants::getSearchingForDistroNews().arg("Arch Linux") + "</b>");
    }
    else if (distro == ectn_CHAKRA)
    {
      writeToTabOutputExt("<b>" +
                          StrConstants::getSearchingForDistroNews().arg("Chakra") + "</b>");
    }
    else if (distro == ectn_MANJAROLINUX)
    {
      writeToTabOutputExt("<b>" +
                          StrConstants::getSearchingForDistroNews().arg("Manjaro Linux") + "</b>");
    }

    qApp->processEvents();
  }

  CPUIntensiveComputing *cic = new CPUIntensiveComputing;
  QString distroRSSXML = retrieveDistroNews(searchForLatestNews);
  delete cic;

  QString html;

  if (distroRSSXML.count() >= 200)
  {
    if (distroRSSXML.at(0)=='*')
    {
      /* If this is an updated RSS, we must warn the user!
         And if the main window is hidden... */
      if (isHidden())
      {
        show();
      }

      ui->twProperties->setTabText(ctn_TABINDEX_NEWS, "** " + StrConstants::getTabNewsName() + " **");
      if (gotoNewsTab)
      {
        ui->twProperties->setCurrentIndex(ctn_TABINDEX_NEWS);
      }
    }
    else
    {
      if(searchForLatestNews)
      {
        ui->twProperties->setTabText(ctn_TABINDEX_NEWS, StrConstants::getTabNewsName());
      }
    }

    //First, we have to parse the raw RSS XML...
    html = parseDistroNews();
  }
  else
  {
    if(searchForLatestNews)
      ui->twProperties->setTabText(ctn_TABINDEX_NEWS, StrConstants::getTabNewsName());

    html = distroRSSXML;
  }

  //Now that we have the html table code, let's put it into TextBrowser's News tab
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_NEWS)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    text->clear();
    text->setHtml(html);
  }

  clearTabOutput();
  qApp->processEvents();

  if (searchForLatestNews && gotoNewsTab)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_NEWS);
  }
}

/*
 * This SLOT is called every time the user clicks a url inside newsTab textBrowser
 */
void MainWindow::onTabNewsSourceChanged(QUrl newSource)
{
  if(newSource.isRelative())
  {
    //If the user clicked a relative and impossible to display link...
    QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_NEWS)->findChild<QTextBrowser*>("textBrowser");
    if (text)
    {
      disconnect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));
      text->setHtml(parseDistroNews());
      connect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));
    }
  }
}

/*
 * This is the TextBrowser News tab, which shows the latest news from Distro news feed
 */
void MainWindow::initTabNews()
{
  QString aux(StrConstants::getTabNewsName());
  QWidget *tabNews = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabNews);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setMargin(0);

  QTextBrowser *text = new QTextBrowser(tabNews);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget(text, 0, 0, 1, 1);
  text->show();

  int tindex = ui->twProperties->insertTab(ctn_TABINDEX_NEWS, tabNews, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  ui->twProperties->setTabText(ui->twProperties->indexOf(tabNews), QApplication::translate(
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  SearchBar *searchBar = new SearchBar(this);
  MyHighlighter *highlighter = new MyHighlighter(text, "");

  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPrevious()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);
  gridLayoutX->addWidget(new SyntaxHighlighterWidget(this, highlighter));

  connect(text, SIGNAL(sourceChanged(QUrl)), this, SLOT(onTabNewsSourceChanged(QUrl)));

  text->show();
  ui->twProperties->setCurrentIndex(tindex);
  text->setFocus();
}
