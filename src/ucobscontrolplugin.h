//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : ucobscontrolplugin.h
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

#pragma once

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QFuture>

#include <util/platform.h>
#include <util/base.h>

#include "networkserver.h"
#include "protocoladapter.h"

//************************************************************************************************
// UCOBSControlPlugin
//************************************************************************************************

class UCOBSControlPlugin: public QObject
{
	Q_OBJECT
public:
	static UCOBSControlPlugin& instance ();
	static void destroy ();
	
	void shutdown ();
		
private:
	UCOBSControlPlugin ();
	~UCOBSControlPlugin ();
	
	NetworkServer server;
	ProtocolAdapter adapter;
	
	static UCOBSControlPlugin* theInstance;
};
