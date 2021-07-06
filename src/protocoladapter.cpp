//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : protocoladapter.cpp
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

#include "protocoladapter.h"
#include "enumerators.h"
#include "networkserver.h"
#include "networkconnection.h"
#include "obsobjects.h"

#include "moc_protocoladapter.cpp"

#define ENABLE_LOGGING 0
#include "common.h"

using namespace OBSRemoteProtocol;

//************************************************************************************************
// ProtocolAdapter
//************************************************************************************************

ProtocolAdapter::ProtocolAdapter (NetworkServer& server)
: server (server)
{
	connect (&server, &NetworkServer::receivedJson, this, &ProtocolAdapter::receivedJson);
	connect (&server, &NetworkServer::connectionAdded, this, &ProtocolAdapter::connectionAdded);
	connect (&server, &NetworkServer::connectionRemoved, this, &ProtocolAdapter::connectionRemoved);
	connect (&frontend, &FrontEnd::streamingStateChanged, this, &ProtocolAdapter::streamingStateChanged);
	connect (&frontend, &FrontEnd::recordingStateChanged, this, &ProtocolAdapter::recordingStateChanged);
	connect (&frontend, &FrontEnd::studioModeChanged, this, &ProtocolAdapter::studioModeChanged);
	connect (&frontend, &FrontEnd::transitionChanged, this, &ProtocolAdapter::transitionChanged);
	connect (&frontend, &FrontEnd::transitionListChanged, this, &ProtocolAdapter::transitionListChanged);
	connect (&frontend, &FrontEnd::transitionStopped, this, &ProtocolAdapter::transitionStopped);
	connect (&frontend, &FrontEnd::transitionDurationChanged, this, &ProtocolAdapter::transitionDurationChanged);
	connect (&frontend, &FrontEnd::sceneChanged, this, &ProtocolAdapter::sceneChanged);
	connect (&frontend, &FrontEnd::previewSceneChanged, this, &ProtocolAdapter::previewSceneChanged);
	connect (&frontend, &FrontEnd::sceneListChanged, this, &ProtocolAdapter::sceneListChanged);
	
	sceneChanged (); // connect to the active scene...
}

//////////////////////////////////////////////////////////////////////////////////////////////////

