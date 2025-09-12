/******************************************************************************
* Copyright (c) 2018, Hobu Inc. (info@hobu.co)
****************************************************************************/

#include <pdal/util/Backtrace.h>
#include "private/BacktraceImpl.hpp"

#include <vector>
#include <string>

#include <pdal/util/Utils.h>

namespace pdal
{
	std::vector<std::string> Utils::backtrace()
	{
		std::vector<std::string> lines;
		BacktraceEntries entries = backtraceImpl();

		// Remove the frame for the unwinding itself.
		if (entries.size())
			entries.pop_front();

		size_t maxLibnameLen(0);
		for (auto& be : entries)
		{
			if (be.libname.empty())
				be.libname = "???";
			maxLibnameLen = std::max(maxLibnameLen, be.libname.size());
		}

		// Replace the simple symbol with a better representation if possible.
		for (size_t i = 0; i < entries.size(); ++i)
		{
			BacktraceEntry& be = entries[i];
			std::string line;

			line = std::to_string(i);
			line += std::string(4 - line.size(), ' ');
			// Should the directory info be stripped from the libname?
			line += be.libname;
			line += std::string(maxLibnameLen + 2 - be.libname.size(), ' ');
			if (be.symname.size())
				line += demangle(be.symname);
			else
			{
				std::ostringstream oss;
				intptr_t ip(reinterpret_cast<intptr_t>(be.addr));
				oss << "0x" << std::hex << std::setw(sizeof(ip) * 2) <<
					std::setfill('0') << ip;
				line += oss.str();
			}
			line += " + " + std::to_string(be.offset);
			lines.push_back(line);
		}
		return lines;
	}
}
