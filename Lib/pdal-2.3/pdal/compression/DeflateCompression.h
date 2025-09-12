/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
****************************************************************************/
#pragma once

#include "Compression.h"

namespace pdal
{

	class DeflateCompressorImpl;

	class DeflateCompressor : public Compressor
	{
	public:
		PDAL_DLL DeflateCompressor(BlockCb cb);
		PDAL_DLL ~DeflateCompressor();

		PDAL_DLL void compress(const char* buf, size_t bufsize);
		PDAL_DLL void done();

	private:
		std::unique_ptr<DeflateCompressorImpl> m_impl;
	};


	class DeflateDecompressorImpl;

	class DeflateDecompressor : public Decompressor
	{
	public:
		PDAL_DLL DeflateDecompressor(BlockCb cb);
		PDAL_DLL ~DeflateDecompressor();

		PDAL_DLL void decompress(const char* buf, size_t bufsize);
		PDAL_DLL void done();

	private:
		std::unique_ptr<DeflateDecompressorImpl> m_impl;
	};

} // namespace pdal
