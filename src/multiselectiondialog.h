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

  void addPackageItem(const QString & name, const QString & description, const QString & repository);
  QStringList getSelectedPackages();

private:
  Ui::MultiSelectionDialog *ui;

private slots:
  void reject();
  void slotOk();
};

#endif // MULTISELECTIONDIALOG_H
