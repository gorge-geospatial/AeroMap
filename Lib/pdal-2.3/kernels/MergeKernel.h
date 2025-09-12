#ifndef MERGEKERNEL_H
#define MERGEKERNEL_H

#include <pdal/Kernel.h>

namespace pdal
{
	class MergeKernel : public Kernel
	{
	public:
		std::string getName() const override;
		int execute() override;

	private:
		void addSwitches(ProgramArgs& args);
		void validateSwitches(ProgramArgs& args);

		StringList m_files;
		std::string m_outputFile;
	};
}

#endif // #ifndef MERGEKERNEL_H
