#ifndef BPFREADER_H
#define BPFREADER_H

#include <vector>

#include <pdal/Reader.h>
#include <pdal/Streamable.h>
#include <pdal/util/Charbuf.h>
#include <pdal/util/IStream.h>
#include <pdal/pdal_export.h>

#include "BpfHeader.h"

#include <vector>

namespace pdal
{
	class BpfReader : public Reader, 
					  public Streamable
	{
		struct Args;
	public:
		BpfReader();
		~BpfReader();

		std::string getName() const;

		virtual point_count_t numPoints() const
		{
			return (point_count_t)m_header.m_numPts;
		}

	private:
		std::istream* m_istreamPtr;
		ILeStream m_stream;
		BpfHeader m_header;
		BpfDimensionList m_dims;
		Dimension::IdList m_schemaDims;
		BpfUlemHeader m_ulemHeader;
		std::vector<BpfUlemFrame> m_ulemFrames;
		BpfPolarHeader m_polarHeader;
		std::vector<BpfPolarFrame> m_polarFrames;
		/// Stream position at the beginning of point records.
		std::streampos m_start;
		/// Index of the next point to read.
		point_count_t m_index;
		/// Buffer for deflated data.
		std::vector<char> m_deflateBuf;
		/// Streambuf for deflated data.
		Charbuf m_charbuf;
		std::unique_ptr<Args> m_args;

		// For dimension-major point-at-a-time usage.
		std::vector<std::unique_ptr<ILeStream>> m_streams;
		std::vector<std::unique_ptr<Charbuf>> m_charbufs;

		virtual QuickInfo inspect();
		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr Layout);
		virtual void addArgs(ProgramArgs& args);
		virtual void ready(PointTableRef table);
		virtual bool processOne(PointRef& point);
		virtual point_count_t read(PointViewPtr data, point_count_t num);
		virtual void done(PointTableRef table);

		bool readUlemData();
		bool readUlemFiles();
		bool readHeaderExtraData();
		bool readPolarData();
		void readPointMajor(PointRef& point);
		point_count_t readPointMajor(PointViewPtr data, point_count_t count);
		void readDimMajor(PointRef& point);
		point_count_t readDimMajor(PointViewPtr data, point_count_t count);
		void readByteMajor(PointRef& point);
		point_count_t readByteMajor(PointViewPtr data, point_count_t count);
		size_t readBlock(std::vector<char>& outBuf, size_t index);
		bool eof();
		int inflate(char* inbuf, uint32_t insize, char* outbuf, uint32_t outsize);

		void seekPointMajor(PointId ptIdx);
		void seekDimMajor(size_t dimIdx, PointId ptIdx);
		void seekByteMajor(size_t dimIdx, size_t byteIdx, PointId ptIdx);
	};
}

#endif // #ifndef BPFREADER_H
