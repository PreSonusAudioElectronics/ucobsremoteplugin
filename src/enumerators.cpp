//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : enumerators.cpp
// Created by  : James Inkster, jinkster@presonus.com
// Description : Utilities to enumerate OBS sources, etc
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

#include "enumerators.h"

#define ENABLE_LOGGING 0
#include "common.h"

//************************************************************************************************
// Enumerators
//************************************************************************************************

namespace Enumerators 
{
	auto sourceEnumerator = [] (void* array, obs_source_t* obsSource)->bool
	{
		if(!obsSource)
			return false;
		QVector<Source*>* sourcesArray = reinterpret_cast<QVector<Source*>*> (array);
		if(!sourcesArray)
			return false;
		Source* source = nullptr;
		if(!obs_source_removed (obsSource)) // ensure the source hasn't already been removed
		{
			switch(obs_source_get_type (obsSource))
			{
			default:
				// skip it, unsupported type
				break;
			case OBS_SOURCE_TYPE_SCENE:
				source = new Scene (*obsSource);
				break;
			case OBS_SOURCE_TYPE_INPUT:
				source = new Input (*obsSource);
				break;
			case OBS_SOURCE_TYPE_FILTER:
				source = new Filter (*obsSource);
				break;
			case OBS_SOURCE_TYPE_TRANSITION:
				source = new Transition (*obsSource);
				break;
			}
		}
		
		if(source)
			sourcesArray->prepend (source);
		return true;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void enumerateSources (QVector<Source*>& sources)
	{
		obs_enum_sources (sourceEnumerator, &sources);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void destroy (QVector<Source*>& sources)
	{
		for(auto i : sources)
			delete i;
		sources.clear ();
	}

	//************************************************************************************************
	// Sources
	//************************************************************************************************

	Sources::Sources (QVector<Source*>& sources)
	: sources (sources)
	{
		enumerateSources (sources);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	Sources::~Sources ()
	{
		destroy (sources);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void enumerateScenes (QVector<Scene*>& scenes)
	{
		obs_frontend_source_list obsScenes = {};
		obs_frontend_get_scenes (&obsScenes);
		for(int i = 0; i < static_cast<int> (obsScenes.sources.num); i++) 
		{
			if(obs_source_t* source = obsScenes.sources.array[i])
			{
				Scene* scene = new Scene (*source);
				scenes.append (scene);
				//LOG ("enumerateScenes: %s", STR (scene->getName ()))
			}
		}
		obs_frontend_source_list_free (&obsScenes);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void destroy (QVector<Scene*>& scenes)
	{
		for(auto i : scenes)
			delete i;
		scenes.clear ();
	}

	//************************************************************************************************
	// Scenes
	//************************************************************************************************

	Scenes::Scenes (QVector<Scene*>& scenes)
	: scenes (scenes)
	{
		enumerateScenes (scenes);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	Scenes::~Scenes ()
	{
		destroy (scenes);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	auto sceneItemEnumerator = [] (obs_scene* obsScene, obs_sceneitem_t* obsSceneItem, void* array)->bool
	{
		if(!obsScene)
			return false;
		QVector<SceneSource*>* sceneSourcesArray = reinterpret_cast<QVector<SceneSource*>*> (array);
		if(!sceneSourcesArray)
			return false;
		SceneSource* sceneItem = new SceneSource (*obsSceneItem);
		sceneSourcesArray->prepend (sceneItem);
		
		return true;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void enumerateSceneSources (QVector<SceneSource*>& sceneSources, Scene& scene)
	{
		//LOG ("enumerateSceneSources +")
		obs_scene_enum_items (scene.getInternalScene (), sceneItemEnumerator, &sceneSources);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void destroy (QVector<SceneSource*>& sceneSources)
	{
		//LOG ("enumerateSceneSources -")
		for(auto i : sceneSources)
			delete i;
		sceneSources.clear ();
	}

	//************************************************************************************************
	// SceneSources
	//************************************************************************************************

	SceneSources::SceneSources (QVector<SceneSource*>& sceneSources, Scene& scene)
	: sceneSources (sceneSources),
	  scene (scene)
	{
		enumerateSceneSources (sceneSources, scene);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	SceneSources::~SceneSources ()
	{
		destroy (sceneSources);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	auto outputEnumerator = [] (void* array, obs_output_t* obsOutput)->bool
	{
		if(!obsOutput)
			return false;
		QVector<Output*>* outputsArray = reinterpret_cast<QVector<Output*>*> (array);
		if(!outputsArray)
			return false;
		Output* output = new Output (*obsOutput);
		outputsArray->prepend (output);
		return true;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void enumerateOutputs (QVector<Output*>& outputs)
	{
		//LOG ("enumerateOutputs +")
		obs_enum_outputs (outputEnumerator, &outputs);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void destroy (QVector<Output*>& outputs)
	{
		//LOG ("enumerateOutputs -")
		for(auto i : outputs)
			delete i;
		outputs.clear ();
	}

	//************************************************************************************************
	// Outputs
	//************************************************************************************************

	Outputs::Outputs (QVector<Output*>& outputs)
	: outputs (outputs)
	{
		enumerateOutputs (outputs);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	Outputs::~Outputs ()
	{
		destroy (outputs);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void enumerateTransitions (QVector<Transition*>& transitions)
	{
		obs_frontend_source_list obsTransitions = {};
		obs_frontend_get_transitions (&obsTransitions);
		for(int i = 0; i < static_cast<int> (obsTransitions.sources.num); i++) 
		{
			if(obs_source_t* source = obsTransitions.sources.array[i])
			{
				Transition* transition = new Transition (*source);
				transitions.append (transition);
				//LOG ("enumerateTransitions: %s", STR (transition->getName ()))
			}
		}
		obs_frontend_source_list_free (&obsTransitions);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void destroy (QVector<Transition*>& transitions)
	{
		for(auto i : transitions)
			delete i;
		transitions.clear ();
	}

	//************************************************************************************************
	// Transitions
	//************************************************************************************************

	Transitions::Transitions (QVector<Transition*>& transitions)
	: transitions (transitions)
	{
		enumerateTransitions (transitions);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	Transitions::~Transitions ()
	{
		destroy (transitions);
	}

} // Enumerators
