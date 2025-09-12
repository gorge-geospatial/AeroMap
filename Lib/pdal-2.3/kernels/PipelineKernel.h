#ifndef PIPELINEKERNEL_H
#define PIPELINEKERNEL_H

#include <pdal/Kernel.h>
#include <pdal/PipelineManager.h>
#include <pdal/PipelineWriter.h>
#include <pdal/util/FileUtils.h>

namespace pdal
{
	class PipelineKernel : public Kernel
	{
	public:
		std::string getName() const override;
		int execute() override;
		PipelineKernel();

	private:
		void addSwitches(ProgramArgs& args);
		void validateSwitches(ProgramArgs& args);
		virtual bool isStagePrefix(const std::string& stage);

		std::string m_inputFile;
		std::string m_pipelineFile;
		std::string m_metadataFile;
		bool m_validate;
		std::string m_PointCloudSchemaOutput;
		std::string m_progressFile;
		int m_progressFd;
		bool m_usestdin;
		bool m_stream;
		bool m_noStream;
		ExecMode m_mode;
		StringList m_dimNames;
	};
}

#endif // #ifndef PIPELINEKERNEL_H
