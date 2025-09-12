#ifndef GEOTIFFSUPPORT_H
#define GEOTIFFSUPPORT_H

#include <pdal/Log.h>
#include <pdal/SpatialReference.h>

namespace pdal
{
	namespace Geotiff
	{
		struct error : public std::runtime_error
		{
			error(const std::string& err) : std::runtime_error(err)
			{}
		};
	}

	struct Entry;

	class GeotiffSrs
	{
	public:
		GeotiffSrs(const std::vector<uint8_t>& directoryRec,
			const std::vector<uint8_t>& doublesRec,
			const std::vector<uint8_t>& asciiRec, LogPtr log);
		SpatialReference srs() const
		{
			return m_srs;
		}

		std::string const& gtiffPrintString()
		{
			return m_gtiff_print_string;
		}

	private:
		SpatialReference m_srs;
		LogPtr m_log;
		std::string m_gtiff_print_string;

		void validateDirectory(const Entry* ent, size_t numEntries,
			size_t numDoubles, size_t asciiSize);
	};

	class GeotiffTags
	{
	public:
		GeotiffTags(const SpatialReference& srs);

		std::vector<uint8_t>& directoryData()
		{
			return m_directoryRec;
		}
		std::vector<uint8_t>& doublesData()
		{
			return m_doublesRec;
		}
		std::vector<uint8_t>& asciiData()
		{
			return m_asciiRec;
		}

	private:
		std::vector<uint8_t> m_directoryRec;
		std::vector<uint8_t> m_doublesRec;
		std::vector<uint8_t> m_asciiRec;
	};
}

#endif // #ifndef GEOTIFFSUPPORT_H
