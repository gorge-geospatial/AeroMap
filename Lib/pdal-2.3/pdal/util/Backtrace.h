/******************************************************************************
* Copyright (c) 2018, Hobu Inc. (info@hobu.co)
****************************************************************************/

#pragma once

#include <vector>
#include <string>

#include <pdal/pdal_export.h>

namespace pdal
{
	namespace Utils
	{
		/**
		  Generate a backtrace as a list of strings.

		  \return  List of functions at the point of the call.
		*/
		std::vector<std::string> backtrace();
	}
}
