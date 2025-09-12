/******************************************************************************
 * Copyright (c) 2008, Mateusz Loskot
 * Copyright (c) 2010, Frank Warmerdam
 * Copyright (c) 2018, Hobu Inc.
 ****************************************************************************/

#pragma once

#include <pdal/pdal_internal.h>

#include <string>

namespace pdal
{
	namespace Config
	{
		enum class Feature
		{
			LASZIP,
			LAZPERF,
			ZSTD,
			ZLIB,
			LZMA,
			LIBXML2
		};

		bool hasFeature(Feature f);
		std::string fullVersionString();
		std::string versionString();
		int versionInteger();
		std::string sha1();
		int versionMajor();
		int versionMinor();
		int versionPatch();
		std::string debugInformation();
		std::string pluginInstallPath();
	}
}

