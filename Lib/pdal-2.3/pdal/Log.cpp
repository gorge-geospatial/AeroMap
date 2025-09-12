/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
* Copyright (c) 2011, Howard Butler, hobu.inc@gmail.com
****************************************************************************/

#include <pdal/Log.h>
#include <pdal/PDALUtils.h>

#include <fstream>
#include <ostream>

namespace pdal
{
	Log::Log(std::string const& leaderString, std::string const& outputName,
		bool timing)
		: m_level(LogLevel::Warning)
		, m_deleteStreamOnCleanup(false)
		, m_timing(timing)
	{
		if (Utils::iequals(outputName, "stdlog"))
			m_log = &std::clog;
		else if (Utils::iequals(outputName, "stderr"))
			m_log = &std::cerr;
		else if (Utils::iequals(outputName, "stdout"))
			m_log = &std::cout;
		else if (Utils::iequals(outputName, "devnull"))
			m_log = &m_nullStream;
		else
		{
			m_log = Utils::createFile(outputName);
			m_deleteStreamOnCleanup = true;
		}
		m_leaders.push(leaderString);
		if (m_timing)
			m_start = m_clock.now();
	}

	Log::Log(std::string const& leaderString, std::ostream* v, bool timing)
		: m_level(LogLevel::Error)
		, m_deleteStreamOnCleanup(false)
		, m_timing(timing)
	{
		m_log = v;
		m_leaders.push(leaderString);
		if (m_timing)
			m_start = m_clock.now();
	}

	LogPtr Log::makeLog(std::string const& leaderString,
		std::string const& outputName, bool timing)
	{
		return LogPtr(new Log(leaderString, outputName, timing));
	}

	LogPtr Log::makeLog(std::string const& leaderString, std::ostream* v,
		bool timing)
	{
		return LogPtr(new Log(leaderString, v, timing));
	}

	Log::~Log()
	{
		if (m_deleteStreamOnCleanup)
		{
			m_log->flush();
			delete m_log;
		}
	}

	void Log::floatPrecision(int level)
	{
		m_log->setf(std::ios_base::fixed, std::ios_base::floatfield);
		m_log->precision(level);
	}

	void Log::clearFloat()
	{
		m_log->unsetf(std::ios_base::fixed);
		m_log->unsetf(std::ios_base::floatfield);
	}

	std::ostream& Log::get(LogLevel level)
	{
		const auto incoming(Utils::toNative(level));
		const auto stored(Utils::toNative(m_level));
		const auto nativeDebug(Utils::toNative(LogLevel::Debug));
		if (incoming <= stored)
		{
			const std::string l = leader();

			*m_log << "(" << l;
			if (l.size())
				*m_log << " ";
			*m_log << getLevelString(level);
			if (m_timing)
				*m_log << " " << now();
			*m_log << ") " <<
				std::string(incoming < nativeDebug ? 0 : incoming - nativeDebug,
					'\t');
			return *m_log;
		}
		return m_nullStream;
	}

	std::string Log::getLevelString(LogLevel level) const
	{
		switch (level)
		{
		case LogLevel::Error:
			return "Error";
			break;
		case LogLevel::Warning:
			return "Warning";
			break;
		case LogLevel::Info:
			return "Info";
			break;
		default:
			return "Debug";
		}
	}

	std::string Log::now() const
	{
		std::chrono::steady_clock::time_point end = m_clock.now();

		std::chrono::duration<double> diff = end - m_start;
		std::stringstream ss;

		ss << std::fixed << std::setprecision(3) << diff.count();
		return ss.str();
	}
}
