/******************************************************************************
* Copyright (c) 2015, Peter J. Gadomski <pete.gadomski@gmail.com>
****************************************************************************/

#include <memory>
#include <vector>

#include <pdal/Reader.h>
#include <pdal/PointTable.h>
#include <pdal/PointView.h>
#include <pdal/util/Extractor.h>
#include <pdal/util/Georeference.h>
#include <pdal/util/IStream.h>

#include "OptechCommon.h"

namespace pdal
{

	class OptechReader : public Reader
	{
	public:
		std::string getName() const;

		static const size_t MaximumNumberOfReturns = 4;
		static const size_t NumBytesInRecord = 69;
		static const size_t BufferSize = 1000000;
		static const size_t MaxNumRecordsInBuffer = BufferSize / NumBytesInRecord;

		OptechReader();

		const CsdHeader& getHeader() const;

	private:
		typedef std::vector<char> buffer_t;
		typedef buffer_t::size_type buffer_size_t;

		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual point_count_t read(PointViewPtr view, point_count_t num);
		size_t fillBuffer();
		virtual void done(PointTableRef table);

		CsdHeader m_header;
		georeference::RotationMatrix m_boresightMatrix;
		std::unique_ptr<IStream> m_istream;
		buffer_t m_buffer;
		LeExtractor m_extractor;
		size_t m_recordIndex;
		size_t m_returnIndex;
		CsdPulse m_pulse;
	};
}
