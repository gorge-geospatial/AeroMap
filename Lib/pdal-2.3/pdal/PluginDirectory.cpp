/******************************************************************************
* Copyright (c) 2018, Hobu Inc. (info@hobu.co)
****************************************************************************/

#include <pdal/PluginDirectory.h>
#include <pdal/util/Algorithm.h>
#include <pdal/util/FileUtils.h>
#include <pdal/pdal_config.h>

namespace pdal
{
	namespace
	{
		StringList pluginSearchPaths()
		{
			StringList searchPaths;
			std::string envOverride;

			Utils::getenv("PDAL_DRIVER_PATH", envOverride);

			if (!envOverride.empty())
				searchPaths = Utils::split2(envOverride, Utils::pathListSeparator);
			else
			{
				StringList possiblePaths
				{ 
					".", 
					"./lib", 
					"../lib", 
					"./bin", 
					"../bin"
					//Utils::dllDir(),
					//Config::pluginInstallPath()
				};

				for (std::string s : possiblePaths)
				{
					s = FileUtils::toCanonicalPath(s);
					if (s.size() && !Utils::contains(searchPaths, s))
						searchPaths.push_back(s);
				}
			}
			return searchPaths;
		}

		// libpdal_plugin_{stagetype}_{name}.{extension}
		// For example, libpdal_plugin_writer_text.so or
		// libpdal_plugin_filter_color.dylib
		std::string validPlugin(const std::string& path, const StringList& types)
		{
			auto typeValid = [&types](std::string& type)
			{
				for (auto t : types)
					if (type == t)
						return true;
				return false;
			};

			// Strip off the leader.
			std::string file = FileUtils::getFilename(path);
			const std::string leader("libpdal_plugin_");
			auto pos = file.find(leader);
			if (pos != 0)
				return std::string();
			file = file.substr(leader.size());

			// Strip off the type.
			std::string type;
			pos = file.find('_');
			if (pos != std::string::npos && pos < file.size() - 1)
				type = file.substr(0, pos);
			if (!typeValid(type))
				return std::string();
			file = file.substr(pos + 1);

			// Strip the extension off of the end.
			pos = file.rfind('.');
			if (pos == std::string::npos ||
				(file.substr(pos) != Utils::dynamicLibExtension))
				return std::string();
			file = file.substr(0, pos);

			// Make sure what's left is valid.  NOTE - pos is modified by parseName().
			pos = 0;
			if (!Stage::parseName(file, pos) || (pos != file.size()))
				return std::string();

			// Coerce the filename to the stage name.
			return type + "s." + file;
		}

	} // unnamed namespace;


	PluginDirectory::PluginDirectory()
	{
		for (const auto& dir : pluginSearchPaths())
		{
			StringList files = FileUtils::directoryList(dir);
			for (auto& file : files)
			{
				file = FileUtils::toAbsolutePath(file);

				std::string plugin;
				plugin = validPlugin(file, { "kernel" });
				if (plugin.size())
				{
					m_kernels.insert(std::make_pair(plugin, file));
					continue;
				}
				plugin = validPlugin(file, { "reader", "writer", "filter" });
				if (plugin.size())
					m_drivers.insert(std::make_pair(plugin, file));
			}
		}
	}

	std::string PluginDirectory::test_validPlugin(const std::string& path,
		const StringList& types)
	{
		return validPlugin(path, types);
	}
}
