#ifndef PACMANEXEC_H
#define PACMANEXEC_H

#include <QObject>
#include "constants.h"
#include "unixcommand.h"

class PacmanExec : public QObject
{
  Q_OBJECT

private:
  bool m_iLoveCandy;
  bool m_debugMode;
  UnixCommand *m_unixCommand;
  CommandExecuting m_commandExecuting;
  QStringList m_lastCommandList; //run in terminal commands
  QStringList m_textPrinted;

  bool searchForKeyVerbs(QString output);
  bool splitOutputStrings(QString output);
  void parsePacmanProcessOutput(QString output);

  void prepareTextToPrint(QString str, TreatString ts = ectn_TREAT_STRING, TreatURLLinks tl = ectn_TREAT_URL_LINK);

private slots:
  //UnixCommand slots:
  void onStarted();
  void onReadOutput();
  void onReadOutputError();
  void onFinished(int exitCode, QProcess::ExitStatus);

public:
  explicit PacmanExec(QObject *parent = 0);
  virtual ~PacmanExec();

  void setDebugMode(bool value);
  void runLastestCommandInTerminal();
  void removeTemporaryFile();
  static bool isDatabaseLocked();
  static void removeDatabaseLock();

  //MIRROR-CHECK
  void doMirrorCheck();

  //PACMAN
  void doInstall(const QString &listOfPackages);
  void doInstallInTerminal(const QString &listOfPackages);

  void doInstallLocal(const QString &listOfPackages);
  void doInstallLocalInTerminal(const QString &listOfPackages);

  void doRemove(const QString &listOfPackages);
  void doRemoveInTerminal(const QString &listOfPackages);

  void doRemoveAndInstall(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall);
  void doRemoveAndInstallInTerminal(const QString &listOfPackagestoRemove, const QString &listOfPackagestoInstall);

  void doSystemUpgrade();
  void doSystemUpgradeInTerminal();

  void doSyncDatabase();

  //AUR
  void doAURUpgrade(const QString &listOfPackages);
  void doAURInstall(const QString &listOfPackages);
  void doAURRemove(const QString &listOfPackages);

signals:
  void percentage(int);
  void started();
  void readOutput();
  void readOutputError();
  void finished(int exitCode, QProcess::ExitStatus);

  void textToPrintExt(QString m_textToPrint);

};

#endif // PACMANEXEC_H
