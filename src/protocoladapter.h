//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : protocoladapter.h
// Created by  : James Inkster, jinkster@presonus.com
// Description : Translates between OBS and remote clients via protocol
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

#include "obsremoteprotocol.h"
#include "statistics.h"
#include "frontend.h"

#include <QtCore/QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>

class NetworkServer;
class NetworkConnection;
class Source;
class Scene;
class SceneSource;

//************************************************************************************************
// ProtocolAdapter
//************************************************************************************************

class ProtocolAdapter : public QObject
{
	Q_OBJECT
public:
	ProtocolAdapter (NetworkServer& server);
	~ProtocolAdapter ();
	
	void sendValues (const QJsonArray& valuesArray, NetworkConnection* connection = 0);
	void send (const QJsonValue& item, NetworkConnection* connection = 0);
	
	void getAll (QJsonArray& valuesArray);
	QJsonValue get (const QString& name);
	QJsonValue set (const QString& name, const QVariant& value);
	
public slots:
	// Protocol:
	QJsonValue getCpuUsage () const;
	QJsonValue getMemoryUsage () const;
	QJsonValue getFreeDisk () const;
	QJsonValue getRecordingTime () const;
	QJsonValue getStreamingTime () const;
	QJsonValue getStudioMode () const;
	QJsonValue getTriggerTransition () const;
	QJsonValue getStreaming () const;
	QJsonValue getRecording () const;
	QJsonValue getSceneList () const;
	QJsonValue getSourceVisibles () const;
	QJsonValue getSourceLocks () const;
	QJsonValue getCurrentScene () const;
	QJsonValue getPreviewScene () const;
	QJsonValue getTransitionsList () const;
	QJsonValue getCurrentTransition () const;
	QJsonValue getTransitionDuration () const;
	QJsonValue getTotalFrames () const;
	QJsonValue getFps () const;
	QJsonValue getDroppedFrames () const;
	QJsonValue getCongestion () const;
	
	void setStudioMode (const QVariant& value);
	void setStreaming (const QVariant& value);
	void setRecording (const QVariant& value);
	void setSceneList (const QVariant& value);
	void setCurrentScene (const QVariant& value);
	void setPreviewScene (const QVariant& value);
	void setSourceLocks (const QVariant& value);
	void setSourceVisibles (const QVariant& value);
	void setTransitionsList (const QVariant& value);
	void setTriggerTransition (const QVariant& value);
	void setCurrentTransition (const QVariant& value);
	void setTransitionDuration (const QVariant& value);	
	
	// NetworkConnection:
	void receivedJson (const QJsonObject& json, NetworkConnection& connection);
	void connectionAdded (NetworkConnection& connection);
	void connectionRemoved (NetworkConnection& connection);
	
	// OBS Object Handling:
	void streamingStateChanged (bool isStreaming);
	void recordingStateChanged (bool isRecording);
	void studioModeChanged (bool isStudioMode);
	void transitionChanged ();
	void transitionDurationChanged ();
	void transitionListChanged ();
	void transitionStopped ();
	void sceneChanged ();
	void previewSceneChanged ();
	void sceneListChanged ();
	void sceneDestroyed (const Source& source);
	void sceneRemoved (const Source& source);
	void sceneActivated (const Source& source, bool isActive);
	void sceneShown (const Source& source, bool isVisible);
	void sceneEnabled (const Source& source, bool isEnabled);
	void sceneRenamed (const Source& source);
	void sceneSourceAdded (const Scene& scene, const SceneSource& source);
	void sceneSourceRemoved (const Scene& scene, const SceneSource& source);
	void sceneSourcesReordered (const Scene& scene);
	void sceneSourcesRefreshed (const Scene& scene);
	void sceneSourceVisibilityChanged (const Scene& scene, const SceneSource& source, bool visible);
	void sceneSourceLockChanged (const Scene& scene, const SceneSource& source, bool locked);
	
protected:
	enum RequestType
	{
		kSet = 0,
		kGet
	};
	static QString buildMethodName (const QString& name, RequestType requestType);
	QJsonValue getSceneSourceList (const Scene& parentScene) const;
	void connectScene (Scene& scene);
	void disconnectScene (Scene& scene);
	
	NetworkServer& server;
	Statistics stats;
	FrontEnd frontend;
};
