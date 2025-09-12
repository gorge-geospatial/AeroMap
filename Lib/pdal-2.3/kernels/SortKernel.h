#ifndef SORTKERNEL_H
#define SORTKERNEL_H

#include <pdal/Kernel.h>

namespace pdal
{
	class SortKernel : public Kernel
	{
	public:
		std::string getName() const override;
		int execute() override;
		SortKernel();

	private:
		void addSwitches(ProgramArgs& args);

		std::string m_inputFile;
		std::string m_outputFile;
		bool m_bCompress;
		bool m_bForwardMetadata;
	};
}

#endif // #ifndef SORTKERNEL_H
