#ifndef STAGEEXTENSIONS_H
#define STAGEEXTENSIONS_H

#include <map>
#include <mutex>

#include <pdal/Log.h>
#include <pdal/pdal_types.h>

namespace pdal
{
	class StageExtensions
	{
	public:
		StageExtensions(LogPtr log);

		void set(const std::string& stage, const StringList& exts);
		std::string defaultReader(const std::string& filename);
		std::string defaultWriter(const std::string& filename);
		StringList extensions(const std::string& stage);
	private:
		void load();

		LogPtr m_log;
		std::mutex m_mutex;
		std::map<std::string, std::string> m_readers;
		std::map<std::string, std::string> m_writers;
	};
}

#endif // #ifndef STAGEEXTENSIONS_H
