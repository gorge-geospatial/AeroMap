#ifndef KERNEL_H
#define KERNEL_H

#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <pdal/PipelineManager.h>
#include <pdal/PluginHelper.h>
#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	class Options;
	class PointView;

	typedef std::shared_ptr<PointView> PointViewPtr;

	class Kernel
	{
		FRIEND_TEST(KernelTest, parseOption);

	public:
		Kernel& operator=(const Kernel&) = delete;
		Kernel(const Kernel&) = delete;
		virtual ~Kernel()
		{}

		// call this, to start the machine
		int run(const StringList& cmdArgs, LogPtr& log);

		virtual std::string getName() const = 0;
		std::string getShortName() const
		{
			StringList names = Utils::split2(getName(), '.');
			return names.size() == 2 ? names[1] : std::string();
		}

		enum class ParseStageResult
		{
			Ok,
			Invalid,
			Unknown
		};

	protected:
		Kernel();

		void addBasicSwitches(ProgramArgs& args);
		StringList extractStageOptions(const StringList& cmdArgs);
		Stage& makeReader(const std::string& inputFile, std::string driver);
		Stage& makeReader(const std::string& inputFile, std::string driver, Options options);
		Stage& makeFilter(const std::string& driver, Stage& parent);
		Stage& makeFilter(const std::string& driver, Stage& parent,	Options options);
		Stage& makeFilter(const std::string& driver);
		Stage& makeWriter(const std::string& outputFile, Stage& parent,	std::string driver);
		Stage& makeWriter(const std::string& outputFile, Stage& parent,	std::string driver, Options options);
		virtual bool isStagePrefix(const std::string& stageType);

		LogPtr m_log;
		PipelineManager m_manager;
		std::string m_driverOverride;

	private:
		virtual void addSwitches(ProgramArgs& /*args*/)
		{}

		// Implement this to do sanity checking of cmd line
		// will throw if the user gave us bad options.
		virtual void validateSwitches(ProgramArgs& /*args*/)
		{}

		// implement this, to do your actual work
		// it will be wrapped in a global catch try/block for you
		virtual int execute() = 0;
		virtual void outputHelp();
		/// \return  True on success, false if the user is asking for help.
		virtual bool doSwitches(const StringList& cmdArgs, ProgramArgs& args);

		int innerRun(ProgramArgs& args);
		void outputVersion();
		int doStartup();
		int doExecution(ProgramArgs& args);
		ParseStageResult parseStageOption(std::string o, std::string& stage,
			std::string& option, std::string& value);

		static ParseStageResult test_parseStageOption(std::string o,
			std::string& stage, std::string& option, std::string& value);

		bool m_showOptions;
		bool m_showTime;
		bool m_hardCoreDebug;
		std::string m_label;
	};

	std::ostream& operator<<(std::ostream& ostr, const Kernel&);
}

#endif // #ifndef KERNEL_H
