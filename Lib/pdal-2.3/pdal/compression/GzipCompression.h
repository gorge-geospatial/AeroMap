/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
****************************************************************************/
#pragma once

#include "Compression.h"

namespace pdal
{

	class DeflateDecompressorImpl;

	// We only support decompression for now.
	// The implementation is in DeflateCompression.cpp.
	class GzipDecompressor : public Decompressor
	{
	public:
		PDAL_DLL GzipDecompressor(BlockCb cb);
		PDAL_DLL ~GzipDecompressor();

		PDAL_DLL void decompress(const char* buf, size_t bufsize);
		PDAL_DLL void done();

	private:
		// Gzip uses deflate.
		std::unique_ptr<DeflateDecompressorImpl> m_impl;
	};

} // namespace pdal
