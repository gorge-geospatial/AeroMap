#ifndef RANDOMKERNEL_H
#define RANDOMKERNEL_H

#include <pdal/Kernel.h>
#include <pdal/util/FileUtils.h>

namespace pdal
{
	class RandomKernel : public Kernel
	{
	public:
		RandomKernel();
		std::string getName() const override;
		int execute() override;

	private:
		void addSwitches(ProgramArgs& arg);

		std::string m_outputFile;
		bool m_bCompress;
		uint64_t m_numPointsToWrite;
		BOX3D m_bounds;
		std::string m_distribution;
		std::string m_means;
		std::string m_stdevs;
	};
}

#endif // #ifndef RANDOMKERNEL_H
