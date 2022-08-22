//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : obsobjects.cpp
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

#include "obsobjects.h"

#define ENABLE_LOGGING 0
#include "common.h"
#include "enumerators.h"
#include "obsremoteprotocol.h"
#include <obs-audio-controls.h>

#include "moc_obsobjects.cpp"

//************************************************************************************************
// Source
//************************************************************************************************

Source::Source (obs_source_t& _source)
: source (&_source)
{
	//LOG ("SOURCE + %s", STR (getName ()))
	
	if(signal_handler_t* handler = obs_source_get_signal_handler (source))
	{
		signal_handler_connect (handler, "destroy", onDestroyed, this);
		signal_handler_connect (handler, "remove", onRemoved, this);
		signal_handler_connect (handler, "activate", onActivated, this);
		signal_handler_connect (handler, "deactivate", onDeactivated, this);
		signal_handler_connect (handler, "show", onShown, this);
		signal_handler_connect (handler, "hide", onHidden, this);
		signal_handler_connect (handler, "enable", onEnabled, this);
		signal_handler_connect (handler, "rename", onRenamed, this);
	
		if(obs_source_get_type (source) == OBS_SOURCE_TYPE_SCENE)
		{
			//LOG ("Connecting SCENE signals:")
			// we have to connect the scene item signals here, in the base-class:
			signal_handler_connect (handler, "item_add", onSceneItemAdded, this);
			signal_handler_connect (handler, "item_remove", onSceneItemRemoved, this);
			signal_handler_connect (handler, "reorder", onSceneItemReordered, this);
			signal_handler_connect (handler, "refresh", onSceneItemRefreshed, this);
			signal_handler_connect (handler, "item_visible", onSceneItemVisibilityChanged, this);
			signal_handler_connect (handler, "item_locked", onSceneItemLockChanged, this);
		}
	}
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////

Source::~Source ()
{	
	//LOG ("SOURCE - %s", STR (getName ()))
	if(signal_handler_t* handler = obs_source_get_signal_handler (source))
	{
		signal_handler_disconnect (handler, "destroy", onDestroyed, this);
		signal_handler_disconnect (handler, "remove", onRemoved, this);
		signal_handler_disconnect (handler, "activate", onActivated, this);
		signal_handler_disconnect (handler, "deactivate", onDeactivated, this);
		signal_handler_disconnect (handler, "show", onShown, this);
		signal_handler_disconnect (handler, "hide", onHidden, this);
		signal_handler_disconnect (handler, "enable", onEnabled, this);
		signal_handler_disconnect (handler, "rename", onRenamed, this);
		
		if(obs_source_get_type (source) == OBS_SOURCE_TYPE_SCENE)
		{
			signal_handler_disconnect (handler, "item_add", onSceneItemAdded, this);
			signal_handler_disconnect (handler, "item_remove", onSceneItemRemoved, this);
			signal_handler_disconnect (handler, "reorder", onSceneItemReordered, this);
			signal_handler_disconnect (handler, "refresh", onSceneItemRefreshed, this);
			signal_handler_disconnect (handler, "item_visible", onSceneItemVisibilityChanged, this);
			signal_handler_disconnect (handler, "item_locked", onSceneItemLockChanged, this);
		}
	}
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onDestroyed (void* param, calldata_t* data)
{
	//LOG ("Source::onDestroyed")
	Source* source = reinterpret_cast<Source*> (param);
	obs_source_t* obsSource = (obs_source_t*)calldata_ptr (data, "source");
	if(source->getInternal () != obsSource)
		return;
	
	source->emit destroyed (*source);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onRemoved (void* param, calldata_t* data)
{
	//LOG ("Source::onRemoved")
	Source* source = reinterpret_cast<Source*> (param);
	obs_source_t* obsSource = (obs_source_t*)calldata_ptr (data, "source");
	if(source->getInternal () != obsSource)
		return;
	
	source->emit removed (*source);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onActivated (void* param, calldata_t* data)
{
	//LOG ("Source::onActivated")
	Source* source = reinterpret_cast<Source*> (param);
	obs_source_t* obsSource = (obs_source_t*)calldata_ptr (data, "source");
	if(source->getInternal () != obsSource)
		return;
	
	source->emit activated (*source, true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onDeactivated (void* param, calldata_t* data)
{
	//LOG ("Source::onDeactivated")
	Source* source = reinterpret_cast<Source*> (param);
	obs_source_t* obsSource = (obs_source_t*)calldata_ptr (data, "source");
	if(source->getInternal () != obsSource)
		return;
	
	source->emit activated (*source, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onShown (void* param, calldata_t* data)
{
	//LOG ("Source::onShown")
	Source* source = reinterpret_cast<Source*> (param);
	obs_source_t* obsSource = (obs_source_t*)calldata_ptr (data, "source");
	if(source->getInternal () != obsSource)
		return;
	
	source->emit shown (*source, true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onHidden (void* param, calldata_t* data)
{
	//LOG ("Source::onHidden")
	Source* source = reinterpret_cast<Source*> (param);
	obs_source_t* obsSource = (obs_source_t*)calldata_ptr (data, "source");
	if(source->getInternal () != obsSource)
		return;
	
	source->emit shown (*source, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onEnabled (void* param, calldata_t* data)
{
	//LOG ("Source::onEnabled")
	Source* source = reinterpret_cast<Source*> (param);
	obs_source_t* obsSource = (obs_source_t*)calldata_ptr (data, "source");
	if(source->getInternal () != obsSource)
		return;
	
	bool enabled = false;
	if(!calldata_get_bool (data, "enabled", &enabled))
		return;
	source->emit enabled (*source, enabled);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onRenamed (void* param, calldata_t* data)
{
	//LOG ("Source::onRenamed")
	Source* source = reinterpret_cast<Source*> (param);
	obs_source_t* obsSource = (obs_source_t*)calldata_ptr (data, "source");
	if(source->getInternal () != obsSource)
		return;
	source->emit renamed (*source);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onSceneItemAdded (void* param, calldata_t* data)
{
	//LOG ("Source::onSceneItemAdded")
	Scene* scene = reinterpret_cast<Scene*> (param);
	if(scene)
		scene->handleSceneItemAdded (data);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onSceneItemRemoved (void* param, calldata_t* data)
{
	//LOG ("Source::onSceneItemRemoved")
	Scene* scene = reinterpret_cast<Scene*> (param);
	if(scene)
		scene->handleSceneItemRemoved (data);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onSceneItemReordered (void* param, calldata_t* data)
{
	LOG ("Source::onSceneItemReordered")
	Scene* scene = reinterpret_cast<Scene*> (param);
	if(scene)
		scene->handleSceneItemsReordered (data);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onSceneItemRefreshed (void* param, calldata_t* data)
{
	//LOG ("Source::onSceneItemRefreshed")
	Scene* scene = reinterpret_cast<Scene*> (param);
	if(scene)
		scene->handleSceneItemsRefreshed (data);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onSceneItemVisibilityChanged (void* param, calldata_t* data)
{
	//LOG ("Source::onSceneItemVisibilityChanged")
	Scene* scene = reinterpret_cast<Scene*> (param);
	if(scene)
		scene->handleSceneItemVisibilityChanged (data);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::onSceneItemLockChanged (void* param, calldata_t* data)
{
	//LOG ("Source::onSceneItemLockChanged")
	Scene* scene = reinterpret_cast<Scene*> (param);
	if(scene)
		scene->handleSceneItemLockChanged (data);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Scene::getTypeString () const
{
	return "Scene";
}

//////////////////////////////////////////////////////////////////////////////////////////////////

obs_scene_t* Scene::getInternalScene () const
{
	return obs_scene_from_source (getInternal ()); // does not increase reference
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Source::getName () const
{
	if(const char* sourceName = obs_source_get_name (source))
		return QString (sourceName);
	return "Error";
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Source::getId () const
{
	if(const char* sourceId = obs_source_get_id (source))
		return QString (sourceId);
	return "Error";
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonObject Source::toJson () const
{
	QJsonObject json;
	json[OBSRemoteProtocol::kSourceName] = getName ();
	return json;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Source::debug ()
{
	LOG ("Source:'%s'\t%s", STR (getName ()), STR (getId ()))
}

//************************************************************************************************
// Scene
//************************************************************************************************

Scene::Scene (obs_source_t& _source)
: Source (_source)
{
	regenerateSources ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Scene::~Scene ()
{
	Enumerators::destroy (sceneSources);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::regenerateSources ()
{
	Enumerators::destroy (sceneSources);
	Enumerators::enumerateSceneSources (sceneSources, *this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SceneSource* Scene::findSceneSource (obs_sceneitem_t& obsSceneItem) const
{
	for(auto i : getSources ())
	{
		if(i->getInternalSceneItem () == &obsSceneItem)
			return i;
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::handleSceneItemAdded (calldata_t* data)
{
	obs_scene_t* obsScene = (obs_scene_t*)calldata_ptr (data, "scene");
	obs_sceneitem_t* obsSceneItem = (obs_sceneitem_t*)calldata_ptr (data, "item");
	if(!obsScene || !obsSceneItem)
		return;
	
	if(getInternalScene () != obsScene)
		return;
	
	regenerateSources ();
	if(SceneSource* sceneSource = findSceneSource (*obsSceneItem))
		emit sceneSourceAdded (*this, *sceneSource);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::handleSceneItemRemoved (calldata_t* data)
{
	obs_scene_t* obsScene = (obs_scene_t*)calldata_ptr (data, "scene");
	obs_sceneitem_t* obsSceneItem = (obs_sceneitem_t*)calldata_ptr (data, "item");
	if(!obsScene || !obsSceneItem)
		return;
	
	if(getInternalScene () != obsScene)
		return;
	
	if(SceneSource* sceneSource = findSceneSource (*obsSceneItem))
	{	
		emit sceneSourceRemoved (*this, *sceneSource);
		regenerateSources ();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::handleSceneItemsReordered (calldata_t* data)
{
	obs_scene_t* obsScene = (obs_scene_t*)calldata_ptr (data, "scene");
	if(getInternalScene () != obsScene)
		return;
	regenerateSources ();
	LOG ("Scene::handleSceneItemsReordered")
	emit sceneSourcesReordered (*this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::handleSceneItemsRefreshed (calldata_t* data)
{
	obs_scene_t* obsScene = (obs_scene_t*)calldata_ptr (data, "scene");
	if(getInternalScene () != obsScene)
		return;
	
	emit sceneSourcesRefreshed (*this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::handleSceneItemVisibilityChanged (calldata_t* data)
{
	obs_scene_t* obsScene = (obs_scene_t*)calldata_ptr (data, "scene");
	obs_sceneitem_t* obsSceneItem = (obs_sceneitem_t*)calldata_ptr (data, "item");
	if(!obsScene || !obsSceneItem)
		return;
	
	if(getInternalScene () != obsScene)
		return;
	
	if(SceneSource* sceneSource = findSceneSource (*obsSceneItem))
	{
		bool visible = false;
		if(!calldata_get_bool (data, "visible", &visible))
			return;
		emit sceneSourceVisibilityChanged (*this, *sceneSource, visible);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Scene::handleSceneItemLockChanged (calldata_t* data)
{
	obs_scene_t* obsScene = (obs_scene_t*)calldata_ptr (data, "scene");
	obs_sceneitem_t* obsSceneItem = (obs_sceneitem_t*)calldata_ptr (data, "item");
	if(!obsScene || !obsSceneItem)
		return;
	
	if(getInternalScene () != obsScene)
		return;
	
	if(SceneSource* sceneSource = findSceneSource (*obsSceneItem))
	{
		bool locked = false;
		if(!calldata_get_bool (data, "locked", &locked))
			return;
		emit sceneSourceLockChanged (*this, *sceneSource, locked);
	}
}

//************************************************************************************************
// SceneSource
//************************************************************************************************

SceneSource::SceneSource (obs_sceneitem_t& _item)
: Source (*obs_sceneitem_get_source (&_item)), // obs_sceneitem_get_source doesn't add a ref-count
  item (&_item) 
{
	//LOG ("SceneSource +")
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SceneSource::~SceneSource ()
{
	//LOG ("SceneSource -")
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString SceneSource::getTypeString () const
{
	return "Scene Item";	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QJsonObject SceneSource::toJson () const
{
	QJsonObject json = Source::toJson ();
	json[OBSRemoteProtocol::kSceneSourceVisible] = isVisible ();
	json[OBSRemoteProtocol::kSceneSourceLocked] = isLocked ();
	return json;
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneSource::isVisible () const
{
	if(item)
		return obs_sceneitem_visible (item);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SceneSource::setVisible (bool state)
{
	if(item)
		obs_sceneitem_set_visible (item, state);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneSource::isLocked () const
{
	if(item)
		return obs_sceneitem_locked (item);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SceneSource::setLocked (bool state)
{
	if(item)
		obs_sceneitem_set_locked (item, state);
}

//************************************************************************************************
// AudioMeter
//************************************************************************************************

AudioMeter::AudioMeter (obs_source_t& source)
: meter (obs_volmeter_create (OBS_FADER_CUBIC))
{
	//LOG ("AudioMeter +");
	obs_volmeter_attach_source (meter, &source);
	obs_volmeter_add_callback (meter, handleLevel, this); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////

AudioMeter::~AudioMeter ()
{
	//LOG ("AudioMeter -");
	obs_volmeter_detach_source (meter);
	obs_volmeter_remove_callback (meter, handleLevel, this);
	obs_volmeter_destroy (meter);	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AudioMeter::handleLevel (void* meter, const float magnitude[MAX_AUDIO_CHANNELS], const float peak[MAX_AUDIO_CHANNELS], const float inputPeak[MAX_AUDIO_CHANNELS])
{
	reinterpret_cast<AudioMeter*> (meter)->updateLevel (magnitude, peak, inputPeak);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AudioMeter::updateLevel (const float magnitude[MAX_AUDIO_CHANNELS], const float peak[MAX_AUDIO_CHANNELS], const float inputPeak[MAX_AUDIO_CHANNELS])
{
	Q_UNUSED (magnitude)
	Q_UNUSED (peak)
	Q_UNUSED (inputPeak)
	for(int i = 0; i < MAX_AUDIO_CHANNELS; ++i)
	{
		LOG ("AudioMeter::updateLevel [%d] Volume: %.2f, %.2f, %.2f\n", i, magnitude[i], peak[i], inputPeak[i]);
	}
}

//************************************************************************************************
// Input
//************************************************************************************************

Input::Input (obs_source_t& source)
: Source (source)
{
	//LOG ("Input +")
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Input::getTypeString () const
{
	return QString ("Input");
}

//************************************************************************************************
// Filter
//************************************************************************************************

Filter::Filter (obs_source_t& source)
: Source (source)
{
	//LOG ("Filter +")
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////

QString Filter::getTypeString () const
{
	return QString ("Filter");
}

//************************************************************************************************
// Transition
//************************************************************************************************

Transition::Transition (obs_source_t& source)
: Source (source)
{
	//LOG ("Transition +")
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////

QString Transition::getTypeString () const
{
	return QString ("Transition");
}

//************************************************************************************************
// Output
//************************************************************************************************

Output::Output (obs_output_t& output)
: output (&output)
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

Output::~Output ()
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

obs_output_t* Output::getInternal () const
{
	return output;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Output::updateInternal (obs_output_t& _output)
{
	if(output == &_output)
		return;
	output = &_output;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Output::debug ()
{
	LOG ("Output:'%s'\t%s\t%dx%d\tactive? %d\n", STR (getName ()), STR (getId ()), getWidth (), getHeight (), isActive ())
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Output::getName () const
{
	if(const char* name = obs_output_get_name (output))
		return QString (name);
	return "Error";
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Output::getId () const
{
	if(const char* id = obs_output_get_id (output))
		return QString (id);
	return "Error";
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int Output::getWidth () const
{
	return obs_output_get_width (output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int Output::getHeight () const
{
	return obs_output_get_height (output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool Output::isActive () const
{
	return obs_output_active (output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool Output::isReconnecting () const
{
	return obs_output_reconnecting (output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

double Output::getCongestion () const
{
	return obs_output_get_congestion (output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Output::getCongestionString () const
{
	return QString::asprintf ("%.02f%%", getCongestion () * 100.f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int Output::getTotalFrames () const
{
	return obs_output_get_total_frames (output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int Output::getDroppedFrames () const
{
	return obs_output_get_frames_dropped (output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

double Output::getFramesPerSecond () const
{
	obs_video_info videoInfo;
	obs_get_video_info (&videoInfo);
	if(videoInfo.fps_den > 0.)
		return static_cast<double> (videoInfo.fps_num) / static_cast<double> (videoInfo.fps_den);
	return 0.;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Output::getFramesPerSecondString () const
{
	double fps = getFramesPerSecond ();
	QString string = QString::asprintf ("%.02f fps", fps);
	//LOG ("getFramesPerSecondString %.02f : '%s'", fps, STR (string))
	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int Output::getTotalBytes () const
{
	return obs_output_get_total_bytes (output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Output::getTimeString () const
{
	if(!obs_output_active (output))
		return kNoTimeString;
	
	video_t* videoOutput = obs_output_video (output);
	if(!videoOutput)
		return kNoTimeString;
	
	uint64_t frameTimeNanos = video_output_get_frame_time (videoOutput);
	int totalFrames = obs_output_get_total_frames (output);
	uint64_t totalNanos  = frameTimeNanos * static_cast<uint64_t> (totalFrames);
	uint64_t totalRecordSeconds = totalNanos / 1000000000;
	
	int seconds = totalRecordSeconds % 60;
	int totalMinutes = totalRecordSeconds / 60;
	int minutes = totalMinutes % 60;
	int hours = totalMinutes / 60;

	return QString::asprintf ("%02d:%02d:%02d", hours, minutes, seconds);
}
