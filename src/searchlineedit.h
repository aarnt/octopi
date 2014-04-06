/*
  Code extracted from
  http://www.jakepetroules.com/2011/07/10/creating-a-windows-explorer-style-search-box-in-qt

  Written by Jake Petroules
  Adapted to suit QTGZManager
*/

#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>

class QToolButton;

class SearchLineEdit : public QLineEdit
{
  Q_OBJECT

private:
  QToolButton *mSearchButton;
  QString styleSheetForCurrentState();
  QString buttonStyleSheetForCurrentState() const;

private slots:
  void updateSearchButton(const QString &text);

protected:
  void resizeEvent(QResizeEvent *event);

public:
  explicit SearchLineEdit(QWidget *parent = NULL);

  inline void initStyleSheet(){ setStyleSheet(styleSheetForCurrentState()); }

public slots:
  void setFoundStyle();
  void setNotFoundStyle();
};



#endif // SEARCHLINEEDIT_H
