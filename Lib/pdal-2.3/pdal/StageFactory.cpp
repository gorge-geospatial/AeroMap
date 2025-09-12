#ifndef STAGEFACTORY_H
#define STAGEFACTORY_H

#include <pdal/StageFactory.h>
#include <pdal/PluginManager.h>
#include <pdal/util/FileUtils.h>

#include <algorithm>
#include <sstream>
#include <string>

namespace pdal
{
	/**
	  Find the default reader for a file.

	  \param filename  Filename for which to infer a reader.
	  \return  Name of the reader driver associated with the file.
	*/
	std::string StageFactory::inferReaderDriver(const std::string& filename)
	{
		std::string ext;

		if (Utils::endsWith(filename, "ept.json") || Utils::startsWith(filename, "ept://"))
			return "readers.ept";
		if (Utils::startsWith(filename, "i3s://"))
			return "readers.i3s";

		ext = FileUtils::extension(filename);
		// Strip off '.' and make lowercase.
		if (ext.length() > 1)
			ext = Utils::tolower(ext.substr(1));

		return PluginManager<Stage>::extensions().defaultReader(ext);
	}

	/**
	  Find the default writer for a file.

	  \param filename  Filename for which to infer a writer.
	  \return  Name of the writer driver associated with the file.
	*/
	std::string StageFactory::inferWriterDriver(const std::string& filename)
	{
		std::string lFilename = Utils::tolower(filename);
		if (lFilename == "devnull" || lFilename == "/dev/null")
			return "writers.null";

		std::string ext;
		if (lFilename == "stdout")
			ext = ".txt";
		else
			ext = Utils::tolower(FileUtils::extension(lFilename));
		// Strip off '.' and make lowercase.
		if (ext.length() > 1)
			ext = Utils::tolower(ext.substr(1));

		return PluginManager<Stage>::extensions().defaultWriter(ext);
	}

	StageFactory::StageFactory(bool /* legacy */)
	{
	}

	Stage* StageFactory::createStage(std::string const& stage_name)
	{
		static_assert(0 < sizeof(Stage), "");
		Stage* s = PluginManager<Stage>::createObject(stage_name);
		if (s)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_ownedStages.push_back(std::unique_ptr<Stage>(s));
		}
		return s;
	}

	void StageFactory::destroyStage(Stage* s)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (auto it = m_ownedStages.begin(); it != m_ownedStages.end(); ++it)
		{
			if (s == it->get())
			{
				m_ownedStages.erase(it);
				break;
			}
		}
	}
}

#endif // #ifndef STAGEFACTORY_H
