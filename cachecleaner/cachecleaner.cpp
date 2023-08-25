/*
Copyright 2015 Michaël Lhomme

This file is part of AppSet.

AppSet is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

AppSet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with AppSet; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "cachecleaner.h"
#include "ui_cachecleaner.h"
#include "../src/strconstants.h"

#include <QKeyEvent>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>

/*
 * CacheCleaner window constructor
 */
CacheCleaner::CacheCleaner(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::CacheCleaner)
{
  //UI initialization
  ui->setupUi(this);

  int keepInstalled = SettingsManager::getKeepNumInstalledPackages();
  ui->keepInstalledPackagesSpinner->setValue(keepInstalled);

  int keepUninstalled = SettingsManager::getKeepNumUninstalledPackages();
  ui->keepUninstalledPackagesSpinner->setValue(keepUninstalled);

  //create package group wrappers
  m_installed = new PackageGroupModel(QLatin1String(""),
                                      ui->installedPackagesList,
                                      ui->keepInstalledPackagesSpinner,
                                      ui->refreshInstalledButton,
                                      ui->cleanInstalledButton);

  m_uninstalled = new PackageGroupModel(QStringLiteral("-u"),
                                        ui->uninstalledPackagesList,
                                        ui->keepUninstalledPackagesSpinner,
                                        ui->refreshUninstalledButton,                                                                                
                                        ui->cleanUninstalledButton);

  m_tcpServer = new QTcpServer(this);
  connect(m_tcpServer, &QTcpServer::newConnection, this, &CacheCleaner::onSendInfoToOctopiHelper);

  restoreGeometry(SettingsManager::getCacheCleanerWindowSize());
}

/*
 * Cache Cleaner destructor
 */
CacheCleaner::~CacheCleaner()
{
  delete m_installed;
  delete m_uninstalled;
  delete ui;
}

/*
 * Start listening for helper connections
 */
bool CacheCleaner::startServer()
{
  bool res=true;

  if (!m_tcpServer->listen(QHostAddress::LocalHost, 12703))
  {
    QMessageBox::critical(this, StrConstants::getApplicationName(),
                          QStringLiteral("Unable to start the server: %1.")
                          .arg(m_tcpServer->errorString()));
    res=false;
  }

  return res;
}

/*
 * Answers Helper if CacheCleaner is executing actions
 */
void CacheCleaner::onSendInfoToOctopiHelper()
{
  QString msg;
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_15);

  //Is octopi-helper running?
  bool isHelperExecuting=UnixCommand::isOctopiHelperRunning();

  bool commandExecuting = (m_installed->isExecutingCommand || m_uninstalled->isExecutingCommand);

  if (isHelperExecuting && commandExecuting)
  {
    msg=QLatin1String("Octopi est occupatus");
    out << msg;
  }
  else if (isHelperExecuting && !commandExecuting)
  {
    msg=QLatin1String("Octopi serenum est");
    out << msg;
  }
  else
  {
    msg=QLatin1String("Atramento nigro");
    out << msg;
  }

  QTcpSocket *clientConnection = m_tcpServer->nextPendingConnection();
  if (clientConnection->isOpen())
  {
    connect(clientConnection, &QAbstractSocket::disconnected,
          clientConnection, &QObject::deleteLater);

    clientConnection->write(block);
    clientConnection->disconnectFromHost();
  }

  //m_installed->isExecutingCommand=false;
  //m_uninstalled->isExecutingCommand=false;
}

/*
 * Save settings when closing window
 */
void CacheCleaner::closeEvent(QCloseEvent *)
{
  QByteArray windowSize=saveGeometry();

  SettingsManager::setCacheCleanerWindowSize(windowSize);
  SettingsManager::setKeepNumInstalledPackages(ui->keepInstalledPackagesSpinner->value());
  SettingsManager::setKeepNumUninstalledPackages(ui->keepUninstalledPackagesSpinner->value());
}

/*
 * Whenever user presses ESC, we quit the program
 */
void CacheCleaner::keyPressEvent(QKeyEvent *ke)
{
  if (ke->key() == Qt::Key_Escape)
  {
    close();
  }
}
