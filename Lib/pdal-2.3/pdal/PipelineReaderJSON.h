#ifndef PIPELINEREADERJSON_H
#define PIPELINEREADERJSON_H

#include <pdal/pdal_internal.h>
#include <pdal/JsonFwd.h>
#include <pdal/StageFactory.h>

#include <vector>
#include <string>

#include <pdal/Options.h>
#include <pdal/StageFactory.h>

namespace pdal
{

	class Stage;
	class PipelineManager;

	class PipelineReaderJSON
	{
		friend class PipelineManager;

	public:
		PipelineReaderJSON(PipelineManager&);

	private:
		PipelineReaderJSON& operator=(const PipelineReaderJSON&) = delete;
		PipelineReaderJSON(const PipelineReaderJSON&) = delete;

		typedef std::map<std::string, Stage*> TagMap;

		void parsePipeline(NL::json&);
		void readPipeline(const std::string& filename);
		void readPipeline(std::istream& input);
		std::string extractType(NL::json& node);
		std::string extractFilename(NL::json& node);
		std::string extractTag(NL::json& node, TagMap& tags);
		std::vector<Stage*> extractInputs(NL::json& node, TagMap& tags);
		Options extractOptions(NL::json& node);
		void handleInputTag(const std::string& tag, const TagMap& tags,
			std::vector<Stage*>& inputs);

		PipelineManager& m_manager;
	};

}

#endif // #ifndef PIPELINEREADERJSON_H
