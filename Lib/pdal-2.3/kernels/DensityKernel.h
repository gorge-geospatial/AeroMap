#ifndef DENSITYKERNEL_H
#define DENSITYKERNEL_H

#include <pdal/Kernel.h>
#include <pdal/PipelineManager.h>

namespace pdal
{
	class DensityKernel : public Kernel
	{
	public:
		std::string getName() const override;
		int execute() override;

	private:
		Stage* m_hexbinStage;
		std::string m_inputFile;
		std::string m_outputFile;
		std::string m_driverName;
		std::string m_layerName;
		uint32_t m_sampleSize;
		int32_t m_density;
		double m_edgeLength;
		double m_cullArea;
		bool m_doSmooth;

		virtual void addSwitches(ProgramArgs& args);
		void outputDensity(pdal::SpatialReference const& ref);
	};
}

#endif // #ifndef DENSITYKERNEL_H
