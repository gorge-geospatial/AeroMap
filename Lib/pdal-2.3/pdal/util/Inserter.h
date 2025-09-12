/******************************************************************************
* Copyright (c) 2014, Hobu Inc., hobu@hobu.co
****************************************************************************/
#pragma once

#include "endian.h"

namespace pdal
{
	class Inserter
	{
	public:
		Inserter(unsigned char* buf, std::size_t size) : m_pbase((char*)buf),
			m_epptr((char*)buf + size), m_pptr((char*)buf)
		{}
		Inserter(char* buf, std::size_t size) : m_pbase(buf),
			m_epptr(buf + size), m_pptr(buf)
		{}

	protected:
		// Base pointer - start of buffer (names taken from std::streambuf).
		char* m_pbase;
		// End pointer.
		char* m_epptr;
		// Current position.
		char* m_pptr;

	public:
		operator bool() const
		{
			return good();
		}
		bool good() const
		{
			return m_pptr < m_epptr;
		}
		void seek(std::size_t pos)
		{
			m_pptr = m_pbase + pos;
		}
		void put(const std::string& s)
		{
			put(s, s.size());
		}
		void put(std::string s, size_t len)
		{
			s.resize(len);
			put(s.data(), len);
		}
		void put(const char* c, size_t len)
		{
			memcpy(m_pptr, c, len);
			m_pptr += len;
		}
		void put(const unsigned char* c, size_t len)
		{
			memcpy(m_pptr, c, len);
			m_pptr += len;
		}
		std::size_t position() const
		{
			return m_pptr - m_pbase;
		}

		virtual Inserter& operator << (uint8_t v) = 0;
		virtual Inserter& operator << (int8_t v) = 0;
		virtual Inserter& operator << (uint16_t v) = 0;
		virtual Inserter& operator << (int16_t v) = 0;
		virtual Inserter& operator << (uint32_t v) = 0;
		virtual Inserter& operator << (int32_t v) = 0;
		virtual Inserter& operator << (uint64_t v) = 0;
		virtual Inserter& operator << (int64_t v) = 0;
		virtual Inserter& operator << (float v) = 0;
		virtual Inserter& operator << (double v) = 0;
	};

	/// Stream wrapper for output of binary data that converts from host ordering
	/// to little endian format
	class LeInserter : public Inserter
	{
	public:
		LeInserter(char* buf, std::size_t size) : Inserter(buf, size)
		{}
		LeInserter(unsigned char* buf, std::size_t size) : Inserter(buf, size)
		{}

		LeInserter& operator << (uint8_t v)
		{
			*m_pptr++ = (char)v;
			return *this;
		}

		LeInserter& operator << (int8_t v)
		{
			*m_pptr++ = v;
			return *this;
		}

		LeInserter& operator << (uint16_t v)
		{
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		LeInserter& operator << (int16_t v)
		{
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		LeInserter& operator << (uint32_t v)
		{
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		LeInserter& operator << (int32_t v)
		{
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		LeInserter& operator << (uint64_t v)
		{
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		LeInserter& operator << (int64_t v)
		{
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		LeInserter& operator << (float v)
		{
			union
			{
				float f;
				uint32_t u;
			} uu;

			uu.f = v;
			memcpy(m_pptr, &uu.f, sizeof(uu.f));
			m_pptr += sizeof(uu.f);
			return *this;
		}

		LeInserter& operator << (double v)
		{
			union
			{
				double d;
				uint64_t u;
			} uu;

			uu.d = v;
			memcpy(m_pptr, &uu.d, sizeof(uu.d));
			m_pptr += sizeof(uu.d);
			return *this;
		}
	};


	/// Stream wrapper for output of binary data that converts from host ordering
	/// to big endian format
	class BeInserter : public Inserter
	{
	public:
		BeInserter(char* buf, std::size_t size) : Inserter(buf, size)
		{}
		BeInserter(unsigned char* buf, std::size_t size) : Inserter(buf, size)
		{}

		BeInserter& operator << (uint8_t v)
		{
			*m_pptr++ = (char)v;
			return *this;
		}

		BeInserter& operator << (int8_t v)
		{
			*m_pptr++ = v;
			return *this;
		}

		BeInserter& operator << (uint16_t v)
		{
			v = ByteSwap(v);
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		BeInserter& operator << (int16_t v)
		{
			v = (int16_t)ByteSwap((uint16_t)v);
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		BeInserter& operator << (uint32_t v)
		{
			v = ByteSwap(v);
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		BeInserter& operator << (int32_t v)
		{
			v = (int32_t)ByteSwap((uint32_t)v);
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		BeInserter& operator << (uint64_t v)
		{
			v = ByteSwap(v);
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		BeInserter& operator << (int64_t v)
		{
			v = (int64_t)ByteSwap((uint64_t)v);
			memcpy(m_pptr, &v, sizeof(v));
			m_pptr += sizeof(v);
			return *this;
		}

		BeInserter& operator << (float v)
		{
			union
			{
				float f;
				uint32_t u;
			} uu;

			uu.f = v;
			uu.u = ByteSwap(uu.u);
			memcpy(m_pptr, &uu.f, sizeof(uu.f));
			m_pptr += sizeof(uu.f);
			return *this;
		}

		BeInserter& operator << (double v)
		{
			union
			{
				double d;
				uint64_t u;
			} uu;

			uu.d = v;
			uu.u = ByteSwap(uu.u);
			memcpy(m_pptr, &uu.d, sizeof(uu.d));
			m_pptr += sizeof(uu.d);
			return *this;
		}
	};
}
