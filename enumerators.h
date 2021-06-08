//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : enumerators.h
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

#pragma once

#include "obsobjects.h"
	
//************************************************************************************************
// Enumerators
//************************************************************************************************

namespace Enumerators
{
	void enumerateScenes (QVector<Scene*>& scenes);
	void destroy (QVector<Scene*>& scenes);
	
	///< Convenience auto-release enumerator
	class Scenes
	{
	public:
		Scenes (QVector<Scene*>& scenes);
		~Scenes ();
	protected:
		QVector<Scene*>& scenes;
	};

	void enumerateSceneSources (QVector<SceneSource*>& sceneSources, Scene& scene);
	void destroy (QVector<SceneSource*>& sceneSources);
	
	///< Convenience auto-release enumerator
	class SceneSources
	{
	public:
		SceneSources (QVector<SceneSource*>& sceneSources, Scene& scene);
		~SceneSources ();
	protected:
		QVector<SceneSource*>& sceneSources;
		Scene& scene;
	};

	void enumerateSources (QVector<Source*>& sources);
	void destroy (QVector<Source*>& sources);

	///< Convenience auto-release enumerator
	class Sources
	{
	public:
		Sources (QVector<Source*>& sources);
		~Sources ();
	protected:
		QVector<Source*>& sources;
	};

	void enumerateOutputs (QVector<Output*>& outputs);
	void destroy (QVector<Output*>& outputs);

	///< Convenience auto-release enumerator
	class Outputs
	{
	public:
		Outputs (QVector<Output*>& outputs);
		~Outputs ();
	protected:
		QVector<Output*>& outputs;
	};

	void enumerateTransitions (QVector<Transition*>& transitions);
	void destroy (QVector<Transition*>& transitions);

	///< Convenience auto-release enumerator
	class Transitions
	{
	public:
		Transitions (QVector<Transition*>& transitions);
		~Transitions ();
	protected:
		QVector<Transition*>& transitions;
	};
} // Enumerators
