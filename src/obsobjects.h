//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : obsobjects.h
// Created by  : James Inkster, jinkster@presonus.com
// Description : Objects used to represent OBS Sources, etc
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
#include <QString>
#include <QVector>
#include <QJsonObject>
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/platform.h>
#include <obs.hpp>

//************************************************************************************************
// Source
//************************************************************************************************

class Source : public QObject
{
	Q_OBJECT
public:
	Source (obs_source_t& source);
	virtual ~Source ();
	
	QString getName () const;
	QString getId () const;
	
	obs_source_t* getInternal () const { return source; }
	
	virtual QJsonObject toJson () const;	
	virtual void debug ();
	virtual QString getTypeString () const = 0;
	
	static void onDestroyed (void* param, calldata_t* data);
	static void onRemoved (void* param, calldata_t* data);
	static void onActivated (void* param, calldata_t* data);
	static void onDeactivated (void* param, calldata_t* data);
	static void onShown (void* param, calldata_t* data);
	static void onHidden (void* param, calldata_t* data);
	static void onEnabled (void* param, calldata_t* data);
	static void onRenamed (void* param, calldata_t* data);
	
	static void onSceneItemAdded (void* param, calldata_t* data);
	static void onSceneItemRemoved (void* param, calldata_t* data);
	static void onSceneItemReordered (void* param, calldata_t* data);
	static void onSceneItemRefreshed (void* param, calldata_t* data);
	static void onSceneItemVisibilityChanged (void* param, calldata_t* data);
	static void onSceneItemLockChanged (void* param, calldata_t* data);
	
signals:
	void destroyed (Source* source); ///< the source is about to be destroyed
	void removed (const Source* source); ///< the source has been removed
	void activated (const Source* source, bool isActive); ///< the source has been activated/deactivated in the main view (visible on stream/recording)
	void shown (const Source* source, bool isVisible); ///< the source is visible/hidden on any display and/or on the main view
	void enabled (const Source* source, bool isEnabled); ///< the source has been disabled/enabled
	void renamed (const Source* source); ///< the source has been renamed
	
protected:
	OBSSource source; // ref-counted
};

//************************************************************************************************
// SceneSource
//************************************************************************************************

class SceneSource : public Source
{
public:
	SceneSource (obs_sceneitem_t& item);
	~SceneSource ();
	
	obs_sceneitem_t* getInternalSceneItem () const { return item; }
	
	bool isVisible () const;
	void setVisible (bool state);
	bool isLocked () const;
	void setLocked (bool state);
	
	// Source
	QString getTypeString () const override;
	virtual QJsonObject toJson () const override;
	
protected:
	OBSSceneItem item; // ref-counted
};

//************************************************************************************************
// Scene
//************************************************************************************************

class Scene : public Source
{
	Q_OBJECT
public:
	Scene (obs_source_t& source);
	~Scene ();
	
	obs_scene_t* getInternalScene () const;
	const QVector<SceneSource*> getSources () const { return sceneSources; }
	SceneSource* findSceneSource (obs_sceneitem_t& obsSceneItem) const;
	
	void handleSceneItemAdded (calldata_t* data);
	void handleSceneItemRemoved (calldata_t* data);
	void handleSceneItemsReordered (calldata_t* data);
	void handleSceneItemsRefreshed (calldata_t* data);
	void handleSceneItemVisibilityChanged (calldata_t* data);
	void handleSceneItemLockChanged (calldata_t* data);
	
	// Source
	QString getTypeString () const override;
	
signals:
	void sceneSourceAdded (const Scene* scene, const SceneSource* source); ///< Called when a scene source has been added to the scene
	void sceneSourceRemoved (const Scene* scene, const SceneSource* source); ///< Called when a scene source has been removed from the scene
	void sceneSourcesReordered (const Scene* scene); ///< Called when scene sources have been reoredered in the scene
	void sceneSourcesRefreshed (const Scene* scene); ///< Called when the entire scene sources list needs to be refreshed. Usually this is only used when groups have changed
	void sceneSourceVisibilityChanged (const Scene* scene, const SceneSource* source, bool visible); ///<  Called when a scene source's visibility state changes
	void sceneSourceLockChanged (const Scene* scene, const SceneSource* source, bool locked); ///<  Called when a scene source has been locked or unlocked
	
protected:
	void regenerateSources ();
	
	QVector<SceneSource*> sceneSources;
};

//************************************************************************************************
// Input
//************************************************************************************************

class Input : public Source
{
public:
	Input (obs_source_t& source);
	
	// Source
	QString getTypeString () const override;
};

//************************************************************************************************
// Filter
//************************************************************************************************

class Filter : public Source
{
public:
	Filter (obs_source_t& source);
	
	// Source
	QString getTypeString () const override;
};

//************************************************************************************************
// Transition
//************************************************************************************************

class Transition : public Source
{
public:
	Transition (obs_source_t& source);
	
	// Source
	QString getTypeString () const override;
};

//************************************************************************************************
// AudioMeter
//************************************************************************************************

class AudioMeter
{
public:
	AudioMeter (obs_source_t& source);
	~AudioMeter ();
	
	void updateLevel (const float magnitude[MAX_AUDIO_CHANNELS], const float peak[MAX_AUDIO_CHANNELS], const float input_peak[MAX_AUDIO_CHANNELS]);
private:
	static void handleLevel (void* meter, const float magnitude[MAX_AUDIO_CHANNELS], const float peak[MAX_AUDIO_CHANNELS], const float input_peak[MAX_AUDIO_CHANNELS]);
	
	obs_volmeter_t* meter;
};

//************************************************************************************************
// Output
//************************************************************************************************

class Output
{
public:
	Output (obs_output_t& output);
	virtual ~Output ();
	
	obs_output_t* getInternal () const;
	void updateInternal (obs_output_t& output);
	
	QString getName () const;
	QString getId () const;
	int getWidth () const;
	int getHeight () const;
	bool isActive () const;
	bool isReconnecting () const;
	double getCongestion () const;
	QString getCongestionString () const;
	int getTotalFrames () const;
	int getDroppedFrames () const;
	double getFramesPerSecond () const;
	QString getFramesPerSecondString () const;
	int getTotalBytes () const;
	QString getTimeString () const;
	
	virtual void debug ();
	
	constexpr static const char* kNoTimeString = "00:00:00";
	
protected:
	OBSOutput output;
};
