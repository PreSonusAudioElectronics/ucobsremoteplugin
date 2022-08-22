//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : ucobscontrolplugin.cpp
// Created by  : James Inkster, jinkster@presonus.com
// Description : Entry point for OBS Plugin
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

#include "ucobscontrolplugin.h"

#define ENABLE_LOGGING 1
#include "common.h"
#include "obsremoteprotocol.h"
#include "moc_ucobscontrolplugin.cpp"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardPaths>

OBS_DECLARE_MODULE()

//************************************************************************************************
// UCOBSControlPlugin
//************************************************************************************************

UCOBSControlPlugin* UCOBSControlPlugin::theInstance = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////

UCOBSControlPlugin& UCOBSControlPlugin::instance ()
{
	if(!theInstance)
		theInstance = new UCOBSControlPlugin;
	return *theInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void UCOBSControlPlugin::destroy ()
{
	delete theInstance;
	theInstance = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

UCOBSControlPlugin::UCOBSControlPlugin ()
: server (this),
  adapter (server)
{
	uint32_t obsVersion = obs_get_version ();
	const char* obsVersionString = obs_get_version_string ();
	LOG ("Welcome to UCOBSControlPlugin version %s! You're using OBS %d (%s)", PLUGIN_VERSION, obsVersion, obsVersionString);
	
	QString configPath = QStandardPaths::writableLocation (QStandardPaths::GenericDataLocation);
	configPath.append ("/presonus/OBS Remote/");
	QDir dir;
	if (!dir.exists (configPath))
		dir.mkpath (configPath);
	configPath.append ("config.json");
	
	LOG ("Reading config from: %s", STR (configPath))
	QFile configFile (configPath);
	configFile.open (QIODevice::ReadWrite | QIODevice::Text);
	QJsonDocument jsonDoc = QJsonDocument::fromJson (configFile.readAll ());
	QJsonObject values = jsonDoc.object ();
	QJsonValue portValue = values.value (OBSRemoteProtocol::kPortId);
	int port = portValue.toInt ();
	if(port <= 0 || port > OBSRemoteProtocol::kPortMax)
	{
		port = OBSRemoteProtocol::kPortDefault;
		values[OBSRemoteProtocol::kPortId] = port;
		jsonDoc.setObject (values);
		/*qint64 bytesWritten = */configFile.write (jsonDoc.toJson ());
		configFile.close ();
	}
	server.start (port);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

UCOBSControlPlugin::~UCOBSControlPlugin ()
{}

//////////////////////////////////////////////////////////////////////////////////////////////////

void UCOBSControlPlugin::shutdown ()
{
	server.stop ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool obs_module_load (void)
{
	// Setup event handler to start the server once OBS is ready
	auto eventCallback = [](enum obs_frontend_event event, void *param) 
	{
		switch(event)
		{
		case OBS_FRONTEND_EVENT_FINISHED_LOADING :
			{
				//LOG ("OBS_FRONTEND_EVENT_FINISHED_LOADING, creating plugin");				   
				UCOBSControlPlugin& plugin = UCOBSControlPlugin::instance (); // instantiate plugin
				Q_UNUSED (plugin)
			} break;
		case OBS_FRONTEND_EVENT_EXIT :
			{
				//LOG ("OBS_FRONTEND_EVENT_EXIT -- relying on module unload for destruction");
				UCOBSControlPlugin::instance ().shutdown ();
				obs_frontend_remove_event_callback ((obs_frontend_event_cb)param, nullptr);
			} break;
		default:
			break;
		}
	};
	obs_frontend_add_event_callback (eventCallback, (void*)(obs_frontend_event_cb)eventCallback);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void obs_module_unload ()
{
	UCOBSControlPlugin::destroy ();
}
