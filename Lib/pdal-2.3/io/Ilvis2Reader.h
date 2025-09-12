/******************************************************************************
* Copyright (c) 2015, Howard Butler (howard@hobu.co)
****************************************************************************/

#include <pdal/pdal_features.h> // For PDAL_HAVE_LIBXML2
#include <pdal/PointView.h>
#include <pdal/Reader.h>
#include <pdal/Streamable.h>
#include <pdal/util/IStream.h>
#include <map>

#ifndef PDAL_HAVE_LIBXML2
namespace pdal
{
	class Ilvis2MetadataReader
	{
	public:
		inline void readMetadataFile(std::string filename, pdal::MetadataNode* m) {};
	};
}
#else
#include "Ilvis2MetadataReader.hpp"
#endif

namespace pdal
{
	class PDAL_DLL Ilvis2Reader : public Reader, public Streamable
	{
	public:
		enum class IlvisMapping
		{
			INVALID,
			LOW,
			HIGH,
			ALL
		};

		struct error : public std::runtime_error
		{
			error(const std::string& err) : std::runtime_error(err)
			{}
		};

		Ilvis2Reader();
		~Ilvis2Reader();

		std::string getName() const;

	private:
		std::unique_ptr<std::ifstream> m_stream;
		IlvisMapping m_mapping;
		StringList m_fields;
		size_t m_lineNum;
		bool m_resample;
		PointLayoutPtr m_layout;
		std::string m_metadataFile;
		Ilvis2MetadataReader m_mdReader;

		virtual void addDimensions(PointLayoutPtr layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual point_count_t read(PointViewPtr view, point_count_t count);
		virtual void done(PointTableRef table);

		virtual void readPoint(PointRef& point, StringList s, std::string pointMap);
	};

	std::ostream& operator<<(std::ostream& out, const Ilvis2Reader::IlvisMapping& mval);
}
