/******************************************************************************
 * Copyright (c) 2014, Hobu Inc. (howard@hobu.co)
 ****************************************************************************/
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>

namespace hexer
{
	class hexer_error : public std::runtime_error
	{
	public:
		hexer_error(std::string const& msg)
			: std::runtime_error(msg)
		{}
	};
}

#endif // #ifndef EXCEPTION_H
