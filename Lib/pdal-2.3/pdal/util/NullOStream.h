/******************************************************************************
* Copyright (c) 2017, Hobu Inc. (info@hobu.co)
****************************************************************************/

// Copied from http://stackoverflow.com/questions/8243743/is-there-a-null-stdostream-implementation-in-c-or-libraries

#pragma once

#include <iostream>
#include <streambuf>

namespace pdal
{
	class NullStreambuf : public std::streambuf
	{
		char dummyBuffer[64];
	protected:
		virtual int overflow(int c)
		{
			setp(dummyBuffer, dummyBuffer + sizeof(dummyBuffer));
			return (c == traits_type::eof()) ? '\0' : c;
		}
	};

	class NullOStream : private NullStreambuf, public std::ostream
	{
	public:
		NullOStream() : std::ostream(this)
		{}
		NullStreambuf* rdbuf() const
		{
			return const_cast<NullOStream*>(this);
		}
	};
}
