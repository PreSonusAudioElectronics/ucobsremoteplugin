//************************************************************************************************
//
// UCOBSControlPlugin
// Copyright (c)2021 PreSonus Audio Electronics, Inc
//
// Filename    : statistics.cpp
// Created by  : James Inkster, jinkster@presonus.com
// Description : Statistics helper object
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

#include "statistics.h"

#define ENABLE_LOGGING 0
#include "common.h"

#include <util/config-file.h>

//************************************************************************************************
// Statistics
//************************************************************************************************

Statistics::Statistics ()
: cpuUsageInfo (0)
{
	cpuUsageInfo = os_cpu_usage_info_start ();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Statistics::~Statistics ()
{
	os_cpu_usage_info_destroy (cpuUsageInfo);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Statistics::getCpuUsage () const
{
	double cpu = os_cpu_usage_info_query (cpuUsageInfo);
	QString string = QString::number (cpu, 'g', 2);
	string.append ("%");
	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Statistics::getMemoryUsage () const
{
	// calculated the same way the UI stats window does...
	long double num = static_cast<long double> (os_get_proc_resident_size ()) / (1024.0l * 1024.0l);
	QString string = QString::number (num, 'f', 1) + QStringLiteral (" MB");
	return string;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString Statistics::getFreeDisk () const
{
	// calculated the same way the UI stats window does...
	config_t* currentProfile = obs_frontend_get_profile_config ();
	if(!currentProfile)
		return "Error";
	
	QString outputMode = config_get_string (currentProfile, "Output", "Mode");
	QString path = (outputMode == "Advanced") ? 
						config_get_string (currentProfile, "SimpleOutput", "FilePath") :
						config_get_string (currentProfile, "AdvOut", "RecFilePath");
	
	static const long double kKilo = 1024ULL;
	static const long double kMByte = (kKilo * kKilo);
	static const long double kGByte = (kKilo * kKilo * kKilo);
	static const long double kTByte = (kKilo * kKilo * kKilo * kKilo);

	QString abbreviation = QStringLiteral (" MB");
	long double numBytes = os_get_free_disk_space (path.toUtf8 ());
	long double num = numBytes / (1024.01 * 1024.01);
	if(numBytes > kTByte)
	{
		num /= kKilo * kKilo;
		abbreviation = QStringLiteral (" TB");
	}
	else if(numBytes > kGByte)
	{
		num /= kKilo;
		abbreviation = QStringLiteral (" GB");
	}
	return QString::number (num, 'f', 1) + abbreviation;
}
