#ifndef PCDWRITER_H
#define PCDWRITER_H

#include "PcdHeader.h"

#include <pdal/Writer.h>

namespace pdal
{
	class PcdWriter : public Writer
	{
		struct DimSpec
		{
			DimSpec() : m_field(PcdField()), m_precision(3)
			{
			}

			DimSpec(PcdField field, uint32_t precision)
			{
				m_field = field;
				m_precision = precision;
			}
			PcdField m_field;
			uint32_t m_precision;
		};

	public:
		std::string getName() const;

		PcdWriter();
		PcdWriter& operator=(const PcdWriter&) = delete;
		PcdWriter(const PcdWriter&) = delete;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void initialize(PointTableRef table);
		virtual void ready(PointTableRef table);
		virtual void write(const PointViewPtr view);
		virtual void done(PointTableRef table);

		DimSpec extractDim(std::string dim, PointTableRef table);
		bool findDim(Dimension::Id id, DimSpec& ds);

		PcdHeader m_header;
		std::ostream* m_ostream;
		std::string m_filename;
		std::string m_compression_string;
		bool m_writeAllDims;
		std::string m_dimOrder;
		uint32_t m_precision;

		std::vector<DimSpec> m_dims;
		DimSpec m_xDim;
		DimSpec m_yDim;
		DimSpec m_zDim;
	};
}

#endif // #ifndef PCDWRITER_H
