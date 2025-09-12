/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
****************************************************************************/
#pragma once

#include <pdal/pdal_internal.h>
#include <functional>

namespace pdal
{
	enum class CompressionType
	{
		None = 0,
		//    Ght = 1,   -- Removed compression type
		Dimensional = 2,
		Lazperf = 3,
		Unknown = 256
	};

	using BlockCb = std::function<void(char* buf, size_t bufsize)>;
	const size_t CHUNKSIZE(1000000);

	class compression_error : public std::runtime_error
	{
	public:
		compression_error() : std::runtime_error("General compression error")
		{}

		compression_error(const std::string& s) :
			std::runtime_error("Compression: " + s)
		{}
	};


	class Compressor
	{
	public:
		virtual ~Compressor()
		{}

		virtual void compress(const char* buf, size_t bufsize) = 0;
		virtual void done()
		{}
	};


	class Decompressor
	{
	public:
		virtual ~Decompressor()
		{}

		virtual void decompress(const char* buf, size_t bufsize) = 0;
		virtual void done()
		{}
	};
}
