/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
****************************************************************************/
#pragma once

#include "Compression.h"

namespace pdal
{
	class LzmaCompressorImpl;
	class LzmaCompressor : public Compressor
	{
	public:
		PDAL_DLL LzmaCompressor(BlockCb cb);
		PDAL_DLL ~LzmaCompressor();

		PDAL_DLL void compress(const char* buf, size_t bufsize);
		PDAL_DLL void done();

	private:
		std::unique_ptr<LzmaCompressorImpl> m_impl;
	};

	class LzmaDecompressorImpl;

	class LzmaDecompressor : public Decompressor
	{
	public:
		PDAL_DLL LzmaDecompressor(BlockCb cb);
		PDAL_DLL ~LzmaDecompressor();

		PDAL_DLL void decompress(const char* buf, size_t bufsize);
		PDAL_DLL void done();

	private:
		std::unique_ptr<LzmaDecompressorImpl> m_impl;
	};
}
