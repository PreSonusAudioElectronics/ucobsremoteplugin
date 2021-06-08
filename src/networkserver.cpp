//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : networkclient.cpp
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

#define ENABLE_LOGGING 1
#include "common.h"

#include "networkserver.h"
#include "networkconnection.h"

#include "moc_networkserver.cpp"

//************************************************************************************************
// NetworkServer
//************************************************************************************************

NetworkServer::NetworkServer (QObject* parent)
: QTcpServer (parent)
{
	connect (&timer, &QTimer::timeout, this, &NetworkServer::idle);
	timer.setInterval (kIdleMs);
	timer.start ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

NetworkServer::~NetworkServer ()
{
	timer.stop ();
	disconnect (&timer, &QTimer::timeout, this, &NetworkServer::idle);
	
	for(auto connection : connections)
		delete connection;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkServer::idle ()
{
	// there's something weird with the event loop in 
	// these OBS plugs, i need to pump these manually?
	bool timedOut = false;
	if(waitForNewConnection (0, &timedOut))
	{
		//LOG ("Server detects new connection.")
	}
	
	for(auto connection : connections)
	{
		if(!connection->idle ())
			if(!deadConnections.contains (connection))
				deadConnections.append (connection);
	}
	
	for(auto connection : deadConnections)
	{
		LOG ("Deleting dead connection")
		connections.removeOne (connection);
		delete connection;
	}
	deadConnections.clear ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkServer::start (qint16 port)
{	
	if(listen (QHostAddress::LocalHost, port))
	{
		LOG ("Server listening on localhost, port %d", port)
	}
	else
	{
		LOG ("Server failed to listen on localhost, port %d", port)
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkServer::stop ()
{
	//LOG ("NetworkServer::stop")
	
	emit stopClients ();
	close ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkServer::broadcastJson (const QJsonObject& json)
{
	if(connections.isEmpty ())
		return false;
	
	for(auto connection : connections)
		sendJson (*connection, json);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkServer::sendJson (NetworkConnection& connection, const QJsonObject& json)
{
	connection.writeJson (json);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkServer::incomingConnection (qintptr socketDescriptor)
{
	NetworkConnection* connection = new NetworkConnection (this);
	if(!connection->setDescriptor (socketDescriptor))
	{
		delete connection;
		return;
	}
	
	connect (this, &NetworkServer::stopClients, connection, &NetworkConnection::terminate);
	connect (connection, &NetworkConnection::disconnectedFromClient, this, &NetworkServer::connectionTerminated);
	connect (connection, &NetworkConnection::receivedJson, this, &NetworkServer::receivedJson);
	
	connections.append (connection);
	emit connectionAdded (*connection);
	
	//LOG ("NetworkServer added new connection");
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkServer::connectionTerminated (NetworkConnection& connection)
{
	//LOG ("NetworkServer::connectionTerminated");
	
	deadConnections.append (&connection);
	emit connectionRemoved (connection);
}
