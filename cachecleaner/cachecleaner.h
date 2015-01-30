#ifndef CACHECLEANER_H
#define CACHECLEANER_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>

#include "../src/unixcommand.h"

namespace Ui {
class CacheCleaner;
}

class CacheCleaner : public QMainWindow
{
    Q_OBJECT

private:
    Ui::CacheCleaner *ui;
    UnixCommand *m_cmdInstalled;
    UnixCommand *m_cmdUninstalled;


public:
    explicit CacheCleaner(QWidget *parent = 0);
    ~CacheCleaner();

protected:
    int getKeepInstalledNumber();
    int getKeepUninstalledNumber();
    void processDryrunResult(QString output, QListWidget *list, QPushButton *cleanButton);

    QString getCleanInstalledOptions();
    QString getCleanUninstalledOptions();

    void closeEvent(QCloseEvent *);

public slots:
    void keepInstalledChanged();
    void keepUninstalledChanged();

    void refreshInstalledCache();
    void refreshUninstalledCache();

    void cleanInstalledCache();
    void cleanUninstalledCache();

    void finishedInstalled(int, QProcess::ExitStatus);
    void finishedUninstalled(int, QProcess::ExitStatus);

    void finishedDryrunInstalled(int, QProcess::ExitStatus);
    void finishedDryrunUninstalled(int, QProcess::ExitStatus);
};

#endif // CACHECLEANER_H
