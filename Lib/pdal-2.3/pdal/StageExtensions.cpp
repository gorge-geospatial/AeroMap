/******************************************************************************
* Copyright (c) 2018, Hobu Inc. (info@hobu.co)
****************************************************************************/

#include <sstream>

#include <pdal/StageExtensions.h>
#include <pdal/util/FileUtils.h>

namespace pdal
{
	namespace
	{
		// NOTE: Only extensions for dynamic stages go here.  Static stage extensions
		//  are defined in the stage files themselves.

		using Extensions = std::map<std::string, StringList>;

		static const Extensions readerExtensions =
		{
		  { "readers.draco", { "drc" } },
		  { "readers.icebridge", { "icebridge", "h5" } },
		  { "readers.matlab", { "mat" } },
		  { "readers.nitf", { "nitf", "nsf", "ntf" } },
		  { "readers.pcd", { "pcd" } },
		  { "readers.rdb", { "rdbx" } },
		  { "readers.mrsid", { "sid" } },
		  { "readers.rxp", { "rxp" } },
		  { "readers.fbx", { "fbx" } },
		  { "readers.slpk", { "slpk" } },
		  { "readers.i3s", { "i3s" } },
		  { "readers.obj", { "obj" } },
		  { "readers.e57", { "e57" } }
		};

		static const Extensions writerExtensions =
		{
		  { "writers.draco", { "drc" } },
		  { "writers.matlab", { "mat" } },
		  { "writers.nitf", { "nitf", "nsf", "ntf" } },
		  { "writers.pcd", { "pcd" } },
		  { "writers.e57", { "e57" } },
		  { "writers.fbx", { "fbx" } }
		};
	}

	StageExtensions::StageExtensions(LogPtr log) : m_log(log)
	{}

	void StageExtensions::load()
	{
		static bool loaded(false);

		if (loaded)
			return;
		loaded = true;

		std::lock_guard<std::mutex> lock(m_mutex);
		for (auto& p : readerExtensions)
		{
			const std::string& stage = p.first;
			for (auto& ext : p.second)
				m_readers[ext] = stage;
		}
		for (auto& p : writerExtensions)
		{
			const std::string& stage = p.first;
			for (auto& ext : p.second)
				m_writers[ext] = stage;
		}
	}

	void StageExtensions::set(const std::string& stage, const StringList& exts)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (Utils::startsWith(stage, "readers."))
			for (const std::string& ext : exts)
				m_readers[ext] = stage;
		else if (Utils::startsWith(stage, "writers."))
			for (const std::string& ext : exts)
				m_writers[ext] = stage;
	}

	// Get the default reader associated with an extension.  Extensions
	// are specified without the leading '.'
	std::string StageExtensions::defaultReader(const std::string& extension)
	{
		load();
		std::lock_guard<std::mutex> lock(m_mutex);
		return (m_readers[extension]);
	}

	// Get the default writer associated with an extension.  Extensions
	// are specified without the leading '.'
	std::string StageExtensions::defaultWriter(const std::string& extension)
	{
		load();
		std::lock_guard<std::mutex> lock(m_mutex);
		return (m_writers[extension]);
	}

	StringList StageExtensions::extensions(const std::string& stage)
	{
		StringList exts;

		std::lock_guard<std::mutex> lock(m_mutex);

		if (Utils::startsWith(stage, "readers."))
		{
			for (auto& entry : m_readers)
				if (entry.second == stage)
					exts.push_back(entry.first);
		}
		else if (Utils::startsWith(stage, "writers."))
		{
			for (auto& entry : m_writers)
				if (entry.second == stage)
					exts.push_back(entry.first);
		}
		return exts;
	}
}
