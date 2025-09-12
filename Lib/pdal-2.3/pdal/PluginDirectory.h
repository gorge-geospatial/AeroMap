#ifndef PLUGINDIRECTORY_H
#define PLUGINDIRECTORY_H

#include <pdal/Log.h>
#include <pdal/Stage.h>
#include <pdal/Kernel.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace pdal
{
	class PluginDirectory
	{
		FRIEND_TEST(PluginManagerTest, SearchPaths);
		FRIEND_TEST(PluginManagerTest, validnames);

	private:
		PluginDirectory();

	public:
		// This is actually thread-safe in C++11. How nice :)
		static PluginDirectory& get()
		{
			static PluginDirectory instance;

			return instance;
		}

		std::map<std::string, std::string> m_kernels;
		std::map<std::string, std::string> m_drivers;

	private:
		static PluginDirectory* m_instance;
		static std::string test_validPlugin(const std::string& path,
			const StringList& types);
	};
}

#endif // #ifndef PLUGINDIRECTORY_H
