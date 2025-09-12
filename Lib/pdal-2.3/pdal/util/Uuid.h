/******************************************************************************
* Copyright (c) 2014, Hobu Inc., hobu@hobu.co
****************************************************************************/

// This is a C++ification of the libuuid code, less the code that actually
// creates UUIDs, which is most of it and we don't need at this time.

#pragma once

#include <cstdint>
#include <string>
#include <iomanip>

#include "Inserter.h"
#include "Extractor.h"

namespace pdal
{
#pragma pack(push)
#pragma pack(1)
	struct uuid
	{
		uint32_t time_low;
		uint16_t time_mid;
		uint16_t time_hi_and_version;
		uint16_t clock_seq;
		uint8_t node[6];
	};
#pragma pack(pop)

	inline bool operator < (const uuid& u1, const uuid& u2)
	{
		if (u1.time_low != u2.time_low)
			return u1.time_low < u2.time_low;
		if (u1.time_mid != u2.time_mid)
			return u1.time_mid < u2.time_mid;
		if (u1.time_hi_and_version != u2.time_hi_and_version)
			return u1.time_hi_and_version < u2.time_hi_and_version;
		for (size_t i = 0; i < sizeof(u1.node); ++i)
			if (u1.node[i] != u2.node[i])
				return u1.node[i] < u2.node[i];
		return false;
	}

	class Uuid
	{
		friend inline bool operator < (const Uuid& u1, const Uuid& u2);
	public:
		Uuid()
		{
			clear();
		}
		Uuid(const char* c)
		{
			unpack(c);
		}
		Uuid(const std::string& s)
		{
			parse(s);
		}

		void clear()
		{
			memset(&m_data, 0, sizeof(m_data));
		}

		void unpack(const char* c)
		{
			BeExtractor e(c, 10);

			e >> m_data.time_low >> m_data.time_mid >>
				m_data.time_hi_and_version >> m_data.clock_seq;
			c += 10;
			std::copy(c, c + 6, m_data.node);
		}

		void pack(char* c) const
		{
			BeInserter i(c, 10);

			i << m_data.time_low << m_data.time_mid <<
				m_data.time_hi_and_version << m_data.clock_seq;
			c += 10;
			std::copy(m_data.node, m_data.node + 6, c);
		}

		bool parse(const std::string& s)
		{
			if (s.length() != 36)
				return false;

			// Format validation.
			const char* cp = s.data();
			for (size_t i = 0; i < 36; i++) {
				if ((i == 8) || (i == 13) || (i == 18) || (i == 23))
				{
					if (*cp != '-')
						return false;
				}
				else if (!isxdigit(*cp))
					return false;
				++cp;
			}

			cp = s.data();
			m_data.time_low = strtoul(cp, NULL, 16);
			m_data.time_mid = (uint16_t)strtoul(cp + 9, NULL, 16);
			m_data.time_hi_and_version = (uint16_t)strtoul(cp + 14, NULL, 16);
			m_data.clock_seq = (uint16_t)strtoul(cp + 19, NULL, 16);

			// Extract bytes as pairs of hex digits.
			cp = s.data() + 24;
			char buf[3];
			buf[2] = 0;
			for (size_t i = 0; i < 6; i++) {
				buf[0] = *cp++;
				buf[1] = *cp++;
				m_data.node[i] = (uint8_t)strtoul(buf, NULL, 16);
			}
			return true;
		}

		std::string unparse() const
		{
			std::stringstream out;

			out << std::hex << std::uppercase << std::setfill('0');
			out << std::setw(8) << m_data.time_low << '-';
			out << std::setw(4) << m_data.time_mid << '-';
			out << std::setw(4) << m_data.time_hi_and_version << '-';
			out << std::setw(2) << (m_data.clock_seq >> 8);
			out << std::setw(2) << (m_data.clock_seq & 0xFF) << '-';
			for (size_t i = 0; i < 6; ++i)
				out << std::setw(2) << (int)m_data.node[i];
			return out.str();
		}

		std::string toString() const
		{
			return unparse();
		}

		bool empty() const
		{
			return isNull();
		}

		bool isNull() const
		{
			const char* c = (const char*)&m_data;
			for (size_t i = 0; i < sizeof(m_data); ++i)
				if (*c++ != 0)
					return false;
			return true;
		}

		static constexpr size_t size()
		{
			return sizeof(m_data);
		}

	private:
		uuid m_data;
	};

	inline bool operator == (const Uuid& u1, const Uuid& u2)
	{
		return !(u1 < u2) && !(u2 < u1);
	}

	inline bool operator < (const Uuid& u1, const Uuid& u2)
	{
		return u1.m_data < u2.m_data;
	}

	inline std::ostream& operator << (std::ostream& out, const Uuid& u)
	{
		out << u.toString();
		return out;
	}

	inline std::istream& operator >> (std::istream& in, Uuid& u)
	{
		std::string s;
		in >> s;
		if (!u.parse(s))
			in.setstate(std::ios::failbit);
		return in;
	}
}
