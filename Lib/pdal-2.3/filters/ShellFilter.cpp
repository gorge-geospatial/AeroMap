/******************************************************************************
* Copyright (c) 2019, Howard Butler, howard@hobu.co
****************************************************************************/

#include "ShellFilter.h"

#include <pdal/util/Algorithm.h>
#include <pdal/util/ProgramArgs.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>


namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.shell",
		"Execute a shell operation inline with PDAL pipeline steps",
		"http://pdal.io/stages/filters.shell.html"
	};

	CREATE_STATIC_STAGE(ShellFilter, s_info)

	std::string ShellFilter::getName() const { return s_info.name; }

	void ShellFilter::addArgs(ProgramArgs& args)
	{
		args.add("command", "Command to run", m_command).setPositional();
	}

	void ShellFilter::initialize()
	{
		std::string allowed;
		int set = Utils::getenv("PDAL_ALLOW_SHELL", allowed);
		if (set == -1)
			throw pdal::pdal_error("PDAL_ALLOW_SHELL environment variable not set, shell access is not allowed");
	}

	PointViewSet ShellFilter::run(PointViewPtr view)
	{
		log()->get(LogLevel::Debug) << "running command : '" << m_command << "'" << std::endl;

		int status = Utils::run_shell_command(m_command.c_str(), m_command_output);
		if (status)
		{
			std::stringstream msg;
			msg << "Command '" << m_command << "' failed to execute";
			msg << " with output '" << m_command_output << "'";
			throw pdal::pdal_error(msg.str());

		}

		log()->get(LogLevel::Debug) << "command output: '" << m_command_output << "'" << std::endl;
		log()->get(LogLevel::Debug) << "status: '" << status << "'" << std::endl;

		PointViewSet views;
		views.insert(view);
		return views;
	}

	void ShellFilter::done(PointTableRef table)
	{
		bool isJson = (m_command_output.find("{") != m_command_output.npos) ||
			(m_command_output.find("}") != m_command_output.npos);

		if (isJson)
			m_metadata.addWithType("output",
				m_command_output,
				"json",
				"Command output");
		else
			m_metadata.add("output", m_command_output, "Command output");

	}
}
