/******************************************************************************
* Copyright (c) 2014, Howard Butler (howard@hobu.co)
****************************************************************************/

#include "LzmaCompression.h"

#include <lzma.h>

namespace pdal
{
	class Lzma
	{
	protected:
		Lzma(BlockCb cb) : m_cb(cb)
		{
			m_strm = LZMA_STREAM_INIT;
		}

		~Lzma()
		{
			lzma_end(&m_strm);
		}

		void run(const char* buf, size_t bufsize, lzma_action mode)
		{
			m_strm.avail_in = bufsize;
			m_strm.next_in = reinterpret_cast<unsigned char*>(
				const_cast<char*>(buf));
			int ret = LZMA_OK;
			do
			{
				m_strm.avail_out = CHUNKSIZE;
				m_strm.next_out = m_tmpbuf;
				ret = lzma_code(&m_strm, mode);
				size_t written = CHUNKSIZE - m_strm.avail_out;
				if (written)
					m_cb(reinterpret_cast<char*>(m_tmpbuf), written);
			} while (ret == LZMA_OK);
			if (ret == LZMA_STREAM_END)
				return;

			switch (ret)
			{
			case LZMA_MEM_ERROR:
				throw compression_error("Memory allocation failure.");
			case LZMA_DATA_ERROR:
				throw compression_error("LZMA data error.");
			case LZMA_OPTIONS_ERROR:
				throw compression_error("Unsupported option.");
			case LZMA_UNSUPPORTED_CHECK:
				throw compression_error("Unsupported integrity check.");
			}
		}

	protected:
		lzma_stream m_strm;

	private:
		unsigned char m_tmpbuf[CHUNKSIZE];
		BlockCb m_cb;
	};


	class LzmaCompressorImpl : public Lzma
	{
	public:
		LzmaCompressorImpl(BlockCb cb) : Lzma(cb)
		{
			if (lzma_easy_encoder(&m_strm, 2, LZMA_CHECK_CRC64) != LZMA_OK)
				throw compression_error("Can't create compressor");
		}

		void compress(const char* buf, size_t bufsize)
		{
			run(buf, bufsize, LZMA_RUN);
		}

		void done()
		{
			run(nullptr, 0, LZMA_FINISH);
		}
	};


	LzmaCompressor::LzmaCompressor(BlockCb cb) :
		m_impl(new LzmaCompressorImpl(cb))
	{}


	LzmaCompressor::~LzmaCompressor()
	{}


	void LzmaCompressor::compress(const char* buf, size_t bufsize)
	{
		m_impl->compress(buf, bufsize);
	}


	void LzmaCompressor::done()
	{
		m_impl->done();
	}


	class LzmaDecompressorImpl : public Lzma
	{
	public:
		LzmaDecompressorImpl(BlockCb cb) : Lzma(cb)
		{
			if (lzma_auto_decoder(&m_strm, (std::numeric_limits<uint32_t>::max)(),
				LZMA_TELL_UNSUPPORTED_CHECK))
				throw compression_error("Can't create decompressor");
		}

		void decompress(const char* buf, size_t bufsize)
		{
			run(buf, bufsize, LZMA_RUN);
		}

		void done()
		{
			run(nullptr, 0, LZMA_FINISH);
		}
	};

	LzmaDecompressor::LzmaDecompressor(BlockCb cb) :
		m_impl(new LzmaDecompressorImpl(cb))
	{}


	LzmaDecompressor::~LzmaDecompressor()
	{}


	void LzmaDecompressor::decompress(const char* buf, size_t bufsize)
	{
		m_impl->decompress(buf, bufsize);
	}


	void LzmaDecompressor::done()
	{
		m_impl->done();
	}
}
