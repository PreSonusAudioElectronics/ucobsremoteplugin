//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : frontend.h
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

#pragma once

#include <obs-frontend-api.h>
#include <QtCore/QObject>
#include <QString>

class Scene;
class Transition;
class Output;
class QSpinBox;
class QListView;

//************************************************************************************************
// FrontEnd
//************************************************************************************************

class FrontEnd : public QObject
{
	Q_OBJECT
public:
	FrontEnd ();
	~FrontEnd ();
	
	void handleEvent (enum obs_frontend_event event);
	
	Scene* getCurrentScene () const;
	void setCurrentScene (const QString& sceneName);
	Scene* getPreviewScene () const;
	void setPreviewScene (const QString& sceneName);
	Transition* getCurrentTransition () const;
	Output* getRecordingOutput () const;
	Output* getStreamingOutput () const;

	bool isStudioMode () const;
	bool isStreaming () const;
	bool isRecording () const;
	int getTransitionDuration () const;
	void setStudioMode (bool isStudio);
	void setStreaming (bool isStreaming);
	void setRecording (bool isRecording);
	void startTransition ();
	void setCurrentTransition (const QString& transitionName);
	void setTransitionDuration (int duration);
	
signals:
	void streamingStateChanged (bool isStreaming);
	void recordingStateChanged (bool isRecording);
	void studioModeChanged (bool isStudioMode);
	void sceneListChanged ();
	void sceneChanged ();
	void previewSceneChanged ();
	void transitionChanged ();
	void transitionListChanged ();
	void transitionStopped ();
	void transitionDurationChanged ();
	
protected:
	Scene* currentScene;
	Scene* previewScene;
	Transition* currentTransition;
	mutable Output* recordingOutput; // need to be mutable because we don't get notified when they change..
	mutable Output* streamingOutput; // need to be mutable because we don't get notified when they change..
	
	void rebuildCurrentScene ();
	void rebuildPreviewScene ();
	void rebuildCurrentTransition ();
	QSpinBox* findTransitionDurationSpinner () const;
	QListView* findSceneTree () const;
};
