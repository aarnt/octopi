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

/*
 * A collection of functions to deal with distro news and find strings in models
 *
 */

#include <iostream>
#include "utils.h"
#include "strconstants.h"
//#include "pacmanexec.h"
#include "unixcommand.h"
#include "searchbar.h"

#include <QStandardItemModel>
#include <QModelIndex>
#include <QTextStream>
#include <QCryptographicHash>
#include <QDomDocument>
#include <QTextBrowser>
#include <QScreen>
#include <QRegularExpression>
#include <QDebug>

/*
 * Returns the full path of a tree view item (normaly a file in a directory tree)
 */
QString utils::showFullPathOfItem(const QModelIndex &index)
{
  QString str;
  if (!index.isValid()) return str;

  const QStandardItemModel *sim = qobject_cast<const QStandardItemModel*>( index.model() );

  if (sim)
  {
    QStringList sl;
    QModelIndex nindex;
    sl << sim->itemFromIndex( index )->text();
    nindex = index;

    while (true){
      nindex = sim->parent( nindex );

      if (nindex != sim->invisibleRootItem()->index())
        sl << sim->itemFromIndex( nindex )->text();

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
  }

  return str;
}

/*
 * Given a filename 'name', searches for it inside a QStandard item model
 * Result is a list containing all QModelIndex occurencies
 */
QList<QModelIndex> * utils::findFileInTreeView(const QString& name, const QStandardItemModel *sim)
{
  QList<QModelIndex> * res = new QList<QModelIndex>();
  QList<QStandardItem *> foundItems;

  if (name.isEmpty() || sim->rowCount() == 0)
  {
    return res;
  }

  foundItems = sim->findItems(Package::parseSearchString(name), Qt::MatchRegularExpression|Qt::MatchRecursive);
  for(QStandardItem *item: qAsConst(foundItems))
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
QString utils::retrieveDistroNews(bool searchForLatestNews)
{
  LinuxDistro distro = UnixCommand::getLinuxDistro();
  QString res;
  QString tmpRssPath = QDir::homePath() + QDir::separator() + QLatin1String(".config/octopi/.tmp_distro_rss.xml");
  QString rssPath = QDir::homePath() + QDir::separator() + QLatin1String(".config/octopi/distro_rss.xml");
  QString contentsRss;

  QFile fileRss(rssPath);
  if (fileRss.exists())
  {
    if (!fileRss.open(QIODevice::ReadOnly | QIODevice::Text)) res = QLatin1String("");
    QTextStream in2(&fileRss);
    contentsRss = in2.readAll();
    fileRss.close();
  }

  if(searchForLatestNews && UnixCommand::hasInternetConnection() && distro != ectn_UNKNOWN)
  {    
    QString curlCommand = QStringLiteral("%1 -o %2 -L");
    QStringList curlParams;
    QString distroRSSUrl = SettingsManager::getDistroRSSUrl();

    if (distro == ectn_ARCHLINUX || distro == ectn_ARCHBANGLINUX || distro == ectn_ARCHCRAFT || distro == ectn_GARUDALINUX)
    {
      curlCommand = curlCommand.arg(distroRSSUrl, tmpRssPath);
      curlParams = curlCommand.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }
    else if (distro == ectn_ARTIXLINUX)
    {
      curlCommand = QStringLiteral("-k %1 -o %2");
      curlCommand = curlCommand.arg(distroRSSUrl, tmpRssPath);
      curlParams = curlCommand.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }
    else if (distro == ectn_CHAKRA)
    {
      curlCommand = QStringLiteral("-k %1 -o %2");
      curlCommand = curlCommand.arg(distroRSSUrl, tmpRssPath);
      curlParams = curlCommand.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }
    else if (distro == ectn_CONDRESOS)
    {
      curlCommand = QStringLiteral("-A \"Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:59.0) Gecko/20100101 Firefox/59.0\" -k \"%1\" -o %2");
      curlCommand = curlCommand.arg(distroRSSUrl, tmpRssPath);
      curlParams = curlCommand.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }
    else if (distro == ectn_KAOS)
    {
      curlCommand = QStringLiteral("-k %1 -o %2");
      curlCommand = curlCommand.arg(distroRSSUrl, tmpRssPath);
      curlParams = curlCommand.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }
    else if (distro == ectn_MANJAROLINUX)
    {
      curlCommand = curlCommand.arg(distroRSSUrl, tmpRssPath);
      curlParams = curlCommand.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }
    else if (distro == ectn_OBARUN)
    {
      curlCommand = QStringLiteral("-k %1 -o %2");
      curlCommand = curlCommand.arg(distroRSSUrl, tmpRssPath);
      curlParams = curlCommand.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }
    else if (distro == ectn_PARABOLA)
    {
      //Parabola has a certificate which is not "trusted" by default, so we use "curl -k"
      curlCommand = QStringLiteral("-k %1 -o %2");
      curlCommand = curlCommand.arg(distroRSSUrl, tmpRssPath);
      curlParams = curlCommand.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    }

    if (UnixCommand::runCurlCommand(curlParams).isEmpty())
    {
      QFile fileTmpRss(tmpRssPath);
      //QFile fileRss(rssPath);

      if (!fileRss.exists())
      {
        fileTmpRss.rename(tmpRssPath, rssPath);

        if (!fileRss.open(QIODevice::ReadOnly | QIODevice::Text)) return(QLatin1String(""));
        QTextStream in2(&fileRss);
        contentsRss = in2.readAll();
        contentsRss = contentsRss.remove(QRegularExpression(QStringLiteral("^\\s*")));
        fileRss.close();
        fileRss.remove();
        if (!fileRss.open(QIODevice::WriteOnly | QIODevice::Text)) return(QLatin1String(""));
        in2 << contentsRss;
        in2.flush();

        if (distro == ectn_CONDRESOS)
        {
          //Let's remove <lastBuildDate>XYZ</lastBuildDate> text so our SHA1 test works as expected
          //qDebug() << contentsRss;
          contentsRss.replace(QRegularExpression(QStringLiteral("(\\t*)(\\n*)<lastBuildDate>(.*)</lastBuildDate>(\\t*)(\\n*)")), QLatin1String(""));
          fileRss.close();
          fileRss.remove();
          if (!fileRss.open(QIODevice::WriteOnly | QIODevice::Text)) return(QLatin1String(""));
          in2 << contentsRss;
          in2.flush();
        }

        fileRss.close();
        res = contentsRss;
      }
      else
      {
        //A rss file already exists. We have to make a SHA1 hash to compare the contents
        QString tmpRssSHA1;
        QString rssSHA1;
        QString contentsTmpRss;

        //QFile fileTmpRss(tmpRssPath);
        if (!fileTmpRss.open(QIODevice::ReadOnly | QIODevice::Text)) return(QLatin1String(""));
        QTextStream in(&fileTmpRss);
        contentsTmpRss = in.readAll();
        contentsTmpRss = contentsTmpRss.remove(QRegularExpression(QStringLiteral("^\\s*")));
        fileTmpRss.close();
        fileTmpRss.remove();
        if (!fileTmpRss.open(QIODevice::WriteOnly | QIODevice::Text)) return(QLatin1String(""));
        in << contentsTmpRss;
        in.flush();

        if (distro == ectn_CONDRESOS)
        {
          //Let's remove <lastBuildDate>XYZ</lastBuildDate> text so our SHA1 test works as expected
          contentsTmpRss.replace(QRegularExpression(QStringLiteral("(\\t*)(\\n*)<lastBuildDate>(.*)</lastBuildDate>(\\t*)(\\n*)")), QLatin1String(""));

          fileTmpRss.close();
          fileTmpRss.remove();
          if (!fileTmpRss.open(QIODevice::WriteOnly | QIODevice::Text)) return(QLatin1String(""));
          in << contentsTmpRss;
          in.flush();
        }

        fileTmpRss.close();

        tmpRssSHA1 = QString::fromUtf8(QCryptographicHash::hash(contentsTmpRss.toLatin1(), QCryptographicHash::Sha1));
        rssSHA1 = QString::fromUtf8(QCryptographicHash::hash(contentsRss.toLatin1(), QCryptographicHash::Sha1));

        if (tmpRssSHA1 != rssSHA1){
          fileRss.remove();
          fileTmpRss.rename(tmpRssPath, rssPath);

          res = QLatin1String("*") + contentsTmpRss; //The asterisk indicates there is a MORE updated rss!
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
    //Maybe we have a file in "./.config/octopi/distro_rss.xml"
    if (fileRss.exists())
    {
      res = contentsRss;
    }
    else if (searchForLatestNews)
    {
      res = QLatin1String("<h3><font color=\"#E55451\">") + StrConstants::getInternetUnavailableError() + QLatin1String("</font></h3>");
    }
    else if (distro != ectn_UNKNOWN)
    {
      res = QLatin1String("<h3><font color=\"#E55451\">") + StrConstants::getNewsErrorMessage() + QLatin1String("</font></h3>");
    }
    else
    {
      res = QLatin1String("<h3><font color=\"#E55451\">") + StrConstants::getIncompatibleLinuxDistroError() + QLatin1String("</font></h3>");
    }
  }

  return res;
}

/*
 * Parses the raw XML contents from the Distro RSS news feed
 * Creates and returns a string containing a HTML code with latest 10 news
 */
QString utils::parseDistroNews()
{
  QString html;
  LinuxDistro distro = UnixCommand::getLinuxDistro();

  if (distro == ectn_ARCHLINUX || distro == ectn_ARCHBANGLINUX || distro == ectn_ARCHCRAFT || distro == ectn_GARUDALINUX)
  {
    html = QLatin1String("<p align=\"center\"><h2>") + StrConstants::getArchLinuxNews() + QLatin1String("</h2></p><ul>");
  }
  else if (distro == ectn_ARTIXLINUX)
  {
    html = QLatin1String("<p align=\"center\"><h2>") + StrConstants::getArtixLinuxNews() + QLatin1String("</h2></p><ul>");
  }
  else if (distro == ectn_CHAKRA)
  {
    html = QLatin1String("<p align=\"center\"><h2>") + StrConstants::getChakraNews() + QLatin1String("</h2></p><ul>");
  }
  else if (distro == ectn_CONDRESOS)
  {
    html = QLatin1String("<p align=\"center\"><h2>") + StrConstants::getCondresOSNews() + QLatin1String("</h2></p><ul>");
  }
  else if (distro == ectn_ENDEAVOUROS)
  {
    html = QLatin1String("<p align=\"center\"><h2>") + StrConstants::getEndeavourOSNews() + QLatin1String("</h2></p><ul>");
  }
  else if (distro == ectn_KAOS)
  {
    html = QLatin1String("<p align=\"center\"><h2>") + StrConstants::getKaOSNews() + QLatin1String("</h2></p><ul>");
  }
  else if (distro == ectn_MANJAROLINUX)
  {
    html = QLatin1String("<p align=\"center\"><h2>") + StrConstants::getManjaroLinuxNews() + QLatin1String("</h2></p><ul>");
  }
  else if (distro == ectn_OBARUN)
  {
    html = QLatin1String("<p align=\"center\"><h2>") + StrConstants::getObarunLinuxNews() + QLatin1String("</h2></p><ul>");
  }
  else if (distro == ectn_PARABOLA)
  {
    html = QLatin1String("<p align=\"center\"><h2>") + StrConstants::getParabolaNews() + QLatin1String("</h2></p><ul>");
  }

  QString rssPath = QDir::homePath() + QDir::separator() + QLatin1String(".config/octopi/distro_rss.xml");
  QDomDocument doc(QStringLiteral("rss"));
  int itemCounter=0;

  QFile file(rssPath);
  if (!file.open(QIODevice::ReadOnly)) return QLatin1String("");
  if (!doc.setContent(&file)) {
      file.close();
      return QLatin1String("");
  }
  file.close();

  QDomElement docElem = doc.documentElement(); //This is rss
  QDomNode n = docElem.firstChild(); //This is channel
  n = n.firstChild();

  while(!n.isNull()) {
    QDomElement e = n.toElement();

    if(!e.isNull())
    {
      if(e.tagName() == QLatin1String("item"))
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
            if (eText.tagName() == QLatin1String("title"))
            {
              itemTitle = QLatin1String("<h3>") + eText.text() + QLatin1String("</h3>");
            }
            else if (eText.tagName() == QLatin1String("link"))
            {
              itemLink = Package::makeURLClickable(eText.text());
              if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX) itemLink += QLatin1String("<br>");
            }
            else if (eText.tagName() == QLatin1String("description"))
            {
              itemDescription = eText.text();
              itemDescription += QLatin1String("<br>");
            }
            else if (eText.tagName() == QLatin1String("pubDate"))
            {
              itemPubDate = eText.text();
              itemPubDate = itemPubDate.remove(QRegularExpression(QStringLiteral("\\n")));
              int pos = itemPubDate.indexOf(QLatin1String("+"));

              if (pos > -1)
              {
                itemPubDate = itemPubDate.mid(0, pos-1).trimmed() + QLatin1String("<br>");
              }
            }
          }

          text = text.nextSibling();
        }

        html += QLatin1String("<li><p>") + itemTitle + QLatin1Char(' ') + itemPubDate + QLatin1String("<br>") +
            itemLink + itemDescription + QLatin1String("</p></li>");
        itemCounter++;
      }
    }

    n = n.nextSibling();
  }

  html += QLatin1String("</ul>");
  //html = html.replace("<a href=", "<a style=\"color:'" + QGuiApplication::palette().link().color().name() + "'\" href=");

  return html;
}

// ---------------------------- QTextBrowser related -----------------------------------

/*
 * Helper method to find the given "findText" in the given QTextEdit
 */
bool utils::strInQTextEdit(QTextBrowser *text, const QString& findText)
{
  bool res = false;

  if (text)
  {
    positionTextEditCursorAtEnd(text);
    res = text->find(findText, QTextDocument::FindBackward | QTextDocument::FindWholeWords);
    positionTextEditCursorAtEnd(text);
  }

  return res;
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void utils::positionTextEditCursorAtEnd(QTextEdit *textEdit)
{
  if (textEdit)
  {
    QTextCursor tc = textEdit->textCursor();
    tc.clearSelection();
    tc.movePosition(QTextCursor::End);
    textEdit->setTextCursor(tc);
  }
}

/*
 * A helper method which writes the given string to a textbrowser
 */
void utils::writeToTextBrowser(QTextBrowser* text, const QString &str, TreatURLLinks treatURLLinks)
{
  if (text)
  {
    positionTextEditCursorAtEnd(text);

    QString newStr = str;

    if(newStr.contains(QLatin1String("removing ")) ||
       newStr.contains(QLatin1String("could not ")) ||
       newStr.contains(QLatin1String("error:"), Qt::CaseInsensitive) ||
       newStr.contains(QLatin1String("failed")) ||
       newStr.contains(QLatin1String("is not synced")) ||
       newStr.contains(QLatin1String("could not be found")) ||
       newStr.contains(StrConstants::getCommandFinishedWithErrors()))
    {
      newStr = QLatin1String("<b><font color=\"#E55451\">") + newStr + QLatin1String("&nbsp;</font></b>"); //RED
    }

    if(treatURLLinks == ectn_TREAT_URL_LINK)
    {
      text->insertHtml(Package::makeURLClickable(newStr));
    }
    else
    {
      text->insertHtml(newStr);
    }

    text->ensureCursorVisible();
  }
}

// ------------------------------- SearchBar related -----------------------------------

/*
 * Helper to position in the first result when searching inside a textBrowser
 */
void utils::positionInFirstMatch(QTextBrowser *tb, SearchBar *sb)
{
  if (tb && sb && sb->isVisible() && !sb->getTextToSearch().isEmpty()){
    tb->moveCursor(QTextCursor::Start);
    if (tb->find(sb->getTextToSearch()))
      sb->getSearchLineEdit()->setFoundStyle();
    else
      sb->getSearchLineEdit()->setNotFoundStyle();
  }
}

/*
 * Every time the user changes the text to search inside a textBrowser...
 */
void utils::searchBarTextChangedInTextBrowser(QTextBrowser *tb, SearchBar *sb, const QString &textToSearch)
{
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (tb){
    static int limit = 100;

    if (textToSearch.isEmpty() || textToSearch.length() < 2){
      sb->getSearchLineEdit()->initStyleSheet();
      tb->setExtraSelections(extraSelections);
      QTextCursor tc = tb->textCursor();
      tc.clearSelection();
      tb->setTextCursor(tc);
      tb->moveCursor(QTextCursor::Start);
      if (sb && sb->isHidden()) tb->setFocus();
      return;
    }

    tb->setExtraSelections(extraSelections);
    tb->moveCursor(QTextCursor::Start);
    QColor color = QColor(Qt::yellow).lighter(130);

    while(tb->find(textToSearch)){
      QTextEdit::ExtraSelection extra;
      extra.format.setBackground(color);
      extra.cursor = tb->textCursor();
      extraSelections.append(extra);

      if (extraSelections.count() == limit)
        break;
    }

    if (extraSelections.count()>0){
      tb->setExtraSelections(extraSelections);
      tb->setTextCursor(extraSelections.at(0).cursor);
      QTextCursor tc = tb->textCursor();
      tc.clearSelection();
      tb->setTextCursor(tc);
      positionInFirstMatch(tb, sb);
    }
    else sb->getSearchLineEdit()->setNotFoundStyle();
  }
}

/*
 * Every time the user presses Enter, Return, F3 or clicks Find Next inside a textBrowser...
 */
void utils::searchBarFindNextInTextBrowser(QTextBrowser *tb, SearchBar *sb)
{
  if (tb && sb && !sb->getTextToSearch().isEmpty()){
    if (!tb->find(sb->getTextToSearch())){
      tb->moveCursor(QTextCursor::Start);
      tb->find(sb->getTextToSearch());
    }
  }
}

/*
 * Every time the user presses Shift+F3 or clicks Find Previous inside a textBrowser...
 */
void utils::searchBarFindPreviousInTextBrowser(QTextBrowser *tb, SearchBar *sb)
{
  if (tb && sb && !sb->getTextToSearch().isEmpty()){
    if (!tb->find(sb->getTextToSearch(), QTextDocument::FindBackward)){
      tb->moveCursor(QTextCursor::End);
      tb->find(sb->getTextToSearch(), QTextDocument::FindBackward);
    }
  }
}

/*
 * Every time the user presses ESC or clicks the close button inside a textBrowser...
 */
void utils::searchBarClosedInTextBrowser(QTextBrowser *tb, SearchBar *sb)
{
  QTextCursor tc = tb->textCursor();
  searchBarTextChangedInTextBrowser(tb, sb, QLatin1String(""));
  tc.clearSelection();
  tb->setTextCursor(tc);

  if (tb)
    tb->setFocus();
}

/*
 * Positions the given QWidget pointer at the screen center
 */
void utils::positionWindowAtScreenCenter(QWidget *w)
{
  QRect screen;
  const auto screens = QGuiApplication::screens();
  for(QScreen *s: screens)
  {
    if (s->name() == QGuiApplication::primaryScreen()->name())
    {
      screen = s->geometry();
    }
  }

  int centerX = (screen.width() - w->width()) / 2;
  int centerY = (screen.height() - w->height()) / 2;
  w->move(QPoint(centerX, centerY));
}
