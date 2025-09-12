#ifndef BPFCOMPRESSOR_H
#define BPFCOMPRESSOR_H

#include <stdexcept>
#include <ostream>

#include <pdal/pdal_features.h>
#include <pdal/util/Charbuf.h>
#include <pdal/util/OStream.h>

#ifdef PDAL_HAVE_ZLIB
#include <zlib.h>
#endif // PDAL_HAVE_ZLIB

namespace pdal
{
	class BpfCompressor
	{
	public:
		struct error : public std::runtime_error
		{
			error(const std::string& err) : std::runtime_error(err)
			{}
		};

#ifdef PDAL_HAVE_ZLIB
		BpfCompressor(OLeStream& out, size_t maxSize) :
			m_out(out), m_inbuf(maxSize), m_blockStart(out), m_rawSize(0),
			m_compressedSize(0)
		{}
#else
		BpfCompressor(OLeStream&, size_t)
		{}
#endif // PDAL_HAVE_ZLIB

		void startBlock();
		void finish();
		void compress();

	private:
		static const int CHUNKSIZE = 1000000;

#ifdef PDAL_HAVE_ZLIB
		OLeStream& m_out;
		Charbuf m_charbuf;
		std::vector<char> m_inbuf;
		z_stream m_strm;
		unsigned char m_tmpbuf[CHUNKSIZE];
		OStreamMarker m_blockStart;
		size_t m_rawSize;
		size_t m_compressedSize;
#endif // PDAL_HAVE_ZLIB
	};
}

#endif // #ifndef BPFCOMPRESSOR_H
