/******************************************************************************
* Copyright (c) 2014, Hobu Inc.
****************************************************************************/

#include <pdal/util/Charbuf.h>

namespace pdal
{
	void Charbuf::initialize(char* buf, size_t count, std::ios::pos_type bufOffset)
	{
		m_bufOffset = bufOffset;
		m_buf = buf;
		setg(buf, buf, buf + count);
		setp(buf, buf + count);
	}

	std::ios::pos_type Charbuf::seekpos(std::ios::pos_type pos, std::ios_base::openmode which)
	{
		pos -= m_bufOffset;
		if (which & std::ios_base::in)
		{
			if (pos >= egptr() - eback())
				return -1;
			char* cpos = eback() + pos;
			setg(eback(), cpos, egptr());
		}
		if (which & std::ios_base::out)
		{
			if (pos > epptr() - m_buf)
				return -1;
			char* cpos = m_buf + pos;
			setp(cpos, epptr());
		}
		return pos;
	}

	std::ios::pos_type
		Charbuf::seekoff(std::ios::off_type off, std::ios_base::seekdir dir,
			std::ios_base::openmode which)
	{
		std::ios::pos_type pos;
		char* cpos = nullptr;
		if (which & std::ios_base::in)
		{
			switch (dir)
			{
			case std::ios::beg:
				cpos = eback() + off - m_bufOffset;
				break;
			case std::ios::cur:
				cpos = gptr() + off;
				break;
			case std::ios::end:
				cpos = egptr() - off;
				break;
			default:
				break;  // Should never happen.
			}
			if (cpos < eback() || cpos > egptr())
				return -1;
			setg(eback(), cpos, egptr());
			pos = cpos - eback();
		}
		if (which & std::ios_base::out)
		{
			switch (dir)
			{
			case std::ios::beg:
				cpos = m_buf + off - m_bufOffset;
				break;
			case std::ios::cur:
				cpos = pptr() + off;
				break;
			case std::ios::end:
				cpos = egptr() - off;
				break;
			default:
				break;  // Should never happen.
			}
			if (cpos < m_buf || cpos > epptr())
				return -1;
			setp(cpos, epptr());
			pos = cpos - m_buf;
		}
		return pos;
	}
}
