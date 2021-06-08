//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : networkconnection.h
// Created by  : James Inkster, jinkster@presonus.com
// Description : Network Connection
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
#include <QtNetwork/QTCPSocket>
#include <QElapsedTimer>

//************************************************************************************************
// NetworkReader
//************************************************************************************************

class NetworkReader: public QObject 
{
	Q_OBJECT
public:
	NetworkReader (QTcpSocket& socket);
	
	bool read ();

signals:
	void receivedJson (const QJsonObject& json);
	
protected:
	void resetBuffers ();
	int readPos;
	QTcpSocket& socket;
	qint64 expectingBytes;
	qint64 bytesToRead;
	QString buffer;
};

//************************************************************************************************
// NetworkWriter
//************************************************************************************************

class NetworkWriter
{
public:
	NetworkWriter (QTcpSocket& socket);
	
	bool write (const QJsonObject& json);
	
protected:
	QTcpSocket& socket;
};

//************************************************************************************************
// NetworkConnection
//************************************************************************************************

class NetworkConnection: public QObject
{
	Q_OBJECT
public:
	static const int kAliveMs = 5000;
	static const int kNumHeaderBytes = 4; // we always send a 4 byte size string before json payload
	
	NetworkConnection (QObject* parent);
	~NetworkConnection ();
	
	bool setDescriptor (qintptr descriptor);
	bool writeJson (const QJsonObject& json);
	bool idle ();
	
signals:
	void receivedJson (const QJsonObject& json, NetworkConnection& connection);
	void disconnectedFromClient (NetworkConnection& connection);
	
public slots:
	void readData ();
	void parsedData (const QJsonObject& json);
	void terminate ();
	void disconnectCompleted ();
	
protected:
	bool writeData (const QByteArray& data);
	bool doWriteJson (const QJsonObject& json);
	
private:
	QTcpSocket socket;
	NetworkReader reader;
	NetworkWriter writer;
	QElapsedTimer aliveTimer;
};
