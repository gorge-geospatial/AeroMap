/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
****************************************************************************/
#pragma once

#include <memory>
#include <vector>

namespace pdal
{
	class LazPerfVlrCompressorImpl;

	// This compressor write data in chunks to a stream. At the beginning of the
	// data is an offset to the end of the data, where the chunk table is
	// stored.  The chunk table keeps a list of the offsets to the beginning of
	// each chunk.  Chunks consist of a fixed number of points (last chunk may
	// have fewer points).  Each time a chunk starts, the compressor is reset.
	// This allows decompression of some set of points that's less than the
	// entire set when desired.
	// The compressor uses the schema of the point data in order to compress
	// the point stream.  The schema is also stored in a VLR that isn't
	// handled as part of the compression process itself.
	class LazPerfVlrCompressor
	{
	public:
		LazPerfVlrCompressor(std::ostream& stream, int format, int ebCount);
		LazPerfVlrCompressor(std::ostream& stream, int format, int ebCount,
			uint32_t chunksize);
		~LazPerfVlrCompressor();

		std::vector<char> vlrData() const;
		void compress(const char* inbuf);
		void done();

	private:
		std::unique_ptr<LazPerfVlrCompressorImpl> m_impl;
	};

	class LazPerfVlrDecompressorImpl;
	class LazPerfVlrDecompressor
	{
	public:
		LazPerfVlrDecompressor(std::istream& stream, int format, int ebCount,
			std::streamoff pointOffset, const char* virdata);
		~LazPerfVlrDecompressor();

		bool seek(int64_t record);
		void decompress(char* outbuf);

	private:
		std::unique_ptr<LazPerfVlrDecompressorImpl> m_impl;
	};

} // namespace pdal

