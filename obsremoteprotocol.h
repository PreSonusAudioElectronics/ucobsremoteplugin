//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : protocol.h
// Created by  : James Inkster, jinkster@presonus.com
// Description : OBS Protocol Message Definitions
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

//////////////////////////////////////////////////////////////////////////////////////////////////
// OBS Protocol Message Definitions - sent in JSON format
//////////////////////////////////////////////////////////////////////////////////////////////////

/*
 An example of a JSON transmission with multiple value items being 'set':
 {
	"type":"set", ///< Could be 'set' or 'get'
	"values":
	 [
	   {
		  "name":"cpuUsage",
		  "value":"0.793976"
	   },
	   {
		  "name":"streaming",
		  "value":false
	   },
	   {
		  "name":"sceneList",
		  "value":
		  [
			 {
				"current":false,
				"name":"My Scene",
				"sceneSources":
				[
					{
						"locked":false,
						"name":"Display Capture",
						"visible":true
					},
					{
						"locked":false,
						"name":"Video Capture Device",
						"visible":true
					}
				]
			 },
			 {
				"current":true,
				"name":"My Other Scene",
				"sceneSources":[]
			 }
		  ]
	   }
 }
*/

namespace OBSRemoteProtocol
{	
	static const int kPortDefault =	2021; ///< The port the OBS Remote Plugin will listen on, unless otherwise specified in /blah/obs-remote/config.json
	static const int kPortMax = 65353;
	constexpr static const char* kPortId = "port";

	static const int kNumHeaderBytes = 4; ///< Each json message on the socket is prepended with the # of bytes proceeding
	static const int kKeepAliveMs = 5000;  ///< the server expects to receive a message of some kind

	/// An array of Value Items is passed back and forth. 
	/// Each Value Item has a type (get/set), name, and the actual value.
	constexpr static const char* kValuesArray = "values"; ///< an array of values
		constexpr static const char* kValueItemType = "type"; ///< type of the item (get/set)
			constexpr static const char* kValueItemTypeGet = "get";
			constexpr static const char* kValueItemTypeSet = "set";
		constexpr static const char* kValueItemName = "name"; ///< name of the item
		constexpr static const char* kValueItemValue = "value";  ///< value of the item

		/// The supported Value Item names (kValueItemName) are:
		constexpr static const char* kItemCPU = "cpuUsage"; ///< kValueItemValue: String (Get)
		constexpr static const char* kItemMemory = "memoryUsage"; ///< kValueItemValue: String (Get)
		constexpr static const char* kItemDisk = "freeDisk"; ///< kValueItemValue: String (Get)
		constexpr static const char* kItemRecordingTime = "recordingTime"; ///< kValueItemValue: String (Get)
		constexpr static const char* kItemStreamingTime = "streamingTime"; ///< kValueItemValue: String (Get)
		constexpr static const char* kItemTotalFrames = "totalFrames"; ///< kValueItemValue: Int (Get)
		constexpr static const char* kItemDroppedFrames = "droppedFrames"; ///< kValueItemValue: Int (Get)
		constexpr static const char* kItemFramesPerSecond = "fps"; ///< kValueItemValue: Int (Get)
		constexpr static const char* kItemCongestion = "congestion"; ///< kValueItemValue: String (Get)
		constexpr static const char* kItemStudioMode = "studioMode"; ///< kValueItemValue: Bool (Get/Set)
		constexpr static const char* kItemTriggerTransition = "triggerTransition"; ///< kValueItemBool: Bool (Set -- triggers a transition, only when in studio mode)
		constexpr static const char* kItemStreaming = "streaming"; ///< kValueItemValue: Bool (Get/Set)
		constexpr static const char* kItemRecording = "recording"; ///< kValueItemValue: Bool (Get/Set)
		constexpr static const char* kItemSceneList = "sceneList"; ///< kValueItemValue: (Get: An array of Scenes) (Set: the name of the desired current/preview scene, depending on Studio Mode)
		constexpr static const char* kItemCurrentScene = "currentScene"; ///< kValueItemValue: String name of the currently active scene (Get/Set)
		constexpr static const char* kItemPreviewScene = "previewScene"; ///< kValueItemValue: String name of the current preview scene (Get/Set)
		constexpr static const char* kItemSceneSourcesLocks = "sourceLocks"; ///< kValueItemValue: Integer 32-bit mask of lock states for the current scene's sources, in-order (Get/Set)
		constexpr static const char* kItemSceneSourcesVisibles = "sourceVisibles"; ///< kValueItemValue: Integer 32-bit mask of visible states for the current scene's sources, in-order (Get/Set)
		constexpr static const char* kItemTransitionList = "transitionsList"; ///< kValueItemValue: An array of Transitions (Get)
		constexpr static const char* kItemTransitionCurrent = "currentTransition"; ///< kValueItemValue: String (name of transition) (Get/Set)
		constexpr static const char* kItemTransitionCurrentDuration = "transitionDuration"; ///< kValueItemValue: Integer (duration of current transition) (Get/Set)

		constexpr static const char* kValueItemNames[] = 
		{
			kItemCPU,
			kItemMemory,
			kItemDisk,
			kItemRecordingTime,
			kItemStreamingTime,
			kItemTotalFrames,
			kItemDroppedFrames,
			kItemCongestion,
			
			kItemStudioMode,
			kItemTriggerTransition,
			kItemStreaming,
			kItemRecording,
			
			kItemSceneList,
			kItemCurrentScene,
			kItemPreviewScene,
			kItemSceneSourcesLocks,
			kItemSceneSourcesVisibles,
			kItemTransitionList,
			kItemTransitionCurrent,
			kItemTransitionCurrentDuration,
		};

		/// Sources:
		constexpr static const char* kSourceName = "name"; ///< String
		constexpr static const char* kSourceSortIndex = "sortIndex"; ///< Int (0-based index of the source, bypassing JSON sorting)
		constexpr static const char* kSourceIsCurrent = "isCurrent"; ///< Bool

		/// Scenes: (Inherits 'Source')
		constexpr static const char* kSceneSourcesList = "sourceList"; ///< an array of Scene Sources
		
		/// Scene Source: (Inherits 'Source')
		constexpr static const char* kSceneSourceVisible = "isVisible"; ///< Bool
		constexpr static const char* kSceneSourceLocked = "isLocked"; ///< Bool

		/// Transition: (Inherits 'Source')
		constexpr static const char* kTransitionDuration = "duration"; ///< Integer
};
