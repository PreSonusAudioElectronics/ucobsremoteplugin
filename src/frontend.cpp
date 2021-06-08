//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : frontend.cpp
// Created by  : James Inkster, jinkster@presonus.com
// Description : Wrapper of OBS frontend functionality
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

#include "frontend.h"
#include "enumerators.h"
#include "obsobjects.h"

#include "moc_frontend.cpp"

#define ENABLE_LOGGING 0
#include "common.h"

#include <QMainWindow>
#include <QSpinBox>
#include <QListView>

//************************************************************************************************
// FrontEnd
//************************************************************************************************

FrontEnd::FrontEnd ()
: currentScene (0),
  previewScene (0),
  currentTransition (0),
  recordingOutput (0),
  streamingOutput (0)
{
	auto eventCallback = [](enum obs_frontend_event event, void* handler) 
	{
		reinterpret_cast<FrontEnd*> (handler)->handleEvent (event);
	};
	obs_frontend_add_event_callback (eventCallback, this);
	rebuildCurrentScene ();
	rebuildPreviewScene ();
	rebuildCurrentTransition ();
	
	if(QSpinBox* spinner = findTransitionDurationSpinner ())
		connect (spinner, QOverload<int>::of (&QSpinBox::valueChanged), this, [=] () { emit transitionDurationChanged (); });
	if(QListView* sceneTree = findSceneTree ())
		if(QAbstractItemModel* model = sceneTree->model ())
			connect (model, &QAbstractItemModel::rowsMoved, this, [=] () { emit sceneListChanged (); });
}

//////////////////////////////////////////////////////////////////////////////////////////////////

