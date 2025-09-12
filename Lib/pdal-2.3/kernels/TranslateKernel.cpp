/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
* Copyright (c) 2015, Bradley J Chambers (brad.chambers@gmail.com)
* Copyright (c) 2013, Howard Butler (hobu.inc@gmail.com)
****************************************************************************/

#include "TranslateKernel.h"

#include <pdal/PipelineWriter.h>
#include <pdal/PointTable.h>
#include <pdal/PointView.h>
#include <pdal/Reader.h>
#include <pdal/StageFactory.h>
#include <pdal/PipelineReaderJSON.h>
#include <pdal/Writer.h>
#include <pdal/util/FileUtils.h>

#include <memory>
#include <string>
#include <vector>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"kernels.translate",
		"The Translate kernel allows users to construct a pipeline " \
			"consisting of a reader, a writer, and N filter stages. " \
			"Any supported stage type can be specified from the command " \
			"line, reducing the need to create custom kernels for every " \
			"combination.",
		"http://pdal.io/apps/translate.html"
	};

	CREATE_STATIC_KERNEL(TranslateKernel, s_info)

	std::string TranslateKernel::getName() const
	{
		return s_info.name;
	}

	TranslateKernel::TranslateKernel()
	{}

	void TranslateKernel::addSwitches(ProgramArgs& args)
	{
		args.add("input,i", "Input filename", m_inputFile).
			setPositional();
		args.add("output,o", "Output filename", m_outputFile).
			setPositional();
		args.add("filter,f", "Filter type", m_filterType).
			setOptionalPositional();
		args.add("json", "PDAL pipeline from which to extract filters.",
			m_filterJSON);
		args.add("pipeline,p", "Pipeline output", m_pipelineOutputFile);
		args.add("metadata,m", "Dump metadata output to the specified file",
			m_metadataFile);
		args.add("reader,r", "Reader type", m_readerType);
		args.add("writer,w", "Writer type", m_writerType);
		args.add("nostream", "Run in standard mode", m_noStream);
		args.add("stream", "Run in stream mode.  Error if not possible.", m_stream);
		args.add("dims", "Dimensions to store", m_dimNames);
	}


	void TranslateKernel::validateSwitches(ProgramArgs&)
	{
		if (m_stream && m_noStream)
			throw pdal_error("Can't specify both 'stream' and 'nostream' options.");

		if (m_stream)
			m_mode = ExecMode::Stream;
		else if (m_noStream)
			m_mode = ExecMode::Standard;
		else
			m_mode = ExecMode::PreferStream;
	}


	/*
	  Build a pipeline from a JSON filter specification.
	*/
	void TranslateKernel::makeJSONPipeline()
	{
		std::string json;

		if (pdal::FileUtils::fileExists(m_filterJSON))
			json = pdal::FileUtils::readFileIntoString(m_filterJSON);

		if (json.empty())
			json = m_filterJSON;
		std::stringstream in(json);
		m_manager.readPipeline(in);

		std::vector<Stage*> roots = m_manager.roots();
		if (roots.size() > 1)
			throw pdal_error("Can't process pipeline with more than one root.");

		Stage* r(nullptr);
		if (roots.size())
			r = dynamic_cast<Reader*>(roots[0]);
		if (r)
		{
			StageCreationOptions ops{ m_inputFile, m_readerType, nullptr,
				Options(), r->tag() };
			m_manager.replace(r, &m_manager.makeReader(ops));
		}
		else
		{
			r = &m_manager.makeReader(m_inputFile, m_readerType);
			if (roots.size())
				roots[0]->setInput(*r);
		}

		std::vector<Stage*> leaves = m_manager.leaves();
		if (leaves.size() != 1)
			throw pdal_error("Can't process pipeline with more than one "
				"terminal stage.");

		Stage* w = dynamic_cast<Writer*>(leaves[0]);
		if (w)
			m_manager.replace(w, &m_manager.makeWriter(m_outputFile, m_writerType));
		else
		{
			// We know we have a leaf because we added a reader.
			StageCreationOptions ops{ m_outputFile, m_writerType, leaves[0],
				Options(), "" };  // These last two args just keep compiler quiet.
			m_manager.makeWriter(ops);
		}
	}


	/*
	  Build a pipeline from filters specified as command-line arguments.
	*/
	void TranslateKernel::makeArgPipeline()
	{
		std::string readerType(m_readerType);
		if (!readerType.empty() && !Utils::startsWith(readerType, "readers."))
			readerType.insert(0, "readers.");
		Stage& reader = m_manager.makeReader(m_inputFile, readerType);
		Stage* stage = &reader;

		// add each filter provided on the command-line,
		// updating the stage pointer
		for (auto const& f : m_filterType)
		{
			std::string filter_name(f);

			if (!Utils::startsWith(f, "filters."))
				filter_name.insert(0, "filters.");

			Stage& filter = m_manager.makeFilter(filter_name, *stage);
			stage = &filter;
		}
		std::string writerType(m_writerType);
		if (!writerType.empty() && !Utils::startsWith(writerType, "writers."))
			writerType.insert(0, "writers.");
		m_manager.makeWriter(m_outputFile, writerType, *stage);
	}


	int TranslateKernel::execute()
	{
		std::ostream* metaOut(nullptr);

		if (m_filterJSON.size() && m_filterType.size())
			throw pdal_error("Cannot set both --filter options and --json options");

		if (m_metadataFile.size())
		{
			if (m_pipelineOutputFile.size())
				m_log->get(LogLevel::Info) << "Metadata will not be written. "
				"'pipeline' option prevents execution.";
			else
			{
				metaOut = FileUtils::createFile(m_metadataFile);
				if (!metaOut)
					throw pdal_error("Couldn't output metadata output file '" +
						m_metadataFile + "'.");
			}
		}

		if (!m_filterJSON.empty())
			makeJSONPipeline();
		else
			makeArgPipeline();

		// If we write pipeline output, we don't run, and therefore don't write
		if (m_pipelineOutputFile.size() > 0)
		{
			PipelineWriter::writePipeline(m_manager.getStage(),
				m_pipelineOutputFile);
			return 0;
		}

		m_manager.pointTable().layout()->setAllowedDims(m_dimNames);
		if (m_manager.execute(m_mode).m_mode == ExecMode::None)
			throw pdal_error("Couldn't run translation pipeline in requested "
				"execution mode.");

		if (metaOut)
		{
			MetadataNode m = m_manager.getMetadata();
			*metaOut << Utils::toJSON(m);
			FileUtils::closeFile(metaOut);
		}

		return 0;
	}
}
