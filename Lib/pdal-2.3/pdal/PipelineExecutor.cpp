/******************************************************************************
* Copyright (c) 2016, Howard Butler (howard@hobu.co)
****************************************************************************/

#include <pdal/PipelineExecutor.h>
#include <pdal/PDALUtils.h>

namespace pdal
{
	PipelineExecutor::PipelineExecutor(std::string const& json)
		: m_json(json)
		, m_executed(false)
		, m_logLevel(pdal::LogLevel::Error)
	{
	}

	std::string PipelineExecutor::getPipeline() const
	{
		if (!m_executed)
			throw pdal_error("Pipeline has not been executed!");

		std::stringstream strm;
		pdal::PipelineWriter::writePipeline(m_manager.getStage(), strm);
		return strm.str();
	}

	std::string PipelineExecutor::getMetadata() const
	{
		if (!m_executed)
			throw pdal_error("Pipeline has not been executed!");

		std::stringstream strm;
		MetadataNode root = m_manager.getMetadata().clone("metadata");
		pdal::Utils::toJSON(root, strm);
		return strm.str();
	}

	std::string PipelineExecutor::getSchema() const
	{
		if (!m_executed)
			throw pdal_error("Pipeline has not been executed!");

		std::stringstream strm;
		MetadataNode root = m_manager.pointTable().layout()->toMetadata().clone("schema");
		pdal::Utils::toJSON(root, strm);
		return strm.str();
	}

	bool PipelineExecutor::validate()
	{
		std::stringstream strm;
		strm << m_json;
		m_manager.readPipeline(strm);
		m_manager.prepare();

		return true;
	}

	int64_t PipelineExecutor::execute()
	{
		std::stringstream strm;
		strm << m_json;
		m_manager.readPipeline(strm);
		point_count_t count = m_manager.execute();

		m_executed = true;

		return count;
	}

	void PipelineExecutor::setLogStream(std::ostream& strm)
	{

		LogPtr log(Log::makeLog("pypipeline", &strm));
		log->setLevel(m_logLevel);
		m_manager.setLog(log);

	}

	void PipelineExecutor::setLogLevel(int level)
	{
		if (level < 0 || level > 8)
			throw pdal_error("log level must be between 0 and 8!");

		m_logLevel = static_cast<pdal::LogLevel>(level);
		setLogStream(m_logStream);
	}

	int PipelineExecutor::getLogLevel() const
	{
		return static_cast<int>(m_logLevel);
	}

	std::string PipelineExecutor::getLog() const
	{
		return m_logStream.str();
	}
}
