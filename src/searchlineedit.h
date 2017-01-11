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
class QValidator;
class QCompleter;
class QStringListModel;

enum ValidatorType { ectn_AUR_VALIDATOR, ectn_FILE_VALIDATOR, ectn_DEFAULT_VALIDATOR };

class SearchLineEdit : public QLineEdit
{
  Q_OBJECT

private:
  bool m_hasLocate;
  QStringListModel *m_completerModel;

  QCompleter *m_completer;
  QValidator *m_defaultValidator;
  QValidator *m_aurValidator;
  QValidator *m_fileValidator;
  QToolButton *m_SearchButton;
  QString styleSheetForCurrentState();  
  QString buttonStyleSheetForCurrentState() const;

private slots:
  void updateSearchButton(const QString &text);

protected:
  void resizeEvent(QResizeEvent *event);

public:
  explicit SearchLineEdit(QWidget *parent = NULL, bool hasSLocate = false);

  inline void initStyleSheet(){ setStyleSheet(styleSheetForCurrentState()); }
  void setRefreshValidator(ValidatorType validatorType);
  void refreshCompleterData();

public slots:
  void setFoundStyle();
  void setNotFoundStyle();
};

#endif // SEARCHLINEEDIT_H
