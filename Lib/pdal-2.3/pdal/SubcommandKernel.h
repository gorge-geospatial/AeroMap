#ifndef SUBCOMMANDKERNEL_H
#define SUBCOMMANDKERNEL_H

#include <pdal/Kernel.h>

namespace pdal
{
	class SubcommandKernel : public Kernel
	{
	public:
		SubcommandKernel& operator=(const Kernel&) = delete;
		SubcommandKernel(const SubcommandKernel&) = delete;
		virtual ~SubcommandKernel()
		{}

	protected:
		SubcommandKernel()
		{}

		std::string m_subcommand;

	private:
		/// \return true on success, false if the user is asking for help
		virtual bool doSwitches(const StringList& cmdArgs, ProgramArgs& args);
		// Prevent the standard addSwitches() from being defined.
		virtual void addSwitches(ProgramArgs& args) final
		{}
		virtual void addSubSwitches(ProgramArgs& args,
			const std::string& subcommand)
		{}

		virtual StringList subcommands() const = 0;
		virtual void outputHelp();
	};
}

#endif // #ifndef SUBCOMMANDKERNEL_H
