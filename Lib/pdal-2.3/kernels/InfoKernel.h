#ifndef INFOKERNEL_H
#define INFOKERNEL_H

#include <pdal/Kernel.h>
#include <pdal/PipelineManager.h>
#include <pdal/PointView.h>
#include <pdal/Stage.h>
#include <pdal/util/FileUtils.h>

namespace pdal
{
	class InfoKernel : public Kernel
	{
	public:
		std::string getName() const override;
		int execute() override;

		InfoKernel();
		void setup(const std::string& filename);
		MetadataNode run(const std::string& filename);

		inline bool showAll() { return m_showAll; }
		inline void doShowAll(bool value) { m_showAll = value; }
		inline void doComputeSummary(bool value) { m_showSummary = value; }
		inline void doComputeBoundary(bool value) { m_boundary = value; }

	private:
		void addSwitches(ProgramArgs& args);
		void validateSwitches(ProgramArgs& args);
		void makeReader(const std::string& filename);
		void makePipeline();
		void dump(MetadataNode& root);
		MetadataNode dumpSummary(const QuickInfo& qi);

		std::string m_inputFile;
		bool m_showStats;
		bool m_showSchema;
		bool m_showAll;
		bool m_showMetadata;
		bool m_boundary;
		std::string m_pointIndexes;
		std::string m_dimensions;
		std::string m_enumerate;
		std::string m_queryPoint;
		std::string m_pipelineFile;
		bool m_showSummary;
		bool m_needPoints;
		bool m_usestdin;

		Stage* m_statsStage;
		Stage* m_hexbinStage;
		Stage* m_infoStage;
		Stage* m_reader;

		MetadataNode m_tree;
	};
}

#endif // #ifndef INFOKERNEL_H
