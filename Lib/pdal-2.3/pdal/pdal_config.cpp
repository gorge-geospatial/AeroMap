/******************************************************************************
 * Copyright (c) 2008, Mateusz Loskot
 * Copyright (c) 2010, Frank Warmerdam
 * Copyright (c) 2018, Hobu Inc.
 ****************************************************************************/

#include <pdal/pdal_config.h>
#include <pdal/pdal_features.h>

#include <sstream>
#include <iomanip>

#include <pdal/gitsha.h>

#include <gdal.h>

#ifdef PDAL_HAVE_LIBXML2
#include <libxml/xmlversion.h>
#endif

#include <pdal/util/Utils.h>

namespace pdal
{
	namespace Config
	{
		bool hasFeature(Feature f)
		{
			bool enabled = false;
			switch (f)
			{
			case Feature::LASZIP:
#ifdef PDAL_HAVE_LASZIP
				enabled = true;
#endif
				break;
			case Feature::LAZPERF:
#ifdef PDAL_HAVE_LAZPERF
				enabled = true;
#endif
				break;
			case Feature::ZSTD:
#ifdef PDAL_HAVE_ZSTD
				enabled = true;
#endif
				break;
			case Feature::ZLIB:
#ifdef PDAL_HAVE_ZLIB
				enabled = true;
#endif
				break;
			case Feature::LZMA:
#ifdef PDAL_HAVE_LZMA
				enabled = true;
#endif
				break;
			case Feature::LIBXML2:
#ifdef PDAL_HAVE_LIBXML2
				enabled = true;
#endif
				break;
			default:
				break;
			}
			return enabled;
		}


		int versionMajor()
		{
			return PDAL_VERSION_MAJOR;
		}


		int versionMinor()
		{
			return PDAL_VERSION_MINOR;
		}


		int versionPatch()
		{
			return PDAL_VERSION_PATCH;
		}


		std::string versionString()
		{
			return std::string(PDAL_VERSION);
		}

		int versionInteger()
		{
			return PDAL_VERSION_INTEGER;
		}

		std::string sha1()
		{
			return g_GIT_SHA1;
		}


		/// Tell the user a bit about PDAL's compilation
		std::string fullVersionString()
		{
			std::ostringstream os;

			std::string sha = sha1();
			if (!Utils::iequals(sha, "Release"))
				sha = sha.substr(0, 6);

			os << PDAL_VERSION << " (git-version: " << sha << ")";

			return os.str();
		}


		std::string debugInformation()
		{
			Utils::screenWidth();
			std::string headline(Utils::screenWidth(), '-');

			std::ostringstream os;

			os << headline << std::endl;
			os << "PDAL debug information" << std::endl;
			os << headline << std::endl << std::endl;

			os << "Version information" << std::endl;
			os << headline << std::endl;
			os << "(" << fullVersionString() << ")" << std::endl;
			os << std::endl;

			os << "Debug build status" << std::endl;
			os << headline << std::endl;
			os << PDAL_BUILD_TYPE << std::endl << std::endl;

			os << "Enabled libraries" << std::endl;
			os << headline << std::endl << std::endl;

			os << "GDAL (" << GDALVersionInfo("RELEASE_NAME") << ") - " <<
				"http://www.gdal.org" << std::endl;

#ifdef PDAL_HAVE_LIBXML2
			os << "libxml (" << LIBXML_DOTTED_VERSION << ") - " <<
				"http://www.xmlsoft.org/" << std::endl;
#endif

			return os.str();
		}

		std::string pluginInstallPath()
		{
			return "";
			//return PDAL_PLUGIN_INSTALL_PATH;
		}
	}
}
