/******************************************************************************
* Copyright (c) 2018, Hobu Inc. (hobu.inc@gmail.com)
****************************************************************************/

#pragma once

#include <string>

namespace pdal
{
	struct PluginInfo
	{
		std::string name;
		std::string description;
		std::string link;
		PluginInfo(const std::string& n, const std::string& d, const std::string& l)
			: name(n), description(d), link(l)
		{}
	};

	struct StaticPluginInfo : public PluginInfo
	{
		StringList extensions;

		StaticPluginInfo(const std::string& n, const std::string& d,
			const std::string& l) : PluginInfo(n, d, l)
		{}

		StaticPluginInfo(const std::string& n, const std::string& d,
			const std::string& l, const StringList& e) :
			PluginInfo(n, d, l), extensions(e)
		{}
	};

}

extern "C"
{
	// This is a placeholder so as not to break existing plugins even though the
	// internal interface has changed.
	typedef void PF_ExitFunc;
	typedef void (*PF_InitFunc)();
}
