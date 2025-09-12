#ifndef TRANSLATEKERNEL_H
#define TRANSLATEKERNEL_H

#include <pdal/Kernel.h>
#include <pdal/PipelineManager.h>

#include <memory>
#include <string>
#include <vector>

namespace pdal
{
	class TranslateKernel : public Kernel
	{
	public:
		std::string getName() const override;
		int execute() override;
		TranslateKernel();

	private:
		virtual void addSwitches(ProgramArgs& args);
		virtual void validateSwitches(ProgramArgs& args);
		void makeJSONPipeline();
		void makeArgPipeline();

		std::string m_inputFile;
		std::string m_outputFile;
		std::string m_pipelineOutputFile;
		std::string m_readerType;
		StringList m_filterType;
		std::string m_writerType;
		std::string m_filterJSON;
		std::string m_metadataFile;
		bool m_noStream;
		bool m_stream;
		ExecMode m_mode;
		StringList m_dimNames;
	};
}

#endif // #ifndef TRANSLATEKERNEL_H
