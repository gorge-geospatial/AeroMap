/******************************************************************************
* Copyright (c) 2014, Andrew Bell
****************************************************************************/
#pragma once

#include <sys/types.h>
#include <stdint.h>

#include <fstream>
#include <cstring>
#include <stack>

#include "endian.h"

namespace pdal
{
	class OStream
	{
	public:
		OStream() : m_stream(NULL), m_fstream(NULL)
		{}
		OStream(const std::string& filename) :
			m_stream(NULL), m_fstream(NULL)
		{
			open(filename);
		}
		OStream(std::ostream* stream) : m_stream(stream), m_fstream(NULL)
		{}
		~OStream()
		{
			delete m_fstream;
		}

		int open(const std::string& filename)
		{
			if (m_stream)
				return -1;
			m_stream = m_fstream = new std::ofstream(filename,
				std::ios_base::out | std::ios_base::binary);
			return 0;
		}
		void close()
		{
			flush();
			delete m_fstream;
			m_fstream = NULL;
			m_stream = NULL;
		}
		bool isOpen() const
		{
			return (bool)m_stream;
		}
		void flush()
		{
			m_stream->flush();
		}
		operator bool()
		{
			return (bool)(*m_stream);
		}
		void seek(std::streampos pos)
		{
			m_stream->seekp(pos, std::ostream::beg);
		}
		void put(const std::string& s)
		{
			put(s, s.size());
		}
		void put(const std::string& s, size_t len)
		{
			std::string os = s;
			os.resize(len);
			m_stream->write(os.c_str(), len);
		}
		void put(const char* c, size_t len)
		{
			m_stream->write(c, len);
		}
		void put(const unsigned char* c, size_t len)
		{
			m_stream->write((const char*)c, len);
		}
		std::streampos position() const
		{
			return m_stream->tellp();
		}
		void pushStream(std::ostream* strm)
		{
			m_streams.push(m_stream);
			m_stream = strm;
		}
		std::ostream* popStream()
		{
			// Can't pop the last stream for now.
			if (m_streams.empty())
				return nullptr;
			std::ostream* strm = m_stream;
			m_stream = m_streams.top();
			m_streams.pop();
			return strm;
		}

	protected:
		std::ostream* m_stream;
		std::ostream* m_fstream; // Dup of above to facilitate cleanup.

	private:
		std::stack<std::ostream*> m_streams;
		OStream(const OStream&);
	};

	/// Stream wrapper for output of binary data that converts from host ordering
	/// to little endian format
	class OLeStream : public OStream
	{
	public:
		OLeStream()
		{}
		OLeStream(const std::string& filename) : OStream(filename)
		{}
		OLeStream(std::ostream* stream) : OStream(stream)
		{}

		OLeStream& operator << (uint8_t v)
		{
			m_stream->put((char)v);
			return *this;
		}

		OLeStream& operator << (int8_t v)
		{
			m_stream->put((char)v);
			return *this;
		}

		OLeStream& operator << (uint16_t v)
		{
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OLeStream& operator << (int16_t v)
		{
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OLeStream& operator << (uint32_t v)
		{
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OLeStream& operator << (int32_t v)
		{
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OLeStream& operator << (uint64_t v)
		{
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OLeStream& operator << (int64_t v)
		{
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OLeStream& operator << (float v)
		{
			uint32_t tmp(0);
			std::memcpy(&tmp, &v, sizeof(v));
			m_stream->write((char*)&tmp, sizeof(tmp));
			return *this;
		}

		OLeStream& operator << (double v)
		{
			uint64_t tmp(0);
			std::memcpy(&tmp, &v, sizeof(v));
			m_stream->write((char*)&tmp, sizeof(tmp));
			return *this;
		}
	};


	/// Stream wrapper for output of binary data that converts from host ordering
	/// to big endian format
	class OBeStream : public OStream
	{
	public:
		OBeStream()
		{}
		OBeStream(const std::string& filename) : OStream(filename)
		{}
		OBeStream(std::ostream* stream) : OStream(stream)
		{}

		OBeStream& operator << (uint8_t v)
		{
			m_stream->put((char)v);
			return *this;
		}

		OBeStream& operator << (int8_t v)
		{
			m_stream->put((char)v);
			return *this;
		}

		OBeStream& operator << (uint16_t v)
		{
			v = ByteSwap(v);
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OBeStream& operator << (int16_t v)
		{
			v = (int16_t)ByteSwap((uint16_t)v);
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OBeStream& operator << (uint32_t v)
		{
			v = ByteSwap(v);
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OBeStream& operator << (int32_t v)
		{
			v = (int32_t)ByteSwap((uint32_t)v);
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OBeStream& operator << (uint64_t v)
		{
			v = ByteSwap(v);
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OBeStream& operator << (int64_t v)
		{
			v = (int64_t)ByteSwap((uint64_t)v);
			m_stream->write((char*)&v, sizeof(v));
			return *this;
		}

		OBeStream& operator << (float v)
		{
			uint32_t tmp(0);
			std::memcpy(&tmp, &v, sizeof(v));
			tmp = ByteSwap(tmp);
			m_stream->write((char*)&tmp, sizeof(tmp));
			return *this;
		}

		OBeStream& operator << (double v)
		{
			uint64_t tmp(0);
			std::memcpy(&tmp, &v, sizeof(v));
			tmp = ByteSwap(tmp);
			m_stream->write((char*)&tmp, sizeof(tmp));
			return *this;
		}
	};

	/// Stream position marker with rewinding/reset support.
	class OStreamMarker
	{
	public:
		OStreamMarker(OStream& stream) : m_stream(stream)
		{
			if (m_stream.isOpen())
				m_pos = m_stream.position();
			else
				m_pos = 0;
		}

		void mark()
		{
			m_pos = m_stream.position();
		}
		void rewind()
		{
			m_stream.seek(m_pos);
		}

	private:
		std::streampos m_pos;
		OStream& m_stream;

		OStreamMarker(const OStreamMarker&);  // not implemented
		OStreamMarker& operator=(const OStreamMarker&);  // not implemented
	};
}
