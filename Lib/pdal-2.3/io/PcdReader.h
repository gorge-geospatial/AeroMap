#ifndef PCDREADER_H
#define PCDREADER_H

#include <pdal/Reader.h>
#include <pdal/Streamable.h>
#include <pdal/util/IStream.h>

#include "PcdHeader.h"

namespace pdal
{
	class PcdReader : public Reader, public Streamable
	{
	public:
		std::string getName() const;

		PcdReader();

	private:
		virtual QuickInfo inspect();
		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual point_count_t read(PointViewPtr view, point_count_t numPts);
		virtual void done(PointTableRef table);
		virtual bool processOne(PointRef& point);
		bool fillFields();

		PcdHeader m_header;
		std::istream* m_istreamPtr;
		ILeStream m_stream;
		Dimension::IdList m_dims;
		StringList m_fields;
		point_count_t m_index;
		size_t m_line;
	};
}

#endif // #ifndef PCDREADER_H
