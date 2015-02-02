#ifndef CACHECLEANER_H
#define CACHECLEANER_H

#include <QMainWindow>

#include "packagegroupmodel.h"

namespace Ui {
class CacheCleaner;
}


/*
 * Main CacheCleaner window
 */
class CacheCleaner : public QMainWindow
{
    Q_OBJECT

private:
    Ui::CacheCleaner *ui;

    PackageGroupModel *m_installed;
    PackageGroupModel *m_uninstalled;

public:
    explicit CacheCleaner(QWidget *parent = 0);
    ~CacheCleaner();

protected:
    void closeEvent(QCloseEvent *);
};

#endif // CACHECLEANER_H
