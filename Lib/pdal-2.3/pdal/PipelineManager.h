#ifndef PIPELINEMANAGER_H
#define PIPELINEMANAGER_H

#include <pdal/PointTable.h>
#include <pdal/PointView.h>
#include <pdal/Options.h>
#include <pdal/Log.h>

#include <vector>
#include <string>

namespace pdal
{

	struct QuickInfo;
	class Stage;
	class StageFactory;

	struct StageCreationOptions
	{
		std::string m_filename;
		std::string m_driver;
		Stage* m_parent;
		Options m_options;
		std::string m_tag;
	};

	class PipelineManager
	{
		FRIEND_TEST(json, tags);
	public:
		struct ExecResult
		{
			ExecResult() : m_mode(ExecMode::None), m_count(0)
			{}
			ExecResult(ExecMode mode, point_count_t count) :
				m_mode(mode), m_count(count)
			{}

			ExecMode m_mode;
			point_count_t m_count;
		};

		PipelineManager(point_count_t streamLimit = 10000);
		~PipelineManager();

		void setProgressFd(int fd)
		{
			m_progressFd = fd;
		}

		void readPipeline(std::istream& input);
		void readPipeline(const std::string& filename);

		// Use these to manually add stages into the pipeline manager.
		Stage& addReader(const std::string& type);
		Stage& addFilter(const std::string& type);
		Stage& addWriter(const std::string& type);

		// These add stages, hook dependencies and set necessary options.
		// They're preferable to the above as they're more flexible and safer.
		Stage& makeReader(const std::string& inputFile, std::string driver);
		Stage& makeReader(const std::string& inputFile, std::string driver,
			Options options);
		Stage& makeReader(StageCreationOptions& opts);

		Stage& makeFilter(const std::string& driver);
		Stage& makeFilter(const std::string& driver, Options options);
		Stage& makeFilter(const std::string& driver, Stage& parent);
		Stage& makeFilter(const std::string& driver, Stage& parent,
			Options options);
		Stage& makeFilter(StageCreationOptions& ops);

		Stage& makeWriter(const std::string& outputFile, std::string driver);
		Stage& makeWriter(const std::string& outputFile, std::string driver,
			Options options);
		Stage& makeWriter(const std::string& outputFile, std::string driver,
			Stage& parent);
		Stage& makeWriter(const std::string& outputFile, std::string driver,
			Stage& parent, Options options);
		Stage& makeWriter(StageCreationOptions& ops);

		// Return the first leaf stage of a pipeline, or nullptr if the pipeline
		// is empty.
		Stage* getStage() const
		{
			const auto& llist = leaves();
			return llist.size() ? llist[0] : nullptr;
		}

		// Set the log to be available to stages.
		void setLog(const LogPtr& log);

		QuickInfo preview() const;
		void prepare() const;
		ExecResult execute(ExecMode mode);
		point_count_t execute();
		void executeStream(StreamPointTable& table);
		void validateStageOptions() const;
		bool pipelineStreamable() const;
		bool hasReader() const;

		// Get the resulting point views.
		const PointViewSet& views() const
		{
			return m_viewSet;
		}

		// Get the point table data.
		PointTableRef pointTable() const
		{
			return m_table;
		}

		MetadataNode getMetadata() const;
		Options& commonOptions()
		{
			return m_commonOptions;
		}
		OptionsMap& stageOptions()
		{
			return m_stageOptions;
		}
		std::vector<Stage*> roots() const;
		std::vector<Stage*> leaves() const;
		void replace(Stage* sOld, Stage* sNew);

		const std::vector<Stage*> stages() const
		{
			return m_stages;
		}
		void destroyStage(Stage* s = nullptr);

	private:
		void setOptions(Stage& stage, const Options& addOps);
		Options stageOptions(Stage& stage);

		std::unique_ptr<StageFactory> m_factory;
		std::unique_ptr<SimplePointTable> m_tablePtr;
		PointTableRef m_table;
		std::unique_ptr<FixedPointTable> m_streamTablePtr;
		StreamPointTable& m_streamTable;
		Options m_commonOptions;
		OptionsMap m_stageOptions;
		PointViewSet m_viewSet;
		std::vector<Stage*> m_stages; // stage observer, never owner
		int m_progressFd;
		std::istream* m_input;
		LogPtr m_log;

		PipelineManager& operator=(const PipelineManager&); // not implemented
		PipelineManager(const PipelineManager&); // not implemented
	};
	typedef std::unique_ptr<PipelineManager> PipelineManagerPtr;

}

#endif // #ifndef PIPELINEMANAGER_H
