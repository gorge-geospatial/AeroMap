/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
****************************************************************************/

#include <pdal/PipelineWriter.h>

#include <pdal/Metadata.h>
#include <pdal/PDALUtils.h>
#include <pdal/Stage.h>

namespace pdal
{
	namespace
	{
		std::string generateTag(Stage* stage, PipelineWriter::TagMap& tags)
		{
			auto tagExists = [tags](const std::string& tag)
			{
				for (auto& t : tags)
				{
					if (t.second == tag)
						return true;
				}
				return false;
			};

			std::string tag = stage->tag();
			if (tag.empty())
			{
				for (size_t i = 1; ; ++i)
				{
					tag = stage->getName() + std::to_string(i);
					tag = Utils::replaceAll(tag, ".", "_");
					if (!tagExists(tag))
						break;
				}
			}
			return tag;
		}

		void generateTags(Stage* stage, PipelineWriter::TagMap& tags)
		{
			for (Stage* s : stage->getInputs())
				generateTags(s, tags);
			tags[stage] = generateTag(stage, tags);
		}

	} // anonymous namespace

	namespace PipelineWriter
	{

		void writePipeline(Stage* stage, const std::string& filename)
		{
			std::ostream* out = Utils::createFile(filename, false);
			writePipeline(stage, *out);
			Utils::closeFile(out);
		}

		void writePipeline(Stage* stage, std::ostream& strm)
		{
			TagMap tags;
			generateTags(stage, tags);

			MetadataNode root;
			stage->serialize(root, tags);
			Utils::toJSON(root, strm);
		}
	}
}
