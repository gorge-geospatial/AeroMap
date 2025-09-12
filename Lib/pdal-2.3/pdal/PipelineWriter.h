#ifndef PIPELINEWRITER_H
#define PIPELINEWRITER_H

#include <pdal/pdal_internal.h>

#include <map>
#include <string>

namespace pdal
{
	class Stage;

	namespace PipelineWriter
	{
		typedef std::map<const Stage*, std::string> TagMap;

		void writePipeline(Stage* s, const std::string& filename);
		void writePipeline(Stage* s, std::ostream& strm);
	}
}

#endif // #ifndef PIPELINEWRITER_H
