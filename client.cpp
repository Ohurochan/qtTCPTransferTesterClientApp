/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <QtNetwork>

#include "client.h"

#include "ui_frmTcpClient.h"

#include <QLayout>

//! [0]
Client::Client(QWidget *parent)
:   QWidget(parent), networkSession(0)
{

    setupUI();

    setupNetwork();

    setupConnection();

    updateUI();
}

void Client::requestNewFortune()
{
    btnSendRequest->setEnabled(false);
    blockSize = 0;
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(),
                             portLineEdit->text().toInt());
}


void Client::readFortune()
{

    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    if (blockSize == 0) {
        if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
            return;
        in >> blockSize;
    }

    if (tcpSocket->bytesAvailable() < blockSize)
        return;

    // Dataの読み出し
    QByteArray imageData;
    QString msgString;
    in >> msgString >> imageData;

    // 現在時刻取得
    QDateTime curTime = QDateTime::currentDateTime();
    QString strCurTime = "RecvTime:" + curTime.toString("hhmmsszzz");

    // UI更新
    currentFortune = QString("Recvd [%1]Bytes:%2, %3").
            arg(blockSize).
            arg(msgString)
            .arg(strCurTime);
    btnSendRequest->setEnabled(true);

    updateUI();
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    btnSendRequest->setEnabled(true);
}

void Client::enableGetFortuneButton()
{
    btnSendRequest->setEnabled((!networkSession || networkSession->isOpen()) &&
                                 !hostCombo->currentText().isEmpty() &&
                                 !portLineEdit->text().isEmpty());

}

void Client::sessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

    currentFortune = tr("This examples requires that you run the "
                            "Fortune Server example as well.");

    enableGetFortuneButton();

    updateUI();
}

/**
 * @brief Client::setupUI
 */
void Client::setupUI()
{
    // Ui Settings from Form
    ui = new Ui::frmTcpClient;
    ui->setupUi(this);
    hostLabel = ui->lblIPAddress;
    portLabel = ui->lblPort;
    hostCombo = ui->cmbIPAddresses;
    portLineEdit = ui->ledtPort;
    btnSendRequest = ui->btnRequest;
    quitButton = ui->btnQuit;
    statusLabel = ui->lblMessages;

    /// ServerNames
    hostCombo->setEditable(true);
    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            hostCombo->addItem(name + QChar('.') + domain);
    }
    if (name != QString("localhost"))
        hostCombo->addItem(QString("localhost"));

    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }
    // add localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }

    /// Server Port
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    /// Labels
    hostLabel->setBuddy(hostCombo);
    portLabel->setBuddy(portLineEdit);

    currentFortune = tr("This tcp client requires that you run the "
                                "Tcp Server App as well.");
    statusLabel->setText(currentFortune);

    /// Buttons
    btnSendRequest->setDefault(true);
    btnSendRequest->setEnabled(false);

//    buttonBox = new QDialogButtonBox;
//    buttonBox->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
//    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);


    setWindowTitle(qAppName());
    portLineEdit->setFocus();
}

/**
 * @brief Client::setupNetwork
 */
void Client::setupNetwork()
{

    tcpSocket = new QTcpSocket(this);
    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        btnSendRequest->setEnabled(false);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    }
}

/**
 * @brief Client::setupConnection
 */
void Client::setupConnection()
{
    connect(hostCombo, SIGNAL(editTextChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(btnSendRequest, SIGNAL(clicked()),
            this, SLOT(requestNewFortune()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readFortune()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
}

void Client::changeEvent(QEvent *ev)
{
    QWidget::changeEvent(ev);
    if(ev->type() == QEvent::Resize)
        updateUI();
}



void Client::updateUI()
{
    statusLabel->setText(currentFortune);
    hostLabel->fixFontSize();
    portLabel->fixFontSize();
    statusLabel->fixFontSize();
}

