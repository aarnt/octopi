#ifndef MULTISELECTIONDIALOG_H
#define MULTISELECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class MultiSelectionDialog;
}

class MultiSelectionDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit MultiSelectionDialog(QWidget *parent = 0);
  ~MultiSelectionDialog();
  
private:
  Ui::MultiSelectionDialog *ui;
};

#endif // MULTISELECTIONDIALOG_H
