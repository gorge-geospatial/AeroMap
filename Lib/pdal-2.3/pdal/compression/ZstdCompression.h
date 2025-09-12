/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
****************************************************************************/
#pragma once

#include "Compression.h"

namespace pdal
{
	class ZstdCompressorImpl;

	class ZstdCompressor : public Compressor
	{
	public:
		ZstdCompressor(BlockCb cb);
		ZstdCompressor(BlockCb cb, int compressionLevel);
		~ZstdCompressor();

		void compress(const char* buf, size_t bufsize);
		void done();

	private:
		std::unique_ptr<ZstdCompressorImpl> m_impl;
	};

	class ZstdDecompressorImpl;

	class ZstdDecompressor : public Decompressor
	{
	public:
		ZstdDecompressor(BlockCb cb);
		~ZstdDecompressor();

		void decompress(const char* buf, size_t bufsize);
		void done()
		{}

	private:
		std::unique_ptr<ZstdDecompressorImpl> m_impl;
	};
}
