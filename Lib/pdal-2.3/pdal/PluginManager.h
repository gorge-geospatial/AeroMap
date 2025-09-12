/******************************************************************************
* Copyright (c) 2015, Bradley J Chambers (brad.chambers@gmail.com)
****************************************************************************/

// The PluginManager was modeled very closely after the work of Gigi Sayfan in
// the Dr. Dobbs article:
// http://www.drdobbs.com/cpp/building-your-own-plugin-framework-part/206503957
// The original work was released under the Apache License v2.

#pragma once

#include <pdal/Log.h>
#include <pdal/PluginInfo.h>
#include <pdal/StageExtensions.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <functional>


namespace pdal
{

	class DynamicLibrary;

	/*
	 * I think PluginManager can eventually be a private header, only accessible
	 * through the factories, but we'll leave it as public for now.
	 */

	template <typename T>
	class PluginManager
	{
		struct Info
		{
			std::string name;
			std::string link;
			std::string description;
			std::function<T* ()> create;
		};
		typedef std::shared_ptr<DynamicLibrary> DynLibPtr;
		typedef std::map<std::string, DynLibPtr> DynamicLibraryMap;
		typedef std::map<std::string, Info> RegistrationInfoMap;

	public:
		PluginManager(const PluginManager&) = delete;
		PluginManager& operator=(const PluginManager&) = delete;
		~PluginManager();

		static std::string description(const std::string& name);
		static std::string link(const std::string& name);
		template <typename C>
		static bool registerPlugin(const PluginInfo& info)
		{
			return get().template l_registerPlugin<C>(info);
		}
		template <typename C>
		static bool registerPlugin(const StaticPluginInfo& info)
		{
			return get().template l_registerPlugin<C>(info);
		}
		static bool loadPlugin(const std::string& pluginFilename);
		static T* createObject(const std::string& objectType);
		static StringList names();
		static void setLog(LogPtr& log);
		static void loadAll();
		static bool loadDynamic(const std::string& driverName);
		static PluginManager<T>& get();
		static StageExtensions& extensions();

	private:
		PluginManager();

		std::string getPath(const std::string& driver);
		void shutdown();
		bool loadByPath(const std::string& path);
		bool l_loadDynamic(const std::string& driverName);
		DynamicLibrary* libraryLoaded(const std::string& path);
		DynamicLibrary* loadLibrary(const std::string& path);
		T* l_createObject(const std::string& objectType);
		template <class C>
		bool l_registerPlugin(const PluginInfo& pi)
		{
			auto f = [&]()
			{
				T* t = dynamic_cast<T*>(new C);
				return t;
			};
			Info info{ pi.name, pi.link, pi.description, f };
			std::lock_guard<std::mutex> lock(m_pluginMutex);
			m_plugins.insert(std::make_pair(pi.name, info));
			return true;
		}
		template <class C>
		bool l_registerPlugin(const StaticPluginInfo& pi)
		{
			l_registerPlugin<C>((const PluginInfo&)pi);
			m_extensions.set(pi.name, pi.extensions);
			return true;
		}

		bool l_loadPlugin(const std::string& pluginFilename);
		StringList l_names();
		std::string l_description(const std::string& name);
		std::string l_link(const std::string& name);
		void l_loadAll();

		DynamicLibraryMap m_dynamicLibraryMap;
		RegistrationInfoMap m_plugins;
		std::mutex m_pluginMutex;
		std::mutex m_libMutex;
		LogPtr m_log;
		StageExtensions m_extensions;
	};

}