ProtocolAdapter::~ProtocolAdapter ()
{
	if(Scene* scene = frontend.getCurrentScene ())
		disconnectScene (*scene);
	if(Scene* scene = frontend.getPreviewScene ())
		disconnectScene (*scene);
	
	disconnect (&server, &NetworkServer::receivedJson, this, &ProtocolAdapter::receivedJson);
	disconnect (&server, &NetworkServer::connectionAdded, this, &ProtocolAdapter::connectionAdded);
	disconnect (&server, &NetworkServer::connectionRemoved, this, &ProtocolAdapter::connectionRemoved);
	disconnect (&frontend, &FrontEnd::streamingStateChanged, this, &ProtocolAdapter::streamingStateChanged);
	disconnect (&frontend, &FrontEnd::recordingStateChanged, this, &ProtocolAdapter::recordingStateChanged);
	disconnect (&frontend, &FrontEnd::studioModeChanged, this, &ProtocolAdapter::studioModeChanged);
	disconnect (&frontend, &FrontEnd::transitionChanged, this, &ProtocolAdapter::transitionChanged);
	disconnect (&frontend, &FrontEnd::transitionListChanged, this, &ProtocolAdapter::transitionListChanged);
	disconnect (&frontend, &FrontEnd::transitionStopped, this, &ProtocolAdapter::transitionStopped);
	disconnect (&frontend, &FrontEnd::transitionDurationChanged, this, &ProtocolAdapter::transitionDurationChanged);
	disconnect (&frontend, &FrontEnd::sceneChanged, this, &ProtocolAdapter::sceneChanged);
	disconnect (&frontend, &FrontEnd::previewSceneChanged, this, &ProtocolAdapter::previewSceneChanged);
	disconnect (&frontend, &FrontEnd::sceneListChanged, this, &ProtocolAdapter::sceneListChanged);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::connectScene (Scene& scene)
{
	connect (&scene, &Source::destroyed, this, &ProtocolAdapter::sceneDestroyed);
	connect (&scene, &Source::removed, this, &ProtocolAdapter::sceneRemoved);
	connect (&scene, &Source::activated, this, &ProtocolAdapter::sceneActivated);
	connect (&scene, &Source::shown, this, &ProtocolAdapter::sceneShown);
	connect (&scene, &Source::enabled, this, &ProtocolAdapter::sceneEnabled);
	connect (&scene, &Source::renamed, this, &ProtocolAdapter::sceneRenamed);
	
	connect (&scene, &Scene::sceneSourceAdded, this, &ProtocolAdapter::sceneSourceAdded);
	connect (&scene, &Scene::sceneSourceRemoved, this, &ProtocolAdapter::sceneSourceRemoved);
	connect (&scene, &Scene::sceneSourcesReordered, this, &ProtocolAdapter::sceneSourcesReordered);
	connect (&scene, &Scene::sceneSourcesRefreshed, this, &ProtocolAdapter::sceneSourcesRefreshed);
	connect (&scene, &Scene::sceneSourceVisibilityChanged, this, &ProtocolAdapter::sceneSourceVisibilityChanged);
	connect (&scene, &Scene::sceneSourceLockChanged, this, &ProtocolAdapter::sceneSourceLockChanged);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::disconnectScene (Scene& scene)
{
	disconnect (&scene, &Source::destroyed, this, &ProtocolAdapter::sceneDestroyed);
	disconnect (&scene, &Source::removed, this, &ProtocolAdapter::sceneRemoved);
	disconnect (&scene, &Source::activated, this, &ProtocolAdapter::sceneActivated);
	disconnect (&scene, &Source::shown, this, &ProtocolAdapter::sceneShown);
	disconnect (&scene, &Source::enabled, this, &ProtocolAdapter::sceneEnabled);
	disconnect (&scene, &Source::renamed, this, &ProtocolAdapter::sceneRenamed);
	
	disconnect (&scene, &Scene::sceneSourceAdded, this, &ProtocolAdapter::sceneSourceAdded);
	disconnect (&scene, &Scene::sceneSourceRemoved, this, &ProtocolAdapter::sceneSourceRemoved);
	disconnect (&scene, &Scene::sceneSourcesReordered, this, &ProtocolAdapter::sceneSourcesReordered);
	disconnect (&scene, &Scene::sceneSourcesRefreshed, this, &ProtocolAdapter::sceneSourcesRefreshed);
	disconnect (&scene, &Scene::sceneSourceVisibilityChanged, this, &ProtocolAdapter::sceneSourceVisibilityChanged);
	disconnect (&scene, &Scene::sceneSourceLockChanged, this, &ProtocolAdapter::sceneSourceLockChanged);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::send (const QJsonValue& item, NetworkConnection* connection)
{
	QJsonArray valuesArray;
	valuesArray.append (item);
	sendValues (valuesArray, connection);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sendValues (const QJsonArray& valuesArray, NetworkConnection* connection)
{
	QJsonObject object;
	object[kValuesArray] = valuesArray;

	if(connection)
		server.sendJson (*connection, object);
	else
		server.broadcastJson (object);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::set (const QString& name, const QVariant& value)
{
	QJsonValue item;
	QString methodName = buildMethodName (name, kSet);
	//LOG ("set methodName %s", STR (methodName))
	if(!QMetaObject::invokeMethod (this, STR (methodName), Qt::DirectConnection, Q_ARG (QVariant, value)))
	{
		LOG ("ProtocolAdapter::set: unhandled protocol request '%s' (%s)", STR (name), STR (methodName))
	}
	return item;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::get (const QString& name)
{
	QJsonObject item;
	QString methodName = buildMethodName (name, kGet);
	//LOG ("get methodName %s", STR (methodName))
	QJsonValue retVal;
	if(QMetaObject::invokeMethod (this, STR (methodName), Qt::DirectConnection, Q_RETURN_ARG (QJsonValue, retVal)))
	{
		item[kValueItemName] = name;
		item[kValueItemValue] = retVal;
		item[kValueItemType] = kValueItemTypeSet; // if they requested a 'get', we respond with a set
	}
	else 
	{
		LOG ("ProtocolAdapter::get: unhandled protocol request '%s' (%s)", STR (name), STR (methodName))
	}
	return item;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::getAll (QJsonArray& valuesArray)
{
	for(int i = 0; i < ARRAY_COUNT (kValueItemNames); i++)
	{
		//LOG ("getAll: %s", kValueItemNames[i])
		QJsonValue item = get (kValueItemNames[i]);
		if(item.isNull ())
		{
			LOG ("Warning: getAll (%s) returned NULL entry", STR (kValueItemNames[i]))
		}
		else
			valuesArray.append (item);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::receivedJson (const QJsonObject& json, NetworkConnection& connection)
{
	QJsonArray getResults;
	const QJsonArray valuesArray = json[kValuesArray].toArray ();
	for(auto value : valuesArray) 
	{
		const QJsonObject item = value.toObject ();
		RequestType requestType = (item[kValueItemType].toString ().compare (kValueItemTypeSet, Qt::CaseInsensitive) == 0) ? kSet : kGet;
		QString name = item[kValueItemName].toString ();
		switch(requestType)
		{
		case kGet :
			{
				getResults.append (get (name));
				//LOG ("ProtocolAdapter::parseJson GET %s", STR (name))
			} break;
				
		case kSet :
			{
				QJsonValue value = item[kValueItemValue];
				//LOG ("ProtocolAdapter::parseJson SET value type %d, %d", value.type (), value.toBool ())
				set (name, value);		 
			} break;
		default:
			LOG ("ProtocolAdapter::receivedJson unhandled requestType %s", STR (item[kValueItemType].toString ()))
			break;
		}
	}
	
	//LOG ("getResults count = %d", getResults.count ())
	if(!getResults.isEmpty ())
		sendValues (getResults, &connection);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::connectionAdded (NetworkConnection& connection)
{
	LOG ("ProtocolAdapter::connectionAdded")
	QJsonArray valuesArray;
	getAll (valuesArray);
	sendValues (valuesArray, &connection);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::connectionRemoved (NetworkConnection& connection)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneDestroyed (const Source& source)
{
	//LOG ("Scene Destroyed: %s", STR (source.getName ()))
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneRemoved (const Source& source)
{
	LOG ("Scene Removed: %s", STR (source.getName ()))
	send (get (OBSRemoteProtocol::kItemSceneList));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneActivated (const Source& source, bool isActive)
{
	LOG ("Scene Activated: %s (%d)", STR (source.getName ()), isActive)
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneShown (const Source& source, bool isVisible)
{
	LOG ("Scene Shown: %s (%d)", STR (source.getName ()), isVisible)
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneEnabled (const Source& source, bool isEnabled)
{
	LOG ("Scene Enabled: %s (%d)", STR (source.getName ()), isEnabled)
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneRenamed (const Source& source)
{
	LOG ("Scene Renamed: %s", STR (source.getName ()))
	send (get (OBSRemoteProtocol::kItemSceneList));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneSourceAdded (const Scene& scene, const SceneSource& source)
{
	LOG ("Scene Source Added: %s", STR (source.getName ()))
	send (get (OBSRemoteProtocol::kItemSceneList));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneSourceRemoved (const Scene& scene, const SceneSource& source)
{
	LOG ("Scene Source Removed: %s", STR (source.getName ()))
	send (get (OBSRemoteProtocol::kItemSceneList));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneSourcesReordered (const Scene& scene)
{
	LOG ("Scene Source Reordered")
	send (get (OBSRemoteProtocol::kItemSceneList));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneSourcesRefreshed (const Scene& scene)
{
	LOG ("Scene Source Refreshed")
	send (get (OBSRemoteProtocol::kItemSceneList));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneSourceVisibilityChanged (const Scene& scene, const SceneSource& source, bool visible)
{
	LOG ("Scene Source Visibilty Changed: %s", STR (source.getName ()))
	send (get (OBSRemoteProtocol::kItemSceneList));
	send (get (OBSRemoteProtocol::kItemSceneSourcesVisibles));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneSourceLockChanged (const Scene& scene, const SceneSource& source, bool locked)
{
	LOG ("Scene Source Lock changed: %s", STR (source.getName ()))
	send (get (OBSRemoteProtocol::kItemSceneList));
	send (get (OBSRemoteProtocol::kItemSceneSourcesLocks));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::transitionDurationChanged ()
{
	LOG ("transitionDurationChanged (%d)", frontend.getTransitionDuration ())
	send (get (OBSRemoteProtocol::kItemTransitionCurrentDuration));
	send (get (OBSRemoteProtocol::kItemTransitionList));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::streamingStateChanged (bool isStreaming)
{
	send (get (OBSRemoteProtocol::kItemStreaming));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::recordingStateChanged (bool isRecording)
{
	send (get (OBSRemoteProtocol::kItemRecording));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::studioModeChanged (bool isStudioMode)
{
	send (get (OBSRemoteProtocol::kItemStudioMode));
	send (get (OBSRemoteProtocol::kItemSceneList));
	send (get (OBSRemoteProtocol::kItemCurrentScene));
	send (get (OBSRemoteProtocol::kItemPreviewScene));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneListChanged ()
{
	send (get (OBSRemoteProtocol::kItemSceneList));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::sceneChanged ()
{
	if(Scene* scene = frontend.getCurrentScene ())
		connectScene (*scene);
	send (get (OBSRemoteProtocol::kItemSceneList));
	send (get (OBSRemoteProtocol::kItemCurrentScene));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::previewSceneChanged ()
{
	if(Scene* scene = frontend.getPreviewScene ())
		connectScene (*scene);
	send (get (OBSRemoteProtocol::kItemSceneList));
	send (get (OBSRemoteProtocol::kItemCurrentScene));
	send (get (OBSRemoteProtocol::kItemPreviewScene));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::transitionChanged ()
{
	send (get (OBSRemoteProtocol::kItemTransitionList));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::transitionListChanged ()
{
	send (get (OBSRemoteProtocol::kItemTransitionList));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::transitionStopped ()
{
	send (get (OBSRemoteProtocol::kItemTriggerTransition));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString ProtocolAdapter::buildMethodName (const QString& name, RequestType requestType)
{
	// ie., The goal here is to turn a protocol string ('currentScene') into a method name ('getCurrentScene')
	QString methodName = name.left (1).toUpper () + name.mid (1); 
	
	switch(requestType)
	{
	case kGet :
		return kValueItemTypeGet + methodName;
	case kSet :
		return kValueItemTypeSet + methodName;
	}
	return "";
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getCpuUsage () const
{
	return stats.getCpuUsage ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getMemoryUsage () const
{
	return stats.getMemoryUsage ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getFreeDisk () const
{
	return stats.getFreeDisk ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getStudioMode () const
{
	return frontend.isStudioMode ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getTriggerTransition () const
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getStreaming () const
{
	return frontend.isStreaming ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getRecording () const
{
	return frontend.isRecording ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getRecordingTime () const
{
	Output* output = frontend.getRecordingOutput ();
	if(!output)
		return Output::kNoTimeString;
	return output->getTimeString ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getStreamingTime () const
{
	Output* output = frontend.getStreamingOutput ();
	if(!output)
		return Output::kNoTimeString;
	return output->getTimeString ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getTotalFrames () const
{
	Output* output = frontend.getStreamingOutput ();
	if(!output)
		return "--";
	return output->getTotalFrames ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getDroppedFrames () const
{
	Output* output = frontend.getStreamingOutput ();
	if(!output)
		return "--";
	return output->getDroppedFrames ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getCongestion () const
{
	Output* output = frontend.getStreamingOutput ();
	if(!output)
		return "--";
	return output->getCongestionString ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getFps () const
{
	Output* output = frontend.getStreamingOutput ();
	if(!output)
		return "--";
	return output->getFramesPerSecondString ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getSceneList () const
{
	LOG ("getScenelist:")
	
	QVector<Scene*> scenes;
	Enumerators::Scenes enumerator (scenes);
	
	QJsonArray scenesArray;
	int sortIndex = 0;
	Scene* previewScene = frontend.getPreviewScene ();
	Scene* currentScene = frontend.getCurrentScene ();
	bool studioMode = frontend.isStudioMode ();
	for(auto i : scenes)
	{
		QJsonObject scene = i->toJson ();
		scene[OBSRemoteProtocol::kSourceSortIndex] = sortIndex;
		bool isCurrent = false;
		if(studioMode)
		{
			if(previewScene && i->getInternal () == previewScene->getInternal ())
				isCurrent = true;
		}
		else
		{	
			if(currentScene && i->getInternal () == currentScene->getInternal ())
				isCurrent = true;
		}
		scene[kSourceIsCurrent] = isCurrent;
		
		LOG ("\t%s %s", STR (i->getName ()), isCurrent ? "[CURRENT]" : "")
		
		// embed each scene's items, as well.
		scene[OBSRemoteProtocol::kSceneSourcesList] = getSceneSourceList (*i);
		scenesArray.push_back (scene);
		
		sortIndex++;
	}
	return scenesArray;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getSourceVisibles () const
{
	Scene* currentScene = frontend.getCurrentScene ();
	if(!currentScene)
		return "";
	
	const QVector<SceneSource*>& sceneSources = currentScene->getSources ();
	int index = -1;
	int bits = 0;
	for(auto i : sceneSources)
	{
		index++;
		if(i->isVisible ()) 
			bits |= (1<<index); 
		else
			bits &= ~(1<<index); 
	}
	LOG ("getSourceVisibles sending %d", bits)

	return bits;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getSourceLocks () const
{
	Scene* currentScene = frontend.getCurrentScene ();
	if(!currentScene)
		return "";
	
	const QVector<SceneSource*>& sceneSources = currentScene->getSources ();
	int index = -1;
	int bits = 0;
	for(auto i : sceneSources)
	{
		index++;
		if(i->isLocked ()) 
			bits |= (1<<index); 
		else
			bits &= ~(1<<index); 
	}
	
	LOG ("getSourceLocks sending %d", bits)
	return bits;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getCurrentScene () const
{
	Scene* currentScene = frontend.getCurrentScene ();
	return (currentScene ? currentScene->getName () : "");
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getPreviewScene () const
{
	Scene* previewScene = frontend.getPreviewScene ();
	if(!previewScene)
	{
		LOG ("ProtocolAdapter::getPreviewScene no preview scene")
	}
	return (previewScene ? previewScene->getName () : "");
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getSceneSourceList (const Scene& parentScene) const
{
	//LOG ("getSceneSourceList")
	
	QJsonArray items;
	int sortIndex = 0;
	const QVector<SceneSource*> sceneSources = parentScene.getSources ();
	for(auto i : sceneSources)
	{
		QJsonObject sceneSource = i->toJson ();
		sceneSource[OBSRemoteProtocol::kSourceSortIndex] = sortIndex;
		items.push_back (sceneSource);
		
		sortIndex++;
	}
	return items;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getTransitionsList () const
{
	//LOG ("getTransitionsList")
	
	QVector<Transition*> transitions;
	Enumerators::Transitions other (transitions);
	QJsonArray transitionsArray;
	int sortIndex = 0;
	Transition* currentTransition = frontend.getCurrentTransition ();
	for(auto i : transitions)
	{
		QJsonObject transition = i->toJson ();
		transition[OBSRemoteProtocol::kSourceSortIndex] = sortIndex;
		bool isCurrent = false;
		if(currentTransition && i->getInternal () == currentTransition->getInternal ())
		{
			isCurrent = true;
			transition[kTransitionDuration] = frontend.getTransitionDuration ();
		}
		transition[kSourceIsCurrent] = isCurrent;
		transitionsArray.push_back (transition);
		
		sortIndex++;
	}
	return transitionsArray;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getCurrentTransition () const
{
	Transition* currentTransition = frontend.getCurrentTransition ();
	return (currentTransition ? currentTransition->getName () : "");
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonValue ProtocolAdapter::getTransitionDuration () const
{
	return frontend.getTransitionDuration ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setStudioMode (const QVariant& value)
{
	bool ok = false;
	double doubleVal = value.toDouble (&ok);
	if(!ok)
	{
		LOG ("setStudioMode expected Integer, got %s", value.typeName ())
		return;
	}
	//LOG ("setStudiomode %.2f type %s", doubleVal, value.typeName ())
	frontend.setStudioMode (doubleVal > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setStreaming (const QVariant& value)
{
	bool ok = false;
	double doubleVal = value.toDouble (&ok);
	if(!ok)
	{
		LOG ("setStreaming expected Integer, got %s", value.typeName ())
		return;
	}
	//LOG ("setStreaming %.2f type %s", doubleVal, value.typeName ())
	frontend.setStreaming (doubleVal > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setRecording (const QVariant& value)
{
	bool ok = false;
	double doubleVal = value.toDouble (&ok);
	if(!ok)
	{
		LOG ("setRecording expected Integer, got %s", value.typeName ())
		return;
	}
	//LOG ("setRecording %.2f type %s", doubleVal, value.typeName ())
	frontend.setRecording (doubleVal > 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setSceneList (const QVariant& value)
{
	if(frontend.isStudioMode ())
		setPreviewScene (value);
	else
		setCurrentScene (value);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setCurrentScene (const QVariant& value)
{
	QString sceneName = value.toString ();
	LOG ("ProtocolAdapter::setCurrentScene: %s", STR (sceneName))
	frontend.setCurrentScene (sceneName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setPreviewScene (const QVariant& value)
{
	QString sceneName = value.toString ();
	LOG ("ProtocolAdapter::setPreviewScene: %s", STR (sceneName))
	frontend.setPreviewScene (sceneName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setSourceLocks (const QVariant& value)
{
	bool ok = false;
	quint32 bits = value.toInt (&ok);
	if(!ok)
	{
		LOG ("setSourceLocks expected int, got %s", value.typeName ())
		return;
	}
	LOG ("setSourceLocks %d type %s", bits, value.typeName ())
	
	Scene* currentScene = frontend.getCurrentScene ();
	if(!currentScene)
		return;
	
	const QVector<SceneSource*>& sceneSources = currentScene->getSources ();
	int index = -1;
	for(auto i : sceneSources)
	{
		index++;
		bool isLocked = (bits & (1<<index)) != 0;
		if(i->isLocked () != isLocked)
			i->setLocked (isLocked);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setSourceVisibles (const QVariant& value)
{
	bool ok = false;
	quint32 bits = value.toInt (&ok);
	if(!ok)
	{
		LOG ("setSourceVisibles expected int, got %s", value.typeName ())
		return;
	}
	LOG ("setSourceVisibles %d type %s", bits, value.typeName ())
	
	Scene* currentScene = frontend.getCurrentScene ();
	if(!currentScene)
		return;
	
	const QVector<SceneSource*>& sceneSources = currentScene->getSources ();
	int index = -1;
	for(auto i : sceneSources)
	{
		index++;
		bool isVisible = (bits & (1<<index)) != 0;
		if(i->isVisible () != isVisible)
			i->setVisible (isVisible);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setTriggerTransition (const QVariant& value)
{
	LOG ("setTriggerTransition")
	frontend.startTransition ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setCurrentTransition (const QVariant& value)
{
	frontend.setCurrentTransition (value.toString ());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setTransitionsList (const QVariant& value)
{
	LOG	("setTransitionsList")
	setCurrentTransition (value);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProtocolAdapter::setTransitionDuration (const QVariant& value)
{
	bool ok = false;
	int duration = value.toInt (&ok);
	if(!ok)
	{
		LOG ("setCurrentTransitionDuration expected integer, got %s", value.typeName ())
		return;
	}
	//LOG ("setCurrentTransitionDuration: %d type %s", duration, value.typeName ())
	frontend.setTransitionDuration (duration);
}
