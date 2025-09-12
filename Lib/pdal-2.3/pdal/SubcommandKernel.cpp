/******************************************************************************
* Copyright (c) 2018, Hobu Inc. (info@hobu.co)
****************************************************************************/

#include <pdal/SubcommandKernel.h>
#include <pdal/util/Algorithm.h>

namespace pdal
{
	bool SubcommandKernel::doSwitches(const StringList& cmdArgs, ProgramArgs& args)
	{
		StringList stringArgs = extractStageOptions(cmdArgs);

		try
		{
			bool help;

			// parseSimple allows us to scan for the help option without
			// raising exception about missing arguments and so on.
			// It also removes consumed args from the arg list, so parse a copy
			// that will be ignored by parse() below.
			// Also search for a subcommand.
			ProgramArgs hargs;
			const StringList& subs = subcommands();
			assert(subs.size());
			hargs.add("help,h", "Print help message", help);
			hargs.add("subcommand", "Subcommand for application", m_subcommand).
				setPositional();
			hargs.parseSimple(stringArgs);
			if (help)
				return false;

			if (m_subcommand.empty())
				throw pdal_error(getName() + ": no subcommand found.");
			if (!Utils::contains(subs, m_subcommand))
				throw pdal_error(getName() + ": invalid subcommand '" +
					m_subcommand + "'.");

			addBasicSwitches(args);
			addSubSwitches(args, m_subcommand);
			args.parse(stringArgs);
		}
		catch (arg_error& e)
		{
			throw pdal_error(getName() + ": " + e.m_error);
		}
		return true;
	}


	void SubcommandKernel::outputHelp()
	{
		auto outputStdOpts = [this]()
		{
			ProgramArgs args;
			addBasicSwitches(args);

			std::cout << "standard options:" << std::endl;
			args.dump(std::cout, 2, Utils::screenWidth());
		};

		auto outputSubOpts = [this](const std::string& sub)
		{
			ProgramArgs args;
			addSubSwitches(args, sub);

			std::cout << "subcommand '" << sub << "' options:" << std::endl;
			args.dump(std::cout, 2, Utils::screenWidth());
		};

		if (Utils::contains(subcommands(), m_subcommand))
		{
			// We're repeating addSubSwitches here (it's also called from
			// outputSubOpts), but it keeps things cleaner since outputSubOpts
			// also writes output.
			ProgramArgs args;
			addSubSwitches(args, m_subcommand);

			std::cout << "usage: " << "pdal " << getShortName() << " " <<
				m_subcommand << " [options] " << args.commandLine() <<
				std::endl;

			outputStdOpts();
			outputSubOpts(m_subcommand);
		}
		else
		{
			std::cout << "usage: " << "pdal " << getShortName() <<
				" <subcommand> [options] " << std::endl;

			outputStdOpts();
			for (const std::string& subcmd : subcommands())
				outputSubOpts(subcmd);
		}

		std::cout << "\nFor more information, see the full documentation for "
			"PDAL at https://pdal.io/\n" << std::endl;
	}
}