FrontEnd::~FrontEnd ()
{
	delete currentScene;
	delete previewScene;
	delete currentTransition;
	delete recordingOutput;
	delete streamingOutput;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::handleEvent (enum obs_frontend_event event)
{
	LOG ("FrontEnd::handleEvent %d", event)
	
	switch(event)
	{
	case OBS_FRONTEND_EVENT_EXIT :
		{
			//LOG ("OBS_FRONTEND_EVENT_EXIT");			
			obs_frontend_remove_event_callback ((obs_frontend_event_cb)event, nullptr);
		} break;
			
	case OBS_FRONTEND_EVENT_STREAMING_STARTED :
		emit streamingStateChanged (true);
		break;
			
	case OBS_FRONTEND_EVENT_STREAMING_STOPPED :
		emit streamingStateChanged (false);
		break;
			
	case OBS_FRONTEND_EVENT_RECORDING_STARTED :
		emit recordingStateChanged (true);
		break;
			
	case OBS_FRONTEND_EVENT_RECORDING_STOPPED :
		emit recordingStateChanged (false);
		break;
			
	case OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED :
		rebuildPreviewScene ();
		emit studioModeChanged (true);
		break;
			
	case OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED :
		emit studioModeChanged (false);
		break;		
			
	case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED :
		if(isStudioMode ())
		{
			rebuildPreviewScene ();
			emit previewSceneChanged ();
		}
		break;
			
	case OBS_FRONTEND_EVENT_TRANSITION_CHANGED :
		rebuildCurrentTransition ();
		emit transitionChanged ();
		break;
				
	case OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED :
		emit transitionListChanged ();
		break;	
				
	case OBS_FRONTEND_EVENT_TRANSITION_STOPPED :
		emit transitionStopped ();
		break;		
			
	case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED :
		emit sceneListChanged ();
		break;
			
	case OBS_FRONTEND_EVENT_SCENE_CHANGED :
		//LOG ("FrontEnd: OBS_FRONTEND_EVENT_SCENE_CHANGED")
		rebuildCurrentScene ();
		emit sceneChanged ();
		break;
			
	default :
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::rebuildCurrentScene ()
{
	LOG ("FrontEnd::rebuildCurrentScene")
	if(currentScene)
	{
		delete currentScene;
		currentScene = 0;
	}
	
	AutoReleaseSource obsScene = obs_frontend_get_current_scene ();
	if(obsScene)
	{
		currentScene = new Scene (*obsScene);
		LOG ("currentScene is now %s", STR (currentScene->getName ()))
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::rebuildPreviewScene ()
{
	LOG ("FrontEnd::rebuildPreviewScene")
	if(previewScene)
	{
		delete previewScene;
		previewScene = 0;
	}
	
	AutoReleaseSource obsScene = obs_frontend_get_current_preview_scene ();
	if(obsScene)
	{
		previewScene = new Scene (*obsScene);
		LOG ("previewScene is now %s", STR (previewScene->getName ()))
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::rebuildCurrentTransition ()
{
	if(currentTransition)
	{
		delete currentTransition;
		currentTransition = 0;
	}
	
	AutoReleaseSource obsTransition = obs_frontend_get_current_transition ();
	if(obsTransition)
		currentTransition = new Transition (*obsTransition);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Scene* FrontEnd::getCurrentScene () const
{
	return currentScene;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::setCurrentScene (const QString& sceneName)
{
	AutoReleaseSource source = obs_get_source_by_name (STR (sceneName)); // increments reference...
	if(!source)
	{
		LOG ("Warning: setCurrentScene failed: couldn't find scene %s", STR (sceneName))
		return;
	}
	
	LOG ("setCurrentScene: %s", STR (sceneName))
	obs_frontend_set_current_scene (source);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Scene* FrontEnd::getPreviewScene () const
{
	return previewScene;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::setPreviewScene (const QString& sceneName)
{
	AutoReleaseSource source = obs_get_source_by_name (STR (sceneName)); // increments reference...
	if(!source)
	{
		LOG ("Warning: setCurrentScene failed: couldn't find scene %s", STR (sceneName))
		return;
	}
	
	LOG ("setPreviewScene: %s", STR (sceneName))
	obs_frontend_set_current_preview_scene (source);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Transition* FrontEnd::getCurrentTransition () const
{
	return currentTransition;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Output* FrontEnd::getRecordingOutput () const
{
	AutoReleaseOutput obsOutput = obs_frontend_get_recording_output ();
	if(!recordingOutput)
		recordingOutput = new Output (*obsOutput);
	recordingOutput->updateInternal (*obsOutput);
	return recordingOutput;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Output* FrontEnd::getStreamingOutput () const
{
	AutoReleaseOutput obsOutput = obs_frontend_get_streaming_output ();
	if(!streamingOutput)
		streamingOutput = new Output (*obsOutput);
	streamingOutput->updateInternal (*obsOutput);
	return streamingOutput;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

QSpinBox* FrontEnd::findTransitionDurationSpinner () const
{
	if(QMainWindow* mainWindow = reinterpret_cast<QMainWindow*> (obs_frontend_get_main_window ()))
		return mainWindow->findChild<QSpinBox*> ("transitionDuration");
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

QListView* FrontEnd::findSceneTree () const
{
	if(QMainWindow* mainWindow = reinterpret_cast<QMainWindow*> (obs_frontend_get_main_window ()))
		return mainWindow->findChild<QListView*> ("scenes");
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool FrontEnd::isStudioMode () const
{
	return obs_frontend_preview_program_mode_active ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool FrontEnd::isStreaming () const
{
	return obs_frontend_streaming_active ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool FrontEnd::isRecording () const
{
	return obs_frontend_recording_active ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int FrontEnd::getTransitionDuration () const
{
	return obs_frontend_get_transition_duration ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::setStudioMode (bool isStudio)
{
	obs_frontend_set_preview_program_mode (isStudio);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::setStreaming (bool isStreaming)
{
	isStreaming ? obs_frontend_streaming_start () : obs_frontend_streaming_stop ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::setRecording (bool isRecording)
{
	isRecording ? obs_frontend_recording_start () : obs_frontend_recording_stop ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::startTransition ()
{
	LOG ("startTransition")
	if(obs_frontend_preview_program_mode_active ())
		obs_frontend_preview_program_trigger_transition ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::setCurrentTransition (const QString& transitionName)
{
	QVector<Transition*> transitions;
	Enumerators::Transitions other (transitions);
	for(auto i : transitions)
	{
		if(i->getName () == transitionName)
		{
			obs_frontend_set_current_transition (i->getInternal ());
			return;
		}
	}
	LOG ("Warning: setCurrentTransition: couldn't find transition %s", STR (transitionName))
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void FrontEnd::setTransitionDuration (int duration)
{
	obs_frontend_set_transition_duration (duration);
}
