//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : networkserver.h
// Created by  : James Inkster, jinkster@presonus.com
// Description : Basic Network Server
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <https://www.gnu.org/licenses/>
//************************************************************************************************

#pragma once

#include <QtCore/QObject>
#include <QtNetwork/QTCPServer>
#include <QJsonObject>
#include <QTimer>

class NetworkConnection;
	
//************************************************************************************************
// NetworkServer
//************************************************************************************************

class NetworkServer: public QTcpServer
{
	Q_OBJECT
public:
	static const int kIdleMs = 30;
	
	NetworkServer (QObject* parent = nullptr);
	~NetworkServer ();
	
	void start (qint16 port);
	void stop ();
	bool broadcastJson (const QJsonObject& json);
	bool sendJson (NetworkConnection& connection, const QJsonObject& json);
	
signals:
	void stopClients ();
	void receivedJson (const QJsonObject& json, NetworkConnection* connection);
	void connectionAdded (NetworkConnection* connection);
	void connectionRemoved (NetworkConnection* connection);
	
protected:
	// QTcpServer
	void incomingConnection (qintptr socketDescriptor) override;
	
public slots:
	void connectionTerminated (NetworkConnection* connection);
	void idle ();
	
private:
	QVector<NetworkConnection*> connections;
	QVector<NetworkConnection*> deadConnections;
	QTimer timer;
};
