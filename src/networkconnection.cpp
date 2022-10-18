//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : networkconnection.cpp
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

#define ENABLE_LOGGING 1
#include "common.h"

#include "networkconnection.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include "moc_networkconnection.cpp"

//************************************************************************************************
// NetworkWriter
//************************************************************************************************

NetworkWriter::NetworkWriter (QTcpSocket& socket)
: socket (socket)
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkWriter::write (const QJsonObject& json)
{
	QByteArray jsonData = QJsonDocument (json).toJson (QJsonDocument::Compact);
	qint32 bytesToSend = jsonData.size ();
	if(bytesToSend <= 0)
	{
		LOG ("Warning: NetworkWriter trying to write %d bytes", bytesToSend)
		return true;
	}
	
	// we write all data with a leading 4-byte 'header' (which is just the # of bytes following)
	QString paddedSizeString = QString ("%1").arg (QString::number (bytesToSend), NetworkConnection::kNumHeaderBytes, QChar ('0'));
	//LOG ("NetworkWriter::write: [%s]:'%s'", STR (paddedSizeString), jsonData.data ())
	if(socket.write (STR (paddedSizeString)) > 0)
	{
		if(socket.write (jsonData) > 0)
			return true;
	}
	return false;
}

//************************************************************************************************
// NetworkReader
//************************************************************************************************

NetworkReader::NetworkReader (QTcpSocket& socket)
: socket (socket),
  readPos (0),
  expectingBytes (0),
  bytesToRead (0)
{
	resetBuffers ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkReader::resetBuffers ()
{
	readPos = -NetworkConnection::kNumHeaderBytes;
	expectingBytes = 0;
	bytesToRead = 0;
	buffer.clear ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkReader::read ()
{
	while(true)
	{
		qint64 bytesAvailable = socket.bytesAvailable ();
		if(bytesAvailable <= 0)
			return true;
		
		if(readPos < 0)
		{
			expectingBytes = 0;
			bytesToRead = 0;
			// we're awaiting the "header" (the number of bytes in the following json)
			QString headerBytes = socket.read (-readPos);
			if(headerBytes.isEmpty ())
			{
				LOG ("No header bytes found")
				return false;
			}
			int offset = NetworkConnection::kNumHeaderBytes + readPos;
			buffer.insert (offset, headerBytes);
			readPos += headerBytes.length ();
		}
		
		if(readPos >= 0)
		{
			if(expectingBytes <= 0)
			{
				// buffer will now contain a string with the # of bytes we need to read
				bool converted = false;
				expectingBytes = buffer.toInt (&converted);
				if(!converted || expectingBytes < 0)
				{
					LOG ("Malformed expectant bytes: %s\n", STR (buffer));
					return false;
				}
				bytesToRead = expectingBytes;
				buffer.clear ();
			}
			
			QString readBytes = socket.read (bytesToRead);
			bytesToRead -= readBytes.length ();
			buffer.insert (readPos, readBytes);
			if(bytesToRead <= 0)
			{
				// convert to json
				QJsonParseError parseError;
				const QJsonDocument jsonDoc = QJsonDocument::fromJson (buffer.toUtf8 (), &parseError);
				if(parseError.error == QJsonParseError::NoError && jsonDoc.isObject ())
				{
					//LOG ("NetworkReader::read: [%s]", STR (buffer))					
					emit receivedJson (jsonDoc.object ());
				}				
				else
				{
					LOG ("Malformed json: %s", STR (buffer))
					return false;
				}
				resetBuffers ();
			}
		}
	}
	return true;
}

//************************************************************************************************
// NetworkConnection
//************************************************************************************************

NetworkConnection::NetworkConnection (QObject* parent)
: QObject (parent),
  socket (this),
  reader (socket),
  writer (socket)
{
	connect (&socket, &QTcpSocket::readyRead, this, &NetworkConnection::readData);
	connect (&socket, &QTcpSocket::disconnected, this, &NetworkConnection::disconnectCompleted);
	connect (&reader, &NetworkReader::receivedJson, this, &NetworkConnection::parsedData);
	
	aliveTimer.start ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

NetworkConnection::~NetworkConnection ()
{
	disconnect (&socket, &QTcpSocket::readyRead, this, &NetworkConnection::readData);
	disconnect (&socket, &QTcpSocket::disconnected, this, &NetworkConnection::disconnectCompleted);
	disconnect (&reader, &NetworkReader::receivedJson, this, &NetworkConnection::parsedData);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkConnection::idle ()
{
	if(!aliveTimer.isValid ())
		return false;
	
	if(aliveTimer.hasExpired (kAliveMs))
	{
		LOG ("Connection alive expired. Disconnecting!")
		aliveTimer.invalidate ();
		terminate ();
		return false;
	}
	
	// I have to run the sockets synchronously, as the event loop isn't pumped
	socket.waitForReadyRead (0);
	socket.waitForBytesWritten (0);
	
	return true;
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkConnection::setDescriptor (qintptr descriptor)
{
	return socket.setSocketDescriptor (descriptor);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkConnection::parsedData (const QJsonObject& json)
{
	aliveTimer.start ();
	emit receivedJson (json, this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkConnection::readData ()
{
	if(!reader.read ())
	{
		aliveTimer.invalidate ();
		LOG ("NetworkConnection::readData failed")
		terminate ();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkConnection::writeJson (const QJsonObject& json)
{
	if(!writer.write (json))
	{
		terminate ();
		LOG ("NetworkConnection::write failed, disconnecting")
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkConnection::doWriteJson (const QJsonObject& json)
{
	// ensure it goes out on the correct thread
	QTimer::singleShot (0, this, std::bind (&NetworkConnection::writeJson, this, json));	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkConnection::terminate ()
{
	//LOG ("NetworkConnection::terminate")
	socket.disconnectFromHost ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkConnection::disconnectCompleted ()
{
	//LOG ("NetworkConnection::disconnectCompleted. Error? %s", STR (socket.errorString ()))
	aliveTimer.invalidate ();
	emit disconnectedFromClient (this);
}

