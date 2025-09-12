/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
****************************************************************************/

#include "ZstdCompression.h"

#include <zstd.h>

namespace pdal
{
	class ZstdCompressorImpl
	{
	public:
		ZSTD_CStream* m_strm;
		ZSTD_inBuffer m_inBuf;
		char m_tmpbuf[CHUNKSIZE];
		BlockCb m_cb;

		ZstdCompressorImpl(BlockCb cb, int compressionLevel) : m_cb(cb)
		{
			m_strm = ZSTD_createCStream();
			ZSTD_initCStream(m_strm, compressionLevel);
		}

		~ZstdCompressorImpl()
		{
			ZSTD_freeCStream(m_strm);
		}

		void compress(const char* buf, size_t bufsize)
		{
			m_inBuf.src = reinterpret_cast<const void*>(buf);
			m_inBuf.size = bufsize;
			m_inBuf.pos = 0;

			size_t ret;
			do
			{
				ZSTD_outBuffer outBuf
				{ reinterpret_cast<void*>(m_tmpbuf), CHUNKSIZE, 0 };
				ret = ZSTD_compressStream(m_strm, &outBuf, &m_inBuf);
				if (ZSTD_isError(ret))
					break;
				if (outBuf.pos)
					m_cb(m_tmpbuf, outBuf.pos);
			} while (m_inBuf.pos != m_inBuf.size);
		}

		void done()
		{
			size_t ret;
			do
			{
				ZSTD_outBuffer outBuf
				{ reinterpret_cast<void*>(m_tmpbuf), CHUNKSIZE, 0 };
				ret = ZSTD_endStream(m_strm, &outBuf);
				if (ZSTD_isError(ret))
					break;
				if (outBuf.pos)
					m_cb(m_tmpbuf, outBuf.pos);
			} while (ret);
		}
	};

	ZstdCompressor::ZstdCompressor(BlockCb cb) :
		m_impl(new ZstdCompressorImpl(cb, 15))
	{}


	ZstdCompressor::ZstdCompressor(BlockCb cb, int compressionLevel) :
		m_impl(new ZstdCompressorImpl(cb, compressionLevel))
	{}


	ZstdCompressor::~ZstdCompressor()
	{}


	void ZstdCompressor::compress(const char* buf, size_t bufsize)
	{
		m_impl->compress(buf, bufsize);
	}


	void ZstdCompressor::done()
	{
		m_impl->done();
	}


	class ZstdDecompressorImpl
	{
	public:
		ZstdDecompressorImpl(BlockCb cb) : m_cb(cb)
		{
			m_strm = ZSTD_createDStream();
			ZSTD_initDStream(m_strm);
		}

		~ZstdDecompressorImpl()
		{
			ZSTD_freeDStream(m_strm);
		}

		void decompress(const char* buf, size_t bufsize)
		{
			m_inBuf.src = reinterpret_cast<const void*>(buf);
			m_inBuf.size = bufsize;
			m_inBuf.pos = 0;

			size_t ret;
			do
			{
				ZSTD_outBuffer outBuf
				{ reinterpret_cast<void*>(m_tmpbuf), CHUNKSIZE, 0 };
				ret = ZSTD_decompressStream(m_strm, &outBuf, &m_inBuf);
				if (ZSTD_isError(ret))
					break;
				if (outBuf.pos)
					m_cb(m_tmpbuf, outBuf.pos);
			} while (m_inBuf.pos != m_inBuf.size);
		}

	private:
		BlockCb m_cb;

		ZSTD_DStream* m_strm;
		ZSTD_inBuffer m_inBuf;
		char m_tmpbuf[CHUNKSIZE];
	};

	ZstdDecompressor::ZstdDecompressor(BlockCb cb) :
		m_impl(new ZstdDecompressorImpl(cb))
	{}


	ZstdDecompressor::~ZstdDecompressor()
	{}


	void ZstdDecompressor::decompress(const char* buf, size_t bufsize)
	{
		m_impl->decompress(buf, bufsize);
	}
}
