#ifndef SPLITKERNEL_H
#define SPLITKERNEL_H

#include <pdal/Kernel.h>

namespace pdal
{
	class SplitKernel : public Kernel
	{
	public:
		std::string getName() const override;
		int execute() override;

	private:
		void addSwitches(ProgramArgs& args);
		void validateSwitches(ProgramArgs& args);

		std::string m_inputFile;
		std::string m_outputFile;
		uint32_t m_capacity;
		double m_length;
		double m_xOrigin;
		double m_yOrigin;
	};
}

#endif // #ifndef SPLITKERNEL_H
