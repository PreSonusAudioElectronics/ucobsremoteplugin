//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : common.h
// Created by  : James Inkster, jinkster@presonus.com
// Description : Commonly used plugin-wide macros, etc
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

#include <util/platform.h>
#include <util/base.h>
#include <obs-module.h>
#include <obs.hpp>

#define PLUGIN_NAME "ucobscontrolplugin"
#define PLUGIN_VERSION "1.1.1"

#if ENABLE_LOGGING
#define LOG(msg, ...) blog(LOG_INFO, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__);
#else
#define LOG(msg, ...) 
#endif

#define STR(qstring) qPrintable (qstring)

/** Count number of items in array. */
#define ARRAY_COUNT(a) int(sizeof(a)/sizeof(a[0]))

/** Reference-counting helpers. */
void dontAddSourceRef (obs_source_t*);
using AutoReleaseSource = OBSRef<obs_source_t*, dontAddSourceRef, obs_source_release>;

void dontAddOutputRef (obs_output_t*);
using AutoReleaseOutput = OBSRef<obs_output_t*, dontAddOutputRef, obs_output_release>;
